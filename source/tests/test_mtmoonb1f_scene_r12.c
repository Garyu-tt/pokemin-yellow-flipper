/* R12 independent B1F spatial coverage and game-input test.
 * Geometry comes from exact pinned MtMoonB1F.blk and Cavern_Coll whitelist;
 * no unported B2F destinations may be faked. */
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"

static void ready(YellowGame* g,uint8_t map,int x,int y){
 yellow_init(g);g->running=true;g->screen=YScreenWorld;
 g->story=YStoryPikachuFollows;g->has_pikachu=1;
 g->follower_valid=0;g->map=map;g->x=(int8_t)x;g->y=(int8_t)y;
}
static void check_all_b1f_cells(void){
 assert(YMapMtMoonB1F==16 && YELLOW_MTMOON_MAP_COUNT==16);
 assert(yellow_full_scene_asset(YMapMtMoonB1F)==NULL); /* old ABI */
 const YellowLevelAsset* scene=yellow_gameplay_scene_asset(YMapMtMoonB1F);
 assert(scene && scene->w==14 && scene->h==14);
 assert(scene->blocks_count==128 && scene->tiles_count==80);
 assert(yellow_map_w(YMapMtMoonB1F)==28 && yellow_map_h(YMapMtMoonB1F)==28);
 assert(!yellow_gameplay_scene_asset(YMapMtMoonB2F));
 unsigned walkable=0;
 for(int y=0;y<28;y++)for(int x=0;x<28;x++){
  uint8_t block=ymap_mtmoonb1f[(y/2)*14+x/2];
  assert(block<128);
  uint8_t t=yblocks_cavern[(unsigned)block*16u+((y%2)*2+1)*4+(x%2)*2];
  bool expected=(t<80 && ycollision_cavern[t]!=0);
  assert((bool)yellow_mtmoonb1f_passable(x,y)==expected);
  assert(yellow_world_tile_id(YMapMtMoonB1F,x*2,y*2)<80);
  if(expected)walkable++;
 }
 assert(walkable==232);
 assert(!yellow_mtmoonb1f_passable(-1,0));
 assert(!yellow_mtmoonb1f_passable(28,0));
 assert(!yellow_mtmoonb1f_passable(0,28));
 assert(yellow_mtmoonb1f_tile_id(-1,0)==0xff);
 assert(yellow_mtmoonb1f_tile_id(56,0)==0xff);
 assert(yellow_tile_id(YMapMtMoonB1F,0,0)==0xff); /* legacy immutable */
}
static void test_reciprocal_warp_by_input(void){
 YellowGame g;ready(&g,YMapMtMoon1F,5,4);
 yellow_input(&g,YDown);
 assert(g.map==YMapMtMoonB1F && g.x==5 && g.y==5);
 assert(g.screen==YScreenWorld);
 ready(&g,YMapMtMoonB1F,5,6);
 yellow_input(&g,YUp);
 assert(g.map==YMapMtMoon1F && g.x==5 && g.y==5);
}
static void test_unported_stays_unported(void){
 YellowGame g;ready(&g,YMapMtMoonB1F,17,10);
 yellow_input(&g,YDown);
 assert(g.map==YMapMtMoonB1F);
 uint8_t dest,dx,dy;
 assert(!yellow_original_warp_at(YMapMtMoonB1F,17,11,&dest,&dx,&dy));
}
int main(void){
 check_all_b1f_cells();
 test_reciprocal_warp_by_input();
 test_unported_stays_unported();
 puts("R12_CAVERN_B1F_784_COLLISION_196_BLOCKS_GAMEPLAY_PASS");
 return 0;
}
