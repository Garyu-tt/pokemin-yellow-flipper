#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
#include "../yellow_pewter_progress.c"
#include "../yellow_atomic_snapshot.c"
int main(void) {
 YellowGame g;yellow_init(&g);
 g.running=true;g.screen=YScreenWorld;g.story=YStoryPikachuFollows;
 g.has_pikachu=1;g.map=YMapRoute3;
 g.pika_level=5;g.balls=5;
 g.hp=y_calc_stats(YK_PIKACHU,5).hp;
 YellowAtomicSnapshot older={0},latest={0},fault={0};
 assert(yellow_atomic_make(&g,1,&older));
 assert(yellow_atomic_make(&g,2,&latest));
 assert(yellow_atomic_select(&older,&latest)==1);
 fault=latest;fault.committed=0;
 assert(yellow_atomic_select(&older,&fault)==0);
 fault=latest;fault.version=2;
 assert(yellow_atomic_select(&older,&fault)==0);
 fault=latest;fault.items.checksum^=1;
 fault.checksum=yellow_atomic_checksum(&fault);
 assert(yellow_atomic_select(&older,&fault)==0);
 fault=latest;fault.pewter.parent_checksum^=1;
 fault.checksum=yellow_atomic_checksum(&fault);
 assert(yellow_atomic_select(&older,&fault)==0);
 fault=latest;fault.route3.parent_checksum^=1;
 fault.checksum=yellow_atomic_checksum(&fault);
 assert(yellow_atomic_select(&older,&fault)==0);
 fault=latest;fault.dex.checksum^=1;
 fault.checksum=yellow_atomic_checksum(&fault);
 assert(yellow_atomic_select(&older,&fault)==0);
 fault=latest;fault.pc.checksum^=1;
 fault.checksum=yellow_atomic_checksum(&fault);
 assert(yellow_atomic_select(&older,&fault)==0);
 fault=latest;fault.forest.checksum^=1;
 fault.checksum=yellow_atomic_checksum(&fault);
 assert(yellow_atomic_select(&older,&fault)==0);
 fault=latest;fault.parent.checksum^=1;
 fault.checksum=yellow_atomic_checksum(&fault);
 assert(yellow_atomic_select(&older,&fault)==0);
 assert(yellow_atomic_select(NULL,&fault)==-1);
 assert(!yellow_atomic_make(&g,0,&fault));
 assert(yellow_atomic_make(&g,0xfffffffeu,&latest));
 assert(yellow_atomic_make(&g,1,&fault));
 assert(yellow_atomic_select(&latest,&fault)==1);
 puts("ATOMIC_SNAPSHOT_CHILD_PARENT_CHECKSUM_GENERATION_ROLLBACK_PASS");
 return 0;
}
