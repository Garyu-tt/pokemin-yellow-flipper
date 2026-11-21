#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "yellow_oak_parcel_sidecar.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
#include "../yellow_pewter_progress.c"
static void must_reject(const YOakParcelSidecar* p,uint32_t parent){
 uint8_t target=0xa5u;
 assert(!y_oak_parcel_restore(&target,parent,p));
 assert(target==0xa5u);
}
int main(void){
 assert(sizeof(YOakParcelSidecar)==16u);
 assert(sizeof(YellowSave)==132u);
 assert(y_oak_parcel_valid(0));
 assert(y_oak_parcel_valid(YOakParcelGot));
 assert(y_oak_parcel_valid(YOakParcelAll));
 assert(!y_oak_parcel_valid(YOakParcelDelivered));
 assert(!y_oak_parcel_valid(4));
 uint8_t flags=0;
 assert(!y_oak_parcel_record_delivered(&flags) && flags==0);
 assert(y_oak_parcel_record_got(&flags) && flags==YOakParcelGot);
 assert(!y_oak_parcel_record_got(&flags) && flags==YOakParcelGot);
 assert(y_oak_parcel_record_delivered(&flags) && flags==YOakParcelAll);
 assert(!y_oak_parcel_record_delivered(&flags) && flags==YOakParcelAll);
 assert(!y_oak_parcel_record_got(NULL));
 assert(!y_oak_parcel_record_delivered(NULL));
 flags=YOakParcelDelivered;
 assert(!y_oak_parcel_record_got(&flags) && flags==YOakParcelDelivered);
 assert(!y_oak_parcel_record_delivered(&flags) && flags==YOakParcelDelivered);
 YellowGame game;yellow_init(&game);
 YellowSave parent;yellow_save_create(&game,&parent);
 assert(parent.magic==YELLOW_SAVE_MAGIC && parent.version==13);
 YellowGame restored_parent;
 assert(yellow_save_load(&restored_parent,&parent));
 YOakParcelSidecar saved={0},copy;
 assert(y_oak_parcel_save(YOakParcelGot,parent.checksum,&saved));
 uint8_t restored=0;
 assert(y_oak_parcel_restore(&restored,parent.checksum,&saved));
 assert(restored==YOakParcelGot);
 assert(y_oak_parcel_record_delivered(&restored));
 YOakParcelSidecar delivered;
 assert(y_oak_parcel_save(restored,parent.checksum,&delivered));
 restored=0;
 assert(y_oak_parcel_restore(&restored,parent.checksum,&delivered));
 assert(restored==YOakParcelAll);
 assert(!y_oak_parcel_save(YOakParcelDelivered,parent.checksum,&copy));
 assert(!y_oak_parcel_save(4,parent.checksum,&copy));
 assert(!y_oak_parcel_save(0,parent.checksum,NULL));
 must_reject(&saved,parent.checksum^1u);
 copy=saved;copy.magic^=1u;must_reject(&copy,parent.checksum);
 copy=saved;copy.version++;must_reject(&copy,parent.checksum);
 copy=saved;copy.reserved=1;must_reject(&copy,parent.checksum);
 copy=saved;copy.flags=YOakParcelDelivered;must_reject(&copy,parent.checksum);
 copy=saved;copy.flags=4;must_reject(&copy,parent.checksum);
 copy=saved;copy.parent_checksum++;must_reject(&copy,parent.checksum);
 copy=saved;copy.checksum^=0x100u;must_reject(&copy,parent.checksum);
 assert(!y_oak_parcel_restore(NULL,parent.checksum,&saved));
 assert(!y_oak_parcel_restore(&restored,parent.checksum,NULL));
 /* Saving a different parent must not allow transplanting parcel progress. */
 YellowSave different=parent;different.rng++;different.checksum++;
 must_reject(&saved,different.checksum);
 /* The v13 parent is unchanged: no invented item/quest inside its 132 bytes. */
 YellowSave again;yellow_save_create(&game,&again);
 assert(memcmp(&parent,&again,sizeof(parent))==0);
 puts("R014_OAK_PARCEL_SOURCE_FLAGS_V13_SIDECAR_PASS");
 return 0;
}
