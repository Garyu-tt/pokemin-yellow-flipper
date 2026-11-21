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

static void city(YellowGame* g,int enabled){
 yellow_init(g);g->running=true;g->screen=YScreenWorld;
 g->story=YStoryPikachuFollows;g->has_pikachu=1;g->follower_valid=0;
 g->map=YMapViridian;g->x=29;g->y=18;g->facing=YDown;
 g->native_mart_intro_extension=enabled!=0;
}
int main(void){
 YellowGame g;city(&g,1);
 yellow_input(&g,YDown);
 assert(g.map==YMapViridianMart&&g.x==3&&g.y==6&&g.steps==1);
 assert(g.screen==YScreenDialog&&g.dialog_count==2);
 assert(strcmp(g.dialog_lines[0],"HEY! YOU CAME")==0);
 assert(strcmp(g.dialog_lines[1],"FROM PALLET TOWN?")==0);
 assert(g.pending==YPendingNone&&g.balls==0&&g.potions==0);
 yellow_input(&g,YOk);assert(g.screen==YScreenDialog&&g.dialog_page==1);
 yellow_input(&g,YOk);assert(g.screen==YScreenDialog&&g.pending==YPendingGrantOakParcel);
 assert(g.map==YMapViridianMart&&g.x==2&&g.y==4&&g.steps==4);
 assert(g.oak_parcel_flags==0u); /* No item before acknowledgement. */
 yellow_input(&g,YOk);yellow_input(&g,YOk);yellow_input(&g,YOk);
 assert(g.screen==YScreenWorld&&g.oak_parcel_flags==YOakParcelGot);
 yellow_input(&g,YDown);yellow_input(&g,YDown);yellow_input(&g,YRight);
 yellow_input(&g,YDown);yellow_input(&g,YDown);
 assert(g.map==YMapViridian&&g.y==20);
 yellow_input(&g,YUp);assert(g.map==YMapViridianMart&&g.screen==YScreenWorld);
 assert(g.balls==0&&g.potions==0&&sizeof(YellowSave)==132);
 city(&g,0);yellow_input(&g,YDown);
 assert(g.map==YMapViridianMart&&g.screen==YScreenWorld);
 city(&g,1);g.map=YMapViridianMart;g.x=2;g.y=5;
 assert(yellow_walkable(&g,2,5)&&!yellow_walkable(&g,1,5));
 yellow_input(&g,YLeft);
 assert(g.x==2&&g.y==5&&g.screen==YScreenWorld);
 puts("R014_MART_AUTO_GREETING_MOVEMENT_PARCEL_ONCE_PASS");
 return 0;
}
