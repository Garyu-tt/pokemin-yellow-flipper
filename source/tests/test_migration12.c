#include <assert.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"

int main(void) {
    YellowGame g;yellow_init(&g);
    g.story=YStoryPikachuFollows;
    g.has_pikachu=1;g.map=YMapRoute1;g.x=10;g.y=28;
    g.screen=YScreenWorld;g.follower_valid=0;
    g.pika_pp[0]=9;g.balls=4;
    YellowSave new_save;yellow_save_create(&g,&new_save);
    assert(new_save.version==13);
    YellowGame copy;assert(yellow_save_load(&copy,&new_save));
    assert(copy.pika_pp[0]==9 && copy.balls==4);

    YellowSave prior11=new_save;
    prior11.version=11;prior11.checksum=checksum(&prior11);
    assert(!yellow_save_load(&copy,&prior11));
    assert(yellow_save_migrate_r11(&copy,&prior11));
    assert(copy.pika_pp[0]==9 && copy.balls==4);
    assert(prior11.version==11); /* non-destructive */

    YellowSave prior10=new_save;
    prior10.version=10;prior10.checksum=checksum(&prior10);
    assert(yellow_save_migrate_r10(&copy,&prior10));
    assert(copy.pika_pp[0]==9);

    prior11.checksum^=1;
    assert(!yellow_save_migrate_r11(&copy,&prior11));
    prior11.checksum^=1;
    prior11.pika_pp[0]=255;
    prior11.checksum=checksum(&prior11);
    assert(!yellow_save_migrate_r11(&copy,&prior11));

    prior10.version=9;prior10.checksum=checksum(&prior10);
    assert(!yellow_save_migrate_r10(&copy,&prior10));

    puts("R12_R10_R11_MIGRATION_AND_INTEGRITY_PASS");
    return 0;
}
