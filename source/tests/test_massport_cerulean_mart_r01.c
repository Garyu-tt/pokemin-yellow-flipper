/* MAP005 original CeruleanMart id67 native22, shared byte-exact MART scene.
 * Door warp #6 is source-proven, not invented. New isolated test.
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
#include "../yellow_pewter_progress.c"
#include "../yellow_atomic_snapshot.c"
#include "../yellow_exact_position_v1.c"
static void ready(YellowGame* g,uint8_t map,int x,int y){
 yellow_init(g);g->running=true;g->screen=YScreenWorld;
 g->story=YStoryPikachuFollows;g->has_pikachu=0;g->follower_valid=0;
 g->map=map;g->x=(int8_t)x;g->y=(int8_t)y;
}
int main(){
 assert(YMapViridianMart==21 && YMapCeruleanMart==22);
 assert(YELLOW_CERULEAN_MART_ID==22);
 assert(sizeof(YellowSave)==132 && sizeof(YellowExactPositionV1)==28);
 assert(yellow_gameplay_scene_asset(YMapCeruleanMart)==
        yellow_gameplay_scene_asset(YMapViridianMart));
 assert(yellow_map_w(YMapCeruleanMart)==8 && yellow_map_h(YMapCeruleanMart)==8);
 unsigned count=0;
 for(int y=0;y<8;y++)for(int x=0;x<8;x++){
  YellowGame g;ready(&g,YMapCeruleanMart,x,y);
  assert(yellow_walkable(&g,x,y)==yellow_viridian_mart_passable(x,y));
  count+=yellow_viridian_mart_passable(x,y);
 }
 assert(count==31);
 for(int y=0;y<16;y++)for(int x=0;x<16;x++)
  assert(yellow_world_tile_id(YMapCeruleanMart,x,y)==
         yellow_world_tile_id(YMapViridianMart,x,y));
 for(int j=0;j<2;j++){
  YellowGame g;ready(&g,YMapCeruleanCity,25,24);
  yellow_input(&g,YDown);
  assert(g.map==YMapCeruleanMart&&g.x==3&&g.y==6&&g.steps==1);
  if(j){
   yellow_input(&g,YRight);
   assert(g.map==YMapCeruleanMart&&g.x==4&&g.y==6);
  }
  yellow_input(&g,YDown);
  assert(g.map==YMapCeruleanCity&&g.x==25&&g.y==26&&
         g.steps==(j?3:2));
  yellow_input(&g,YUp);
  assert(g.map==YMapCeruleanMart&&g.x==3&&g.y==6);
 }
 YellowGame g,restored;YellowAtomicSnapshot parent={0};
 YellowExactPositionV1 ext={0};
 ready(&g,YMapCeruleanMart,3,6);
 g.has_pikachu=1;g.pika_level=8;g.hp=y_calc_stats(YK_PIKACHU,8).hp;
 g.experience=y_exp_threshold(YK_PIKACHU,8);
 assert(yellow_atomic_make(&g,55,&parent));
 assert(parent.parent.version==13&&parent.parent.map==YMapRoute2);
 assert(yellow_exact_position_make(&g,&parent,&ext));
 assert(yellow_atomic_unpack(&parent,&restored));
 assert(restored.map==YMapRoute2);
 assert(yellow_exact_position_restore(&restored,&parent,&ext));
 assert(restored.map==YMapCeruleanMart&&restored.x==3&&restored.y==6);
 puts("MAP005_CERULEAN_MART_SHARED_ORIGINAL_64_CELLS_WARPS_V13_PASS");
 return 0;
}
