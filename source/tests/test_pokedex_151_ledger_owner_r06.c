#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
static void dex_numbers_and_bounds(void) {
    const uint8_t expected[]={133,16,19,25,10,11,17,29,32,74,95,50,27};
    assert(sizeof(YellowDex)==38);
    assert(sizeof(YellowDexSave)==52);
    for(unsigned i=0;i<sizeof(expected);i++)
        assert(yellow_dex_number(i)==expected[i]);
    assert(yellow_dex_number(13)==0 && yellow_dex_number(255)==0);
    YellowDex d={0};
    assert(!yellow_dex_mark_seen(&d,0));
    assert(!yellow_dex_mark_caught(&d,152));
    assert(!yellow_dex_mark_caught(NULL,1));
    for(int n=1;n<=151;n++) {
        assert(yellow_dex_mark_seen(&d,(uint8_t)n));
        assert(yellow_dex_seen(&d,(uint8_t)n));
        assert(!yellow_dex_caught(&d,(uint8_t)n));
    }
    assert(yellow_dex_seen_count(&d)==151 && yellow_dex_caught_count(&d)==0);
    for(int n=1;n<=151;n++)assert(yellow_dex_mark_caught(&d,(uint8_t)n));
    assert(yellow_dex_caught_count(&d)==151);
    assert(yellow_dex_valid(&d));
    d.seen[18]|=0x80;
    assert(!yellow_dex_valid(&d));
    d.seen[18]&=0x7f;
    d.seen[0]&=0xfe;
    assert(!yellow_dex_valid(&d));
}
static void battle_capture_runtime_and_legacy(void) {
    YellowGame g;yellow_init(&g);
    assert(yellow_dex_seen_count(&g.dex)==0);
    assert(yellow_dex_caught_count(&g.dex)==0);
    g.story=YStoryPikachuFollows;g.has_pikachu=1;
    g.screen=YScreenWorld;g.map=YMapRoute1;g.x=10;g.y=28;
    g.balls=3;g.follower_valid=0;
    yellow_begin_battle(&g,YK_PIDGEY,3);
    assert(yellow_dex_seen(&g.dex,16)&&!yellow_dex_caught(&g.dex,16));
    assert(yellow_dex_seen_count(&g.dex)==1);
    g.foe_hp=1;
    yellow_input(&g,YOk);
    assert(g.screen==YScreenFightMenu);
    for(unsigned i=0;i<yellow_active_move_count(&g);i++)yellow_input(&g,YDown);
    yellow_input(&g,YOk);
    assert(g.caught_count==1 && yellow_dex_caught(&g.dex,16));
    assert(yellow_dex_seen_count(&g.dex)==1);
    while(g.screen==YScreenDialog)yellow_input(&g,YOk);
    YellowSave parent;yellow_save_create(&g,&parent);
    assert(parent.version==13 && sizeof(parent)==132);
    YellowGame previous;
    assert(yellow_save_load(&previous,&parent));
    assert(yellow_dex_caught(&previous.dex,25));/* legacy Pikachu party */
    assert(yellow_dex_caught(&previous.dex,16));/* legacy Pidgey capture */
    assert(!yellow_dex_caught(&previous.dex,133));
    assert(yellow_dex_seen_count(&previous.dex)==2);
    YellowDexSave sidecar;
    assert(yellow_dex_save(&g.dex,parent.checksum,&sidecar));
    assert(sidecar.magic==YELLOW_DEX_SAVE_MAGIC && sidecar.version==1);
    assert(sidecar.parent_checksum==parent.checksum);
    YellowDex restored={0};
    assert(yellow_dex_restore(&restored,parent.checksum,&sidecar));
    assert(yellow_dex_caught(&restored,16));
    YellowDex before=restored;
    YellowDexSave broken=sidecar;broken.checksum^=1;
    assert(!yellow_dex_restore(&restored,parent.checksum,&broken));
    assert(memcmp(&before,&restored,sizeof(restored))==0);
    assert(!yellow_dex_restore(&restored,parent.checksum+1,&sidecar));
    broken=sidecar;broken.seen[18]|=0x80;
    assert(!yellow_dex_restore(&restored,parent.checksum,&broken));
    broken=sidecar;broken.magic=0;
    assert(!yellow_dex_restore(&restored,parent.checksum,&broken));
    assert(memcmp(&before,&restored,sizeof(restored))==0);
    YellowSave again;yellow_save_create(&g,&again);
    assert(memcmp(&parent,&again,sizeof(parent))==0);/* new feature keeps v13 unchanged */
}
int main(void) {
    dex_numbers_and_bounds();
    battle_capture_runtime_and_legacy();
    puts("POKEDEX_151_PINNED_IDS_CAPTURE_SEEN_CAUGHT_SIDECAR_V13_PASS");
    return 0;
}
