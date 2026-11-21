/* NEW independent runtime test: pinned original Cerulean source
 * ID 3, native map ID 18, safe 8-step Route4 reciprocal boundary.
 * This file did NOT exist in accepted v058; immutable tests unchanged.
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
static uint32_t fingerprint(const uint8_t* p,unsigned n){
 uint32_t h=2166136261u;
 for(unsigned i=0;i<n;i++)h=(h^p[i])*16777619u;
 return h;
}
static void start(YellowGame* g,uint8_t map,int x,int y){
 yellow_init(g);
 g->running=true;g->screen=YScreenWorld;
 g->story=YStoryPikachuFollows;
 g->has_pikachu=0;g->follower_valid=0;
 g->map=map;g->x=(int8_t)x;g->y=(int8_t)y;
}
int main(void){
 assert(YMapCeruleanCity==18);
 assert(YELLOW_CERULEAN_MAP_ID==18);
 assert(YELLOW_MAP_COUNT==11 && sizeof(YellowSave)==132);
 assert(!yellow_full_scene_asset(YMapCeruleanCity));
 const YellowLevelAsset* a=yellow_gameplay_scene_asset(YMapCeruleanCity);
 assert(a&&a->w==20&&a->h==18&&a->blocks_count==128&&a->tiles_count==96);
 assert(yellow_map_w(YMapCeruleanCity)==40&&yellow_map_h(YMapCeruleanCity)==36);
 assert(fingerprint(a->map,360)==0xaf2abdfbu);
 assert(fingerprint(ycollision_cerulean_city,180)==0x110028c8u);
 for(int y=0;y<36;y++)for(int x=0;x<40;x++){
  uint8_t bid=ymap_cerulean_city[(y/2)*20+x/2];
  assert(bid<128);
  uint8_t source_tile=ypewter_overworld_blocks[(unsigned)bid*16+((y%2)*2+1)*4+(x%2)*2];
  bool walkable=source_tile<96&&ypewter_overworld_col[source_tile]!=0;
  assert((bool)yellow_cerulean_city_passable(x,y)==walkable);
  assert(yellow_world_tile_id(YMapCeruleanCity,x*2,y*2)!=0xff);
 }
 assert(!yellow_cerulean_city_passable(-1,0));
 assert(!yellow_cerulean_city_passable(0,-1));
 assert(!yellow_cerulean_city_passable(40,0));
 assert(!yellow_cerulean_city_passable(0,36));
 assert(yellow_world_tile_id(YMapCeruleanCity,80,0)==0xff);
 const uint8_t corridors[]={4,10,11,13,14,15,16,17};
 for(unsigned i=0;i<sizeof(corridors);i++){
  unsigned y=corridors[i];
  assert(yellow_route4_passable(89,y));
  assert(yellow_cerulean_city_passable(0,y+8));
  YellowGame g;start(&g,YMapRoute4,89,y);
  yellow_input(&g,YRight);
  assert(g.map==YMapCeruleanCity&&g.x==0&&g.y==(int)y+8&&g.steps==1);
  yellow_input(&g,YLeft);
  assert(g.map==YMapRoute4&&g.x==89&&g.y==(int)y&&g.steps==2);
 }
 const uint8_t blocked[]={0,1,2,3,5,6,7,8,9,12};
 for(unsigned i=0;i<sizeof(blocked);i++){
  YellowGame g;start(&g,YMapRoute4,89,blocked[i]);
  yellow_input(&g,YRight);
  assert(g.map==YMapRoute4 && g.x==89 && g.steps==0);
 }
 for(unsigned j=0;j<10;j++){
  /* No phantom interior Cerulean native door to an unfinished map. */
  static const uint8_t warp_xy[10][2]={{27,11},{13,15},{19,17},{30,19},{13,25},
     {25,25},{4,11},{27,9},{9,11},{9,9}};
  uint8_t to=255,xx=255,yy=255;
  assert(!destination(YMapCeruleanCity,warp_xy[j][0],warp_xy[j][1],&to,&xx,&yy));
 }
 YellowGame g,loaded;YellowSave parent;start(&g,YMapCeruleanCity,0,12);
 g.has_pikachu=1;
 yellow_save_create(&g,&parent);
 assert(parent.version==13&&parent.map==YMapRoute2&&parent.x==9&&parent.y==0);
 assert(yellow_save_load(&loaded,&parent));
 assert(loaded.map==YMapRoute2&&loaded.x==9&&loaded.y==0);
 puts("MAP001_CERULEAN_SOURCE_1440_COLLISIONS_8_REVERSE_BOUNDARIES_V13_PASS");
 return 0;
}
