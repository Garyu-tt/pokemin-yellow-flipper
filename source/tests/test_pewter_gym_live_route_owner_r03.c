#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_owner_pewter_source.c"
#include "../yellow_core.c"

static void ready(YellowGame* g,uint8_t map,int x,int y){
 yellow_init(g);g->running=true;g->screen=YScreenWorld;
 g->story=YStoryPikachuFollows;g->has_pikachu=0;g->follower_valid=0;
 g->map=map;g->x=(int8_t)x;g->y=(int8_t)y;
}
static void assert_source_tiles_and_walls(void){
 const YellowLevelAsset* a=yellow_scene_asset(YMapPewterGym);
 assert(YELLOW_MAP_COUNT==11 && YELLOW_GYM_MAP_COUNT==13);
 assert(YMapPewterGym==12 && a && a->w==5 && a->h==7);
 assert(yellow_map_w(YMapPewterGym)==10 && yellow_map_h(YMapPewterGym)==14);
 assert(yellow_tile_id(YELLOW_MAP_COUNT,0,0)==0xff);
 for(int y=0;y<7;y++)for(int x=0;x<5;x++){
  uint8_t block=255;assert(y_pewter_block_at(54,x,y,&block));
  assert(a->map[y*5+x]==block);
 }
 for(int y=0;y<14;y++)for(int x=0;x<10;x++){
  bool source=false;assert(y_pewter_passable(54,x,y,&source));
  assert((bool)yellow_pewter_gym_passable(x,y)==source);
 }
 for(int y=0;y<28;y++)for(int x=0;x<20;x++)
  assert(yellow_world_tile_id(YMapPewterGym,x,y)<a->tiles_count);
 assert(yellow_world_tile_id(YMapPewterGym,-1,0)==0xff);
 assert(yellow_world_tile_id(YMapPewterGym,20,0)==0xff);
 assert(!yellow_pewter_gym_passable(-1,2));
 assert(!yellow_pewter_gym_passable(10,12));
 assert(!yellow_pewter_gym_passable(0,0));
}
static void test_source_warps_and_actual_gameplay(void){
 YPewterWarp source;
 assert(y_pewter_warp_at(2,16,17,&source));
 assert(source.target_map==54 && source.target_warp==1);
 assert(y_pewter_warp_at(54,4,13,&source));
 assert(source.target_map==2 && source.target_warp==3);
 assert(y_pewter_warp_at(54,5,13,&source));
 YellowGame g;ready(&g,YMapPewterCity,16,18);
 yellow_input(&g,YUp);
 assert(g.map==YMapPewterGym && g.x==4 && g.y==12);
 assert(g.steps==1 && g.screen==YScreenWorld);
 yellow_input(&g,YUp);
 assert(g.map==YMapPewterGym && g.x==4 && g.y==11 && g.steps==2);
 yellow_input(&g,YDown);
 assert(g.map==YMapPewterGym && g.y==12 && g.steps==3);
 yellow_input(&g,YDown);
 assert(g.map==YMapPewterCity && g.x==16 && g.y==18 && g.steps==4);
 ready(&g,YMapPewterCity,17,18);
 yellow_input(&g,YUp);
 assert(g.map==YMapPewterCity && g.x==17 && g.y==18);
 ready(&g,YMapPewterGym,4,12);
 yellow_input(&g,YLeft);
 assert(g.map==YMapPewterGym && g.y==12);
}
static void test_legacy_v13_save_safe_fallback(void){
 YellowGame g,loaded;YellowSave s;
 ready(&g,YMapPewterGym,4,12);
 g.has_pikachu=1;g.follower_valid=1;g.follower_map=YMapPewterGym;
 g.follower_x=4;g.follower_y=11;
 yellow_save_create(&g,&s);
 assert(s.version==13 && s.map==YMapRoute2 && s.x==9 && s.y==0);
 assert(!s.follower_valid && s.follower_map==YMapRoute2);
 assert(yellow_save_load(&loaded,&s));
 assert(loaded.map==YMapRoute2 && loaded.x==9 && loaded.y==0);
 /* Unsupported direct gym ID must still be rejected by the v13 loader. */
 s.map=YMapPewterGym;
 assert(!yellow_save_load(&loaded,&s));
}
int main(void){
 assert_source_tiles_and_walls();
 test_source_warps_and_actual_gameplay();
 test_legacy_v13_save_safe_fallback();
 puts("PEWTER_GYM_ORIGINAL_SOURCE_RENDER_WARPS_RUNTIME_AND_SAVE_COMPAT_PASS");
 return 0;
}
