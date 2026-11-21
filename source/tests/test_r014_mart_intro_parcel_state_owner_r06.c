#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
#include "../yellow_pewter_progress.c"
#include "../yellow_atomic_snapshot.c"
#include "../yellow_exact_position_v1.c"
static void at_mart_door(YellowGame* g,uint8_t flags,bool corrupt){
 yellow_init(g);g->running=true;g->screen=YScreenWorld;
 g->story=YStoryPikachuFollows;g->has_pikachu=1;g->follower_valid=0;
 g->map=YMapViridian;g->x=29;g->y=18;g->facing=YDown;
 g->native_mart_intro_extension=true;
 g->oak_parcel_flags=flags;g->oak_parcel_integrity_error=corrupt;
}
static void enter(YellowGame* g){
 yellow_input(g,YDown);
 assert(g->map==YMapViridianMart&&g->x==3&&g->y==6);
}
int main(void){
 YellowGame g;
 at_mart_door(&g,0,false);enter(&g);
 assert(g.screen==YScreenDialog&&g.dialog_count==2);
 assert(!g.oak_parcel_flags);
 yellow_input(&g,YOk);yellow_input(&g,YOk);
 assert(g.screen==YScreenDialog&&g.pending==YPendingGrantOakParcel);
 assert(g.x==2&&g.y==4&&g.oak_parcel_flags==0u);
 yellow_input(&g,YOk);yellow_input(&g,YOk);yellow_input(&g,YOk);
 assert(g.screen==YScreenWorld&&g.oak_parcel_flags==YOakParcelGot);
 yellow_input(&g,YDown);yellow_input(&g,YDown);yellow_input(&g,YRight);
 yellow_input(&g,YDown);yellow_input(&g,YDown);assert(g.map==YMapViridian);
 yellow_input(&g,YUp);assert(g.map==YMapViridianMart&&g.screen==YScreenWorld);
 at_mart_door(&g,YOakParcelGot,false);enter(&g);
 assert(g.screen==YScreenWorld&&g.oak_parcel_flags==YOakParcelGot);
 at_mart_door(&g,YOakParcelAll,false);enter(&g);
 assert(g.screen==YScreenWorld&&g.oak_parcel_flags==YOakParcelAll);
 at_mart_door(&g,0,true);enter(&g);
 assert(g.screen==YScreenWorld&&g.oak_parcel_integrity_error);
 at_mart_door(&g,0x80u,false);enter(&g);
 /* invalid state must not launch original grant prelude */
 assert(g.screen==YScreenWorld);
 assert(sizeof(YellowSave)==132u);
 puts("R014_MART_GRANT_ONCE_FAILCLOSED_V13_PASS");
 return 0;
}
