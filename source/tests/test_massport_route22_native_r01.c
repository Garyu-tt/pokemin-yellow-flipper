/* Route22 MAP003 independent gameplay QA. Original source ID33/native20. */
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
static uint32_t fnv(const uint8_t* data,unsigned n){
 uint32_t h=2166136261u;
 for(unsigned i=0;i<n;i++)h=(h^data[i])*16777619u;
 return h;
}
static void start(YellowGame* g,uint8_t map,int x,int y){
 yellow_init(g);g->screen=YScreenWorld;g->running=true;
 g->story=YStoryPikachuFollows;g->has_pikachu=0;
 g->follower_valid=0;g->map=map;g->x=(int8_t)x;g->y=(int8_t)y;
}
int main(void){
 assert(YMapViridian==5&&YMapRoute9==19&&YMapRoute22==20);
 assert(YELLOW_ROUTE22_MAP_ID==20 && YELLOW_MAP_COUNT==11);
 assert(sizeof(YellowSave)==132 && sizeof(YellowExactPositionV1)==28);
 const YellowLevelAsset* a=yellow_gameplay_scene_asset(YMapRoute22);
 assert(a&&a->w==20&&a->h==9&&a->tiles_count==96&&a->blocks_count==128);
 assert(yellow_map_w(YMapRoute22)==40&&yellow_map_h(YMapRoute22)==18);
 assert(fnv(ymap_route22,180)==0x44d919adu);
 assert(fnv(ycollision_route22,90)==0xb26423e5u);
 uint32_t tex_hash=2166136261u;
 for(int y=0;y<36;y++)for(int x=0;x<80;x++){
  int block=ymap_route22[(y/4)*20+x/4];
  int expected=ypewter_overworld_blocks[(unsigned)block*16+(y%4)*4+x%4];
  int actual=yellow_world_tile_id(YMapRoute22,x,y);
  assert(actual==expected);
  tex_hash=(tex_hash^(unsigned)actual)*16777619u;
 }
 assert(tex_hash==0xcc94be17u);
 assert(yellow_world_tile_id(YMapRoute22,-1,0)==0xff);
 assert(yellow_world_tile_id(YMapRoute22,80,0)==0xff);
 unsigned passes=0;
 for(int y=0;y<18;y++)for(int x=0;x<40;x++){
  int tile=ypewter_overworld_blocks[
    (unsigned)ymap_route22[(y/2)*20+x/2]*16+
    ((y%2)*2+1)*4+(x%2)*2];
  int expected=tile<96&&ypewter_overworld_col[tile]!=0;
  int actual=yellow_route22_passable(x,y);
  assert(actual==expected);passes+=actual;
 }
 assert(passes==290);
 assert(!yellow_route22_passable(-1,0));
 assert(!yellow_route22_passable(40,0));
 const int open[]={14,15,16,17};
 for(unsigned i=0;i<4;i++){
  YellowGame g;int y=open[i];start(&g,YMapViridian,0,y);
  yellow_input(&g,YLeft);
  assert(g.map==YMapRoute22&&g.x==39&&g.y==y-8&&g.steps==1);
  yellow_input(&g,YRight);
  assert(g.map==YMapViridian&&g.x==0&&g.y==y&&g.steps==2);
 }
 const int blocked[]={8,9,10,11,12,13,18,19,20,21,22,23,24,25};
 for(unsigned i=0;i<14;i++){
  YellowGame g;start(&g,YMapViridian,0,blocked[i]);
  yellow_input(&g,YLeft);
  assert(g.map==YMapViridian&&g.x==0&&g.steps==0);
 }
 YellowGame g,restored;YellowAtomicSnapshot snap={0};
 YellowExactPositionV1 ex={0};
 start(&g,YMapRoute22,39,6);
 g.has_pikachu=1;g.pika_level=8;
 g.hp=y_calc_stats(YK_PIKACHU,8).hp;
 g.experience=y_exp_threshold(YK_PIKACHU,8);
 assert(yellow_atomic_make(&g,44,&snap));
 assert(snap.parent.map==YMapRoute2&&snap.parent.version==13);
 assert(yellow_exact_position_make(&g,&snap,&ex));
 assert(yellow_atomic_unpack(&snap,&restored));
 assert(restored.map==YMapRoute2);
 assert(yellow_exact_position_restore(&restored,&snap,&ex));
 assert(restored.map==YMapRoute22&&restored.x==39&&restored.y==6);
 puts("MAP003_ROUTE22_SOURCE_2880_TILES_720_COLLISIONS_4_LINKS_V13_SAVE_PASS");
 return 0;
}
