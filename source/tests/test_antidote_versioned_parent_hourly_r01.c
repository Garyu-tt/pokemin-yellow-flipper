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
#include "../yellow_antidote_inventory_slot.c"
int main(void) {
 YellowGame g={0}, loaded={0}; yellow_init(&g);
 g.running=true; g.screen=YScreenWorld; g.story=YStoryPikachuFollows;
 g.has_pikachu=1;g.pika_level=5;g.hp=15;g.antidotes=3;
 YellowAtomicSnapshot v3={0},v1={0},bad={0};
 assert(yellow_atomic_make(&g,7,&v3));
 assert(v3.version==3u && v3.parent.version==13u && sizeof(v3.parent)==132u);
 assert(yellow_atomic_unpack(&v3,&loaded));
 assert(loaded.antidotes==0); /* stock intentionally outside v13 */
 v1=v3;v1.version=1u;v1.checksum=yellow_atomic_checksum(&v1);
 assert(yellow_atomic_unpack(&v1,&loaded));
 assert(yellow_atomic_select(&v1,&v3)==0); /* same generation, first wins */
 bad=v3;bad.version=2u;bad.checksum=yellow_atomic_checksum(&bad);
 assert(!yellow_atomic_unpack(&bad,NULL));
 bad=v3;bad.version=4u;bad.checksum=yellow_atomic_checksum(&bad);
 assert(!yellow_atomic_unpack(&bad,NULL));
 YellowAntidoteInventorySlot slot={{0}};
 assert(yellow_antidote_inventory_create(3,v3.parent.checksum,v3.generation,&slot));
 uint8_t stock=0;assert(yellow_antidote_inventory_restore(&slot,v3.parent.checksum,7,&stock)&&stock==3);
 assert(!yellow_antidote_inventory_restore(&slot,v3.parent.checksum,8,&stock)&&stock==3);
 for(unsigned i=0;i<24;i++)for(unsigned j=0;j<8;j++){
  YellowAntidoteInventorySlot corrupted=slot;
  corrupted.wire[i]^=(uint8_t)(1u<<j);
  stock=88;
  assert(!yellow_antidote_inventory_restore(&corrupted,v3.parent.checksum,7,&stock)&&stock==88);
 }
 printf("R20_V3_PARENT_V13_LEGACY_V1_RESTORE_RESERVED_V2_REJECT_MUTATION_192_PASS\n");
 return 0;
}
