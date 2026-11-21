/* New independent Viridian Mart / Source 42 test; v062 protected unchanged. */
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
static uint32_t fnv(const uint8_t* p,unsigned n){
 uint32_t h=2166136261u;
 for(unsigned i=0;i<n;i++)h=(h^p[i])*16777619u;
 return h;
}
static void ready(YellowGame* g,uint8_t map,int x,int y){
 yellow_init(g);
 g->screen=YScreenWorld;g->running=true;
 g->story=YStoryPikachuFollows;g->has_pikachu=0;
 g->follower_valid=0;g->map=map;g->x=(int8_t)x;g->y=(int8_t)y;
}
int main(){
 assert(YMapRoute22==20&&YMapViridianMart==21&&YELLOW_MART_MAP_ID==21);
 assert(sizeof(YellowSave)==132&&sizeof(YellowExactPositionV1)==28);
 const YellowLevelAsset* a=yellow_gameplay_scene_asset(YMapViridianMart);
 assert(a&&a->w==4&&a->h==4&&a->blocks_count==40&&a->tiles_count==96);
 assert(yellow_map_w(YMapViridianMart)==8&&yellow_map_h(YMapViridianMart)==8);
 assert(fnv(ymart_map,16)==0xd654bfc0u);
 assert(fnv(ymart_collision,8)==0x464f8cb7u);
 assert(fnv(ymart_blocks,640)==0x26e2a75du);
 unsigned walk=0;
 for(int y=0;y<8;y++)for(int x=0;x<8;x++){
  uint8_t b=ymart_map[(y/2)*4+x/2];
  uint8_t tile=ymart_blocks[(unsigned)b*16+((y%2)*2+1)*4+(x%2)*2];
  bool src_collision=(tile==17||tile==26||tile==28||tile==60||tile==94);
  assert(yellow_viridian_mart_passable(x,y)==src_collision);
  YellowGame g;ready(&g,YMapViridianMart,x,y);
  assert(yellow_walkable(&g,x,y)==src_collision ||
         (y==7&&(x==3||x==4))); /* warp tiles override normal collision */
  walk+=(unsigned)src_collision;
 }
 assert(walk==31);
 for(int y=0;y<16;y++)for(int x=0;x<16;x++){
  unsigned block=ymart_map[(y/4)*4+x/4];
  uint8_t exp=ymart_blocks[block*16+(y%4)*4+x%4];
  assert(yellow_world_tile_id(YMapViridianMart,x,y)==exp);
 }
 assert(yellow_world_tile_id(YMapViridianMart,16,0)==0xff);
 assert(yellow_world_tile_id(YMapViridianMart,-1,0)==0xff);
 assert(!yellow_viridian_mart_passable(8,0));
 assert(!yellow_viridian_mart_passable(0,8));
 for(int i=0;i<2;i++){
  YellowGame g;ready(&g,YMapViridian,29,18);
  yellow_input(&g,YDown);
  assert(g.map==YMapViridianMart&&g.x==3&&g.y==6&&g.steps==1);
  if(i){
   yellow_input(&g,YRight);
   assert(g.map==YMapViridianMart&&g.x==4&&g.y==6);
  }
  yellow_input(&g,YDown);
  assert(g.map==YMapViridian&&g.x==29&&g.y==20&&g.steps==(i?3:2));
 }
 YellowGame g,restored;
 YellowAtomicSnapshot atom={0};YellowExactPositionV1 extra={0};
 ready(&g,YMapViridianMart,3,6);
 g.has_pikachu=1;g.pika_level=8;
 g.hp=y_calc_stats(YK_PIKACHU,8).hp;
 g.experience=y_exp_threshold(YK_PIKACHU,8);
 assert(yellow_atomic_make(&g,25,&atom));
 assert(atom.parent.version==13&&atom.parent.map==YMapRoute2);
 assert(yellow_exact_position_make(&g,&atom,&extra));
 assert(yellow_atomic_unpack(&atom,&restored));
 assert(restored.map==YMapRoute2);
 assert(yellow_exact_position_restore(&restored,&atom,&extra));
 assert(restored.map==YMapViridianMart&&restored.x==3&&restored.y==6);
 puts("MAP004_MART_256_ORIGINAL_TILES_64_COLLISION_3_WARPS_SAVE_PASS");
 return 0;
}
