#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
#include "../yellow_pewter_progress.c"
#define YELLOW_ATOMIC_ALLOW_V4 1
#include "../yellow_atomic_snapshot.c"
int main(void) {
 YellowGame g;yellow_init(&g);
 g.story=YStoryPikachuFollows;g.has_pikachu=1;
 g.screen=YScreenWorld;g.map=YMapRoute1;g.x=10;g.y=10;
 YellowAtomicSnapshot s={0};
 assert(yellow_atomic_make(&g,9u,&s));
 assert(s.version==4u && s.parent.version==13u);
 YellowGame recovered;
 assert(yellow_atomic_unpack(&s,&recovered));
 YellowAtomicSnapshot old=s;
 old.version=3u;old.checksum=yellow_atomic_checksum(&old);
 assert(yellow_atomic_unpack(&old,&recovered));
 assert(yellow_atomic_select(&old,&s)==0); /* equal generation tie -> slot0 */
 old.version=1u;old.checksum=yellow_atomic_checksum(&old);
 assert(yellow_atomic_unpack(&old,&recovered));
 old.version=2u;old.checksum=yellow_atomic_checksum(&old);
 assert(!yellow_atomic_unpack(&old,&recovered));
 old=s;old.version=5u;old.checksum=yellow_atomic_checksum(&old);
 assert(!yellow_atomic_unpack(&old,&recovered));
 old=s;old.checksum^=1u;
 assert(!yellow_atomic_unpack(&old,&recovered));
 old=s;old.committed=0;
 assert(!yellow_atomic_unpack(&old,&recovered));
 puts("V4_PARENT_MIGRATION_V3_V1_CHECKSUM_SEAL_AND_REJECT_PASS");
 return 0;
}
