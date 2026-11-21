#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_owner_pewter_source.c"
#include "../yellow_core.c"

static void ready(YellowGame *g, uint8_t map, uint8_t x, uint8_t y) {
 yellow_init(g);
 g->running=true;
 g->screen=YScreenWorld;
 g->story=YStoryPikachuFollows;
 g->has_pikachu=0;
 g->follower_valid=0;
 g->map=map; g->x=x; g->y=y;
}
static void original_city_data(void) {
 assert(YELLOW_MAP_COUNT==11 && YELLOW_LEVELS_COUNT==11);
 assert(YELLOW_PLAYABLE_MAP_COUNT==12 && YMapPewterCity==11);
 assert(yellow_tile_id(YELLOW_MAP_COUNT,0,0)==0xff);
 const YellowLevelAsset* level=yellow_scene_asset(YMapPewterCity);
 assert(level && level->w==20 && level->h==18);
 assert(yellow_map_w(YMapPewterCity)==40 && yellow_map_h(YMapPewterCity)==36);
 for(int y=0;y<18;y++)for(int x=0;x<20;x++){
   uint8_t block=0;
   assert(y_pewter_block_at(2,x,y,&block));
   assert(block==level->map[y*20+x]);
 }
 for(int y=0;y<36;y++)for(int x=0;x<40;x++){
   bool allowed=0;
   assert(y_pewter_passable(2,x,y,&allowed));
   assert((bool)yellow_pewter_city_passable(x,y)==allowed);
 }
 assert(!yellow_scene_asset(20));
 assert(!yellow_pewter_city_passable(-1,0));
 assert(!yellow_pewter_city_passable(40,35));
 assert(yellow_world_tile_id(YMapPewterCity,-1,0)==0xff);
 assert(yellow_world_tile_id(YMapPewterCity,80,0)==0xff);
 for(int y=0;y<72;y++)for(int x=0;x<80;x++)
   assert(yellow_world_tile_id(YMapPewterCity,x,y)<level->tiles_count);
}
static void route2_north_approach_is_walkable(void) {
 /* Viridian Forest's north gate deposits the player on Route 2 at (3,10).
    Require an actual connected approach to the Pewter north boundary. */
 YellowGame g;
 ready(&g,YMapRoute2,3,10);
 enum {W=20,H=72,N=W*H};
 uint8_t seen[N]={0};
 uint16_t queue[N];
 unsigned start=10*W+3,goal=9;
 unsigned head=0,tail=0;
 queue[tail++]=(uint16_t)start;
 seen[start]=1;
 while(head<tail && !seen[goal]) {
   unsigned cell=queue[head++];int x=(int)(cell%W),y=(int)(cell/W);
   static const int8_t dx[]={0,0,-1,1},dy[]={-1,1,0,0};
   for(unsigned k=0;k<4;k++) {
     int nx=x+dx[k],ny=y+dy[k];
     uint8_t dst=0,wx=0,wy=0;
     if(nx<0||ny<0||nx>=W||ny>=H)continue;
     unsigned next=(unsigned)ny*W+(unsigned)nx;
     if(seen[next]||!yellow_walkable(&g,nx,ny) ||
        destination(YMapRoute2,nx,ny,&dst,&wx,&wy))continue;
     seen[next]=1;queue[tail++]=(uint16_t)next;
     assert(tail<=N);
   }
 }
 assert(seen[goal]);
}
static void source_route2_to_pewter_gameplay(void) {
 YellowGame g;
 ready(&g,YMapRoute2,9,0);
 yellow_input(&g,YUp);
 assert(g.map==YMapPewterCity && g.x==19 && g.y==35);
 assert(g.screen==YScreenWorld && g.steps==1);
 assert(yellow_walkable(&g,g.x,g.y));
 yellow_input(&g,YUp);
 assert(g.map==YMapPewterCity && g.x==19 && g.y==34 && g.steps==2);
 yellow_input(&g,YDown);
 assert(g.map==YMapPewterCity && g.x==19 && g.y==35 && g.steps==3);
 yellow_input(&g,YDown);
 assert(g.map==YMapRoute2 && g.x==9 && g.y==0 && g.steps==4);
 ready(&g,YMapRoute2,8,0);
 yellow_input(&g,YUp);
 assert(g.map==YMapRoute2 && g.y==0);
 ready(&g,YMapRoute2,9,0);
 g.story=YStoryBeforeOak;
 yellow_input(&g,YUp);
 assert(g.map==YMapRoute2);
}
static void save_in_city_resumes_at_gate_without_corruption(void) {
 YellowGame g,loaded;
 ready(&g,YMapPewterCity,19,34);
 g.has_pikachu=1;
 g.follower_valid=1;g.follower_map=YMapPewterCity;
 g.follower_x=19;g.follower_y=35;
 YellowSave out;
 yellow_save_create(&g,&out);
 assert(out.map==YMapRoute2 && out.x==9 && out.y==0);
 assert(!out.follower_valid && out.follower_map==YMapRoute2);
 assert(yellow_save_load(&loaded,&out));
 assert(loaded.map==YMapRoute2 && loaded.x==9 && loaded.y==0);
 /* Legacy loader must still reject a forged map-ID 11 v13 save. */
 out.map=YMapPewterCity;
 assert(!yellow_save_load(&loaded,&out));
}
int main(void) {
 original_city_data();
 save_in_city_resumes_at_gate_without_corruption();
 route2_north_approach_is_walkable();
 source_route2_to_pewter_gameplay();
 puts("PEWTER_CITY_SOURCE_PLAYABLE_ROUTE_AND_LEGACY_CONTRACT_PASS");
 return 0;
}
