#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include "yellow_core.h"
#include "pydp_reader.h"
#include "pyts_reader.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
#include "../pydp_reader.c"
#include "../pyts_reader.c"
static int read_at(void* ctx,uint32_t pos,uint8_t* out,size_t n) {
 FILE* f=(FILE*)ctx;
 if(!n || n>128 || fseek(f,(long)pos,SEEK_SET))return 0;
 return fread(out,1,n,f)==n;
}
int main(void) {
 FILE* data=fopen("../../../bulk-pokeyellow-r01/pokeyellow_data.pydb","rb");
 FILE* atlas=fopen("../../../tileset-source-r01/pokeyellow_tilesets.pyts","rb");
 assert(data && atlas);
 PydpPack db;PytAtlas tiles;
 assert(pydp_open(&db,read_at,data)&&pydp_verify(&db));
 assert(pyt_open(&tiles,read_at,atlas)&&pyt_verify(&tiles));
 unsigned checked=0,passable=0;
 const uint8_t native[2]={YMapRoute1,YMapViridianForest};
 const uint8_t originals[2]={12,51};
 const unsigned expected_passable[2]={553,719};
 for(unsigned map_i=0;map_i<2;map_i++) {
  uint8_t map=native[map_i],original=originals[map_i],ts=255;
  PydpMap size;assert(pydp_map(&db,original,&size));
  assert(yellow_map_w(map)==size.width*2);
  assert(yellow_map_h(map)==size.height*2);
  assert(pyt_map_tileset(&tiles,original,&ts));
  YellowGame game;yellow_init(&game);
  game.map=map;game.story=YStoryPikachuFollows;
  game.screen=YScreenWorld;game.has_pikachu=1;
  unsigned local_passable=0;
  for(int y=0;y<yellow_map_h(map);y++)for(int x=0;x<yellow_map_w(map);x++) {
   uint8_t tile=0,allowed=0;
   bool actual=false;
   assert(pyt_map_tile_id(&tiles,&db,original,(uint8_t)(x/2),
                 (uint8_t)(y/2),(uint8_t)((x*2)%4),
                 (uint8_t)((y*2+1)%4),&tile));
   assert(pyt_collision_allowed(&tiles,ts,tile,&allowed));
   assert(yellow_original_collision(map,x,y,&actual));
   assert(actual==(allowed!=0));
   uint8_t old_tile=yellow_tile_id(map,x*2,y*2+1);
   assert(actual==(old_tile<96 && yellow_levels[map].collision[old_tile]!=0));
   assert(base_passable(map,x,y)==actual);
   uint8_t dest=255,dx=255,dy=255;
   bool warp=destination(map,x,y,&dest,&dx,&dy);
   assert(yellow_walkable(&game,x,y)==(warp || (actual && !npc_at(&game,x,y))));
   checked++;
   if(actual){local_passable++;passable++;}
  }
  assert(local_passable==expected_passable[map_i]);
  bool out=true;
  assert(!yellow_original_collision(map,-1,0,&out));
  assert(!yellow_original_collision(map,0,-1,&out));
  assert(!yellow_original_collision(map,yellow_map_w(map),0,&out));
  assert(!yellow_original_collision(map,0,yellow_map_h(map),&out));
 }
 bool state=false;
 assert(!yellow_original_collision(YMapPallet,0,0,&state));
 assert(!yellow_original_collision(YMapRoute1,0,0,NULL));
 assert(checked==2352 && passable==1272);
 YellowGame after;yellow_init(&after);
 after.story=YStoryPikachuFollows;after.map=YMapRoute1;
 after.x=10;after.y=20;after.has_pikachu=1;
 YellowSave previous;yellow_save_create(&after,&previous);
 YellowGame restored;assert(yellow_save_load(&restored,&previous));
 assert(previous.version==13&&restored.map==YMapRoute1);
 assert(restored.x==10&&restored.y==20);
 fclose(data);fclose(atlas);
 puts("ORIGINAL_ROUTE1_FOREST_2352_COLLISION_PARITY_NATIVE_MOVEMENT_SAVE_PASS");
 return 0;
}
