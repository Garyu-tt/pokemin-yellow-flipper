#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "yellow_oak_parcel_sidecar.h"
#include "yellow_oak_parcel_slot.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
#include "../yellow_pewter_progress.c"
#include "../yellow_atomic_snapshot.c"

static void rejected_unchanged(const YOakParcelSlot* slot,
                               uint32_t checksum,uint32_t generation){
 uint8_t flags=0xa5u;
 assert(!y_oak_parcel_slot_restore(&flags,checksum,generation,slot));
 assert(flags==0xa5u);
}
int main(void){
 assert(sizeof(YellowSave)==132u);
 assert(sizeof(YOakParcelSidecar)==16u);
 assert(sizeof(YOakParcelSlot)==32u);
 assert(offsetof(YOakParcelSlot,seal)==28u);
 YellowGame game;
 yellow_init(&game);
 assert(game.oak_parcel_flags==0);
 YellowAtomicSnapshot old={0},newer={0},damaged={0};
 assert(yellow_atomic_make(&game,7u,&old));
 assert(yellow_atomic_unpack(&old,NULL));
 YOakParcelSlot slot_a={0},slot_b={0},corrupt={0};
 assert(y_oak_parcel_slot_make(0,old.parent.checksum,old.generation,&slot_a));
 assert(!y_oak_parcel_slot_make(0,old.parent.checksum,0,NULL));
 assert(!y_oak_parcel_slot_make(0,old.parent.checksum,0,&corrupt));
 assert(!y_oak_parcel_slot_make(YOakParcelDelivered,old.parent.checksum,7,&corrupt));
 game.oak_parcel_flags=YOakParcelGot;
 assert(yellow_atomic_make(&game,8u,&newer));
 assert(yellow_atomic_unpack(&newer,NULL));
 assert(y_oak_parcel_slot_make(game.oak_parcel_flags,newer.parent.checksum,
                               newer.generation,&slot_b));
 assert(yellow_atomic_select(&old,&newer)==1);
 YellowGame restored={0};
 assert(yellow_atomic_unpack(&newer,&restored));
 assert(restored.oak_parcel_flags==0); /* v13 still has no invented quest bits */
 assert(y_oak_parcel_slot_restore(&restored.oak_parcel_flags,
          newer.parent.checksum,newer.generation,&slot_b));
 assert(restored.oak_parcel_flags==YOakParcelGot);
 assert(!y_oak_parcel_slot_restore(NULL,newer.parent.checksum,8,&slot_b));
 assert(!y_oak_parcel_slot_restore(&restored.oak_parcel_flags,
          newer.parent.checksum,8,NULL));
 rejected_unchanged(&slot_b,newer.parent.checksum,7);
 rejected_unchanged(&slot_b,newer.parent.checksum^1u,8);
 corrupt=slot_b;corrupt.generation=7;rejected_unchanged(&corrupt,newer.parent.checksum,8);
 corrupt=slot_b;corrupt.magic++;rejected_unchanged(&corrupt,newer.parent.checksum,8);
 corrupt=slot_b;corrupt.seal=0;rejected_unchanged(&corrupt,newer.parent.checksum,8);
 corrupt=slot_b;corrupt.checksum^=1u;rejected_unchanged(&corrupt,newer.parent.checksum,8);
 corrupt=slot_b;corrupt.parcel.flags=YOakParcelDelivered;
 rejected_unchanged(&corrupt,newer.parent.checksum,8);
 corrupt=slot_b;corrupt.parcel.checksum^=1u;
 rejected_unchanged(&corrupt,newer.parent.checksum,8);
 /* Torn latest parent must not import uncommitted quest flags. */
 damaged=newer;damaged.committed=0;
 assert(yellow_atomic_select(&old,&damaged)==0);
 YellowGame fallback={0};
 assert(yellow_atomic_unpack(&old,&fallback));
 assert(y_oak_parcel_slot_restore(&fallback.oak_parcel_flags,
          old.parent.checksum,old.generation,&slot_a));
 assert(fallback.oak_parcel_flags==0);
 rejected_unchanged(&slot_b,old.parent.checksum,old.generation);
 /* Even with identical parent checksum, a stale slot cannot cross generations. */
 assert(old.parent.checksum==newer.parent.checksum);
 rejected_unchanged(&slot_b,old.parent.checksum,old.generation);
 /* Delivered flag is a distinct transition, not merely 'got'. */
 uint8_t delivered=YOakParcelGot;
 assert(y_oak_parcel_record_delivered(&delivered));
 assert(y_oak_parcel_slot_make(delivered,newer.parent.checksum,9,&corrupt));
 delivered=0;
 assert(y_oak_parcel_slot_restore(&delivered,newer.parent.checksum,9,&corrupt));
 assert(delivered==YOakParcelAll);
 puts("R014_OAK_PARCEL_ATOMIC_GENERATION_SLOT_PASS");
 return 0;
}
