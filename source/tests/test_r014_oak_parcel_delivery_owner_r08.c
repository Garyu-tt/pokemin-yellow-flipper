#include <assert.h>
#include <stdio.h>
#include "yellow_core.h"
#include "yellow_oak_parcel_sidecar.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
#include "../yellow_pewter_progress.c"
#include "../yellow_atomic_snapshot.c"
#include "../yellow_exact_position_v1.c"
static void oak(YellowGame* g,uint8_t flags,bool corrupt) {
 yellow_init(g);g->running=true;g->screen=YScreenWorld;
 g->story=YStoryPikachuFollows;g->has_pikachu=1;g->follower_valid=0;
 g->map=YMapOaksLab;g->x=5;g->y=3;g->facing=YUp;
 g->oak_parcel_flags=flags;g->oak_parcel_integrity_error=corrupt;
}
static void delivery(void) {
 YellowGame g;oak(&g,YOakParcelGot,false);
 int visible_oak=0;
 for(unsigned i=0;i<yellow_npc_count;i++)
  if(yellow_npcs[i].map==YMapOaksLab && yellow_npcs[i].type==YActorOak &&
     yellow_npcs[i].x==5 && yellow_npcs[i].y==2 &&
     yellow_npc_visible(&g,&yellow_npcs[i]))visible_oak++;
 assert(visible_oak==1);
 assert(y_oak_parcel_in_bag(g.oak_parcel_flags,false));
 yellow_input(&g,YOk);
 assert(g.screen==YScreenDialog&&g.pending==YPendingDeliverOakParcel);
 assert(g.oak_parcel_flags==YOakParcelGot);
 yellow_input(&g,YOk);
 assert(g.screen==YScreenDialog&&g.oak_parcel_flags==YOakParcelGot);
 yellow_input(&g,YOk);
 assert(g.screen==YScreenWorld&&g.oak_parcel_flags==YOakParcelAll);
 assert(!y_oak_parcel_in_bag(g.oak_parcel_flags,false));
 YellowSave parent;yellow_save_create(&g,&parent);
 assert(sizeof(YellowSave)==132u&&parent.version==13u);
 YOakParcelSidecar side;
 assert(y_oak_parcel_save(g.oak_parcel_flags,parent.checksum,&side));
 uint8_t restored=0;
 assert(y_oak_parcel_restore(&restored,parent.checksum,&side));
 assert(restored==YOakParcelAll);
 assert(!y_oak_parcel_record_delivered(&restored));
 yellow_input(&g,YOk);
 assert(g.oak_parcel_flags==YOakParcelAll);
 puts("R014_OAK_REAL_NPC_TALK_DELIVERY_CONSUMES_ITEM_ONCE_V13_PASS");
}
static void no_false_delivery(void) {
 YellowGame g;
 oak(&g,0,false);yellow_input(&g,YOk);
 assert(g.pending!=YPendingDeliverOakParcel&&g.oak_parcel_flags==0);
 oak(&g,YOakParcelAll,false);yellow_input(&g,YOk);
 assert(g.pending!=YPendingDeliverOakParcel&&g.oak_parcel_flags==YOakParcelAll);
 oak(&g,YOakParcelGot,true);yellow_input(&g,YOk);
 assert(g.pending!=YPendingDeliverOakParcel&&g.oak_parcel_flags==YOakParcelGot);
 oak(&g,YOakParcelGot,false);g.map=YMapPallet;yellow_input(&g,YOk);
 assert(g.pending!=YPendingDeliverOakParcel&&g.oak_parcel_flags==YOakParcelGot);
 oak(&g,YOakParcelGot,false);g.has_pikachu=0;yellow_input(&g,YOk);
 assert(g.pending!=YPendingDeliverOakParcel&&g.oak_parcel_flags==YOakParcelGot);
 puts("R014_OAK_NO_PREMATURE_NO_CORRUPT_NO_WRONG_MAP_DELIVERY_PASS");
}
int main(void){delivery();no_false_delivery();puts("R014_OAK_DELIVERY_R08_ALL_PASS");return 0;}
