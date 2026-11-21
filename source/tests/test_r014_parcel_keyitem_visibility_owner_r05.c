#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "yellow_oak_parcel_sidecar.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
#include "../yellow_pewter_progress.c"
#include "../yellow_atomic_snapshot.c"
int main(void){
 assert(sizeof(YellowSave)==132u);
 assert(!y_oak_parcel_in_bag(0,false));
 assert(y_oak_parcel_in_bag(YOakParcelGot,false));
 assert(!y_oak_parcel_in_bag(YOakParcelAll,false));
 assert(!y_oak_parcel_in_bag(YOakParcelDelivered,false));
 assert(!y_oak_parcel_in_bag(0x80u,false));
 assert(!y_oak_parcel_in_bag(YOakParcelGot,true));
 YellowGame g;yellow_init(&g);
 assert(!y_oak_parcel_in_bag(g.oak_parcel_flags,g.oak_parcel_integrity_error));
 assert(y_oak_parcel_record_got(&g.oak_parcel_flags));
 assert(!y_oak_parcel_record_got(&g.oak_parcel_flags));
 assert(y_oak_parcel_in_bag(g.oak_parcel_flags,g.oak_parcel_integrity_error));
 YellowSave a={0},b={0};yellow_save_create(&g,&a);
 g.oak_parcel_integrity_error=true;
 assert(!y_oak_parcel_in_bag(g.oak_parcel_flags,g.oak_parcel_integrity_error));
 yellow_save_create(&g,&b);
 assert(memcmp(&a,&b,sizeof(a))==0);
 g.oak_parcel_integrity_error=false;
 assert(y_oak_parcel_record_delivered(&g.oak_parcel_flags));
 assert(!y_oak_parcel_in_bag(g.oak_parcel_flags,g.oak_parcel_integrity_error));
 assert(!y_oak_parcel_record_delivered(&g.oak_parcel_flags));
 puts("R014_PARCEL_KEY_ITEM_POCKET_VISIBILITY_NO_GRANT_V13_PASS");
 return 0;
}
