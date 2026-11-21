#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "yellow_core.h"
#include "yellow_oak_parcel_slot.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
#include "../yellow_pewter_progress.c"
#include "../yellow_atomic_snapshot.c"

int main(void){
 assert(sizeof(YellowSave)==132u);
 YellowGame g;yellow_init(&g);
 assert(g.oak_parcel_flags==0 && !g.oak_parcel_integrity_error);
 assert(y_oak_parcel_runtime_safe(0,false));
 assert(y_oak_parcel_runtime_safe(YOakParcelGot,false));
 assert(y_oak_parcel_runtime_safe(YOakParcelAll,false));
 assert(!y_oak_parcel_runtime_safe(0,true));
 assert(!y_oak_parcel_runtime_safe(YOakParcelGot,true));
 assert(!y_oak_parcel_runtime_safe(YOakParcelAll,true));
 assert(!y_oak_parcel_runtime_safe(YOakParcelDelivered,false));
 assert(!y_oak_parcel_runtime_safe(0x80u,false));
 g.oak_parcel_integrity_error=true;
 assert(!y_oak_parcel_runtime_safe(g.oak_parcel_flags,g.oak_parcel_integrity_error));
 /* Integrity error is intentionally volatile, never pollutes legacy v13. */
 YellowSave before={0},after={0};
 yellow_save_create(&g,&before);
 g.oak_parcel_integrity_error=false;
 yellow_save_create(&g,&after);
 assert(memcmp(&before,&after,sizeof(before))==0);
 YOakParcelSlot slot={0};
 assert(y_oak_parcel_slot_make(YOakParcelGot,before.checksum,13u,&slot));
 uint8_t flags=0;
 assert(y_oak_parcel_slot_restore(&flags,before.checksum,13u,&slot));
 assert(flags==YOakParcelGot);
 puts("R014_PARCEL_FAIL_CLOSED_GUARD_AND_V13_ABI_PASS");
 return 0;
}
