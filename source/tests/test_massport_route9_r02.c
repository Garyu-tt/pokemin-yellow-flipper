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
static void begin(YellowGame* g,uint8_t map,int x,int y){
 yellow_init(g);g->running=true;g->screen=YScreenWorld;
 g->story=YStoryPikachuFollows;g->has_pikachu=0;
 g->follower_valid=0;g->map=map;g->x=x;g->y=y;
}
int main(void){
 assert(YMapCeruleanCity==18 && YMapRoute9==19);
 assert(sizeof(YellowSave)==132 && sizeof(YellowExactPositionV1)==28);
 const YellowLevelAsset* scene=yellow_gameplay_scene_asset(YMapRoute9);
 assert(scene && scene->w==30 && scene->h==9);
 assert(yellow_map_w(YMapRoute9)==60 && yellow_map_h(YMapRoute9)==18);
 unsigned total=0;
 for(int y=0;y<18;y++)for(int x=0;x<60;x++)
   total+=(unsigned)yellow_route9_passable(x,y);
 assert(total==457);
 assert(yellow_world_tile_id(YMapRoute9,120,0)==0xff);
 assert(yellow_world_tile_id(YMapRoute9,0,-1)==0xff);
 const int open[]={12,13,14,16,17,19,20,21,22,23,24,25};
 for(unsigned i=0;i<12;i++){
  YellowGame g;int y=open[i];begin(&g,YMapCeruleanCity,39,y);
  yellow_input(&g,YRight);
  assert(g.map==YMapRoute9&&g.x==0&&g.y==y-8&&g.steps==1);
  yellow_input(&g,YLeft);
  assert(g.map==YMapCeruleanCity&&g.x==39&&g.y==y&&g.steps==2);
 }
 const int blocked[]={8,9,10,11,15,18};
 for(unsigned i=0;i<6;i++){
  YellowGame g;begin(&g,YMapCeruleanCity,39,blocked[i]);
  yellow_input(&g,YRight);
  assert(g.map==YMapCeruleanCity&&g.x==39&&g.steps==0);
 }
 YellowGame g,loaded;YellowAtomicSnapshot atom={0};
 YellowExactPositionV1 pos={0};
 begin(&g,YMapRoute9,8,9);
 g.has_pikachu=1;g.pika_level=8;
 g.hp=y_calc_stats(YK_PIKACHU,8).hp;
 g.experience=y_exp_threshold(YK_PIKACHU,8);
 assert(yellow_atomic_make(&g,31,&atom));
 assert(atom.parent.version==13&&atom.parent.map==YMapRoute2);
 assert(yellow_exact_position_make(&g,&atom,&pos));
 assert(yellow_atomic_unpack(&atom,&loaded));
 assert(loaded.map==YMapRoute2);
 assert(yellow_exact_position_restore(&loaded,&atom,&pos));
 assert(loaded.map==YMapRoute9&&loaded.x==8&&loaded.y==9);
 puts("ROUTE9_REAL_INPUT_12_RECIPROCAL_6_BLOCKED_POSITION_SAVE_PASS");
 return 0;
}
