#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "yellow_source_warp_graph.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
static void validate_edges(void) {
 unsigned per[11]={0};
 YOriginalWarp w;
 assert(yellow_original_warp_count()==28);
 unsigned char links[11][11]={{0}};
 /* Existing six native outdoor connections remain source-validated. */
 const uint8_t outdoor[6][2]={{0,4},{4,5},{5,7},{4,0},{5,4},{7,5}};
 for(unsigned i=0;i<6;i++)links[outdoor[i][0]][outdoor[i][1]]=1;
 for(uint8_t i=0;i<yellow_original_warp_count();i++) {
  assert(yellow_original_warp_record(i,&w));
  assert(w.src<YELLOW_MAP_COUNT && w.target<YELLOW_MAP_COUNT);
  assert(w.x<yellow_map_w(w.src) && w.y<yellow_map_h(w.src));
  assert(w.target_x<yellow_map_w(w.target) && w.target_y<yellow_map_h(w.target));
  assert(w.source_warp_id>=1 && w.target_warp_id>=1);
  uint8_t dest=255,x=255,y=255;
  assert(destination(w.src,w.x,w.y,&dest,&x,&y));
  assert(dest==w.target && x==w.target_x && y==w.target_y);
  assert(yellow_original_warp_at(w.src,w.x,w.y,&dest,&x,&y));
  YellowGame game;yellow_init(&game);
  game.map=w.src;game.story=YStoryPikachuFollows;
  game.has_pikachu=1;
  assert(yellow_walkable(&game,w.x,w.y));
  per[w.src]++;
  links[w.src][w.target]=1;
 }
 const uint8_t counts[11]={3,2,2,2,0,1,2,2,6,4,4};
 for(unsigned i=0;i<11;i++)assert(per[i]==counts[i]);
 /* Graph contains all 11 already supported native maps, bidirectionally
    accessible from Pallet. This is NOT the whole original world map. */
 uint16_t visited=1,prior=0;
 while(visited!=prior) {
  prior=visited;
  for(unsigned i=0;i<11;i++)if(visited&(1u<<i))
   for(unsigned k=0;k<11;k++)if(links[i][k])visited|=(uint16_t)(1u<<k);
 }
 assert(visited==((1u<<11)-1));
 assert(!yellow_original_warp_record(28,&w));
 assert(!yellow_original_warp_record(0,NULL));
 uint8_t dest=70,x=80,y=90;
 assert(!destination(YMapRoute1,0,0,&dest,&x,&y));
 assert(!destination(YMapRedsHouse,7,1,&dest,&x,&y));
 assert(!destination(YMapViridian,29,19,&dest,&x,&y));
 assert(!destination(YMapRoute2,12,9,&dest,&x,&y));
 assert(!yellow_original_warp_at(255,0,0,&dest,&x,&y));
 assert(!yellow_original_warp_at(YMapPallet,-1,5,&dest,&x,&y));
 assert(!yellow_original_warp_at(YMapPallet,5,5,NULL,&x,&y));
 assert(dest==70 && x==80 && y==90);
}
static void test_native_interaction(void) {
 YellowGame g;yellow_init(&g);
 g.story=YStoryPikachuFollows;g.has_pikachu=1;
 g.screen=YScreenWorld;g.map=YMapPallet;g.x=5;g.y=6;g.follower_valid=0;
 g.facing=YUp;
 uint16_t old=g.steps;
 yellow_input(&g,YUp);
 assert(g.map==YMapRedsHouse && g.x==2 && g.y==6 && g.steps==old+1);
 /* Round trip through the exact pair of original source warp coordinates. */
 g.x=2;g.y=6;g.follower_valid=0;g.screen=YScreenWorld;g.facing=YDown;
 yellow_input(&g,YDown);
 assert(g.map==YMapPallet && g.x==5 && g.y==6);
 YellowSave snap;yellow_save_create(&g,&snap);
 YellowGame recovered;
 assert(snap.version==13 && yellow_save_load(&recovered,&snap));
 assert(recovered.map==YMapPallet && recovered.x==5 && recovered.y==6);
}
static void source_route2_signs(void) {
 YellowGame g;yellow_init(&g);
 g.screen=YScreenWorld;g.story=YStoryPikachuFollows;
 g.has_pikachu=1;g.map=YMapRoute2;g.follower_valid=0;
 g.x=5;g.y=66;g.facing=YUp;
 YellowEventIR found={0};
 assert(yellow_ir_at_position(YMapRoute2,5,65,YEIR_BG,&found));
 yellow_input(&g,YOk);
 assert(g.screen==YScreenDialog && g.dialog_count==2);
 assert(!strcmp(g.dialog_lines[0],"ROUTE 2"));
 assert(!strcmp(g.dialog_lines[1],"VIRIDIAN CITY -"));
 assert(!strcmp(g.dialog_lines[2],"PEWTER CITY"));
 g.screen=YScreenWorld;g.dialog_count=0;
 g.x=11;g.y=12;g.facing=YUp;g.follower_valid=0;
 assert(yellow_ir_at_position(YMapRoute2,11,11,YEIR_BG,&found));
 yellow_input(&g,YOk);
 assert(g.screen==YScreenDialog && g.dialog_count==1);
 assert(!strcmp(g.dialog_lines[0],"DIGLETT'S CAVE"));
}
int main(void) {
 validate_edges();test_native_interaction();source_route2_signs();
 puts("PINNED_28_ORIGINAL_WARPS_ALL_11_NATIVE_MAPS_CONNECTED_SAVE_V13_PASS");
 return 0;
}
