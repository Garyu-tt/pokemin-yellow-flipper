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
 yellow_init(g);
 g->running=true;g->screen=YScreenWorld;
 g->story=YStoryPikachuFollows;g->has_pikachu=0;
 g->follower_valid=0;g->map=map;g->x=x;g->y=y;
}
int main(void){
 assert(YMapCeruleanCity==18&&YMapRoute9==19&&YMapCeruleanMart==22);
 assert(YMapRoute10==23);
 assert(sizeof(YellowSave)==132&&sizeof(YellowExactPositionV1)==28);
 const YellowLevelAsset* a=yellow_gameplay_scene_asset(YMapRoute10);
 assert(a&&a->w==10&&a->h==36);
 assert(yellow_map_w(YMapRoute10)==20&&yellow_map_h(YMapRoute10)==72);
 assert(yellow_world_tile_id(YMapRoute10,-1,0)==0xff);
 assert(yellow_world_tile_id(YMapRoute10,40,0)==0xff);
 assert(yellow_world_tile_id(YMapRoute10,0,144)==0xff);
 unsigned pass=0;
 for(int y=0;y<72;y++)for(int x=0;x<20;x++)
  pass+=(unsigned)yellow_route10_passable(x,y);
 assert(pass==505);
 assert(!yellow_route10_passable(-1,8));
 assert(!yellow_route10_passable(20,8));
 assert(!yellow_route10_passable(0,72));
 for(int y=8;y<=9;y++){
  assert(yellow_route9_passable(59,y));
  assert(yellow_route10_passable(0,y));
  YellowGame g;begin(&g,YMapRoute9,59,y);
  yellow_input(&g,YRight);
  assert(g.map==YMapRoute10&&g.x==0&&g.y==y&&g.steps==1);
  yellow_input(&g,YLeft);
  assert(g.map==YMapRoute9&&g.x==59&&g.y==y&&g.steps==2);
  begin(&g,YMapRoute10,0,y);
  yellow_input(&g,YLeft);
  assert(g.map==YMapRoute9&&g.x==59&&g.y==y&&g.steps==1);
  yellow_input(&g,YRight);
  assert(g.map==YMapRoute10&&g.x==0&&g.y==y&&g.steps==2);
 }
 for(int y=0;y<18;y++)if(y!=8&&y!=9){
  YellowGame g;begin(&g,YMapRoute9,59,y);
  yellow_input(&g,YRight);
  assert(g.map==YMapRoute9&&g.x==59&&g.steps==0);
  begin(&g,YMapRoute10,0,y);
  yellow_input(&g,YLeft);
  assert(g.map==YMapRoute10&&g.x==0&&g.steps==0);
 }
 /* Unsupported original Route10 Pokecenter and Power Plant destinations
  * cannot become a fake accessible native map. */
 uint8_t dest=0,dx=0,dy=0;
 assert(!destination(YMapRoute10,11,19,&dest,&dx,&dy));
 assert(!destination(YMapRoute10,6,39,&dest,&dx,&dy));
 YellowGame g,loaded;YellowAtomicSnapshot atom={0};
 YellowExactPositionV1 pos={0};
 begin(&g,YMapRoute10,3,9);
 g.has_pikachu=1;g.pika_level=8;
 g.hp=y_calc_stats(YK_PIKACHU,8).hp;
 g.experience=y_exp_threshold(YK_PIKACHU,8);
 assert(yellow_atomic_make(&g,31,&atom));
 assert(atom.parent.version==13&&atom.parent.map==YMapRoute2);
 assert(yellow_exact_position_make(&g,&atom,&pos));
 assert(yellow_atomic_unpack(&atom,&loaded));
 assert(loaded.map==YMapRoute2);
 assert(yellow_exact_position_restore(&loaded,&atom,&pos));
 assert(loaded.map==YMapRoute10&&loaded.x==3&&loaded.y==9);
 puts("ROUTE10_NATIVE_SOURCE_REAL_INPUT_2_BIDIRECTIONAL_16_BLOCKED_POSITION_SAVE_PASS");
 return 0;
}
