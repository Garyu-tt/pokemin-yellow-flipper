#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include "pydp_reader.h"
#include "pyts_reader.h"
#include "../pydp_reader.c"
#include "../pyts_reader.c"
static int io(void* f,uint32_t pos,uint8_t* out,size_t n) {
 if(!n||n>128||fseek((FILE*)f,(long)pos,SEEK_SET))return 0;
 return fread(out,1,n,(FILE*)f)==n;
}
static void original_case(const PydpPack* maps,const PytAtlas* atlas,
                          uint8_t id,const char* map_name,const char* block_file,
                          uint8_t expected_tileset) {
 char path[240];
 snprintf(path,sizeof(path),"../upstream/pokeyellow/maps/%s.blk",map_name);
 FILE* original_map=fopen(path,"rb");assert(original_map);
 snprintf(path,sizeof(path),"../upstream/pokeyellow/gfx/blocksets/%s.bst",block_file);
 FILE* original_blocks=fopen(path,"rb");assert(original_blocks);
 PydpMap dims;uint8_t ts=255;
 assert(pydp_map(maps,id,&dims)&&pyt_map_tileset(atlas,id,&ts));
 assert(ts==expected_tileset);
 unsigned cells=(unsigned)dims.width*dims.height;
 for(unsigned index=0;index<cells;index++) {
  if(index!=0&&index!=cells/2&&index!=cells-1)continue;
  uint8_t block=0;
  assert(fseek(original_map,(long)index,SEEK_SET)==0);
  assert(fread(&block,1,1,original_map)==1);
  assert(block<128);
  for(uint8_t y=0;y<4;y++)for(uint8_t x=0;x<4;x++) {
   uint8_t actual=255,expected=255;
   assert(pyt_map_tile_id(atlas,maps,id,
           (uint8_t)(index%dims.width),(uint8_t)(index/dims.width),x,y,&actual));
   assert(fseek(original_blocks,(long)block*16+y*4+x,SEEK_SET)==0);
   assert(fread(&expected,1,1,original_blocks)==1);
   assert(actual==expected);
  }
 }
 fclose(original_map);fclose(original_blocks);
}
int main(void) {
 FILE* world=fopen("../../../bulk-pokeyellow-r01/pokeyellow_data.pydb","rb");
 FILE* atlas=fopen("../../../tileset-source-r01/pokeyellow_tilesets.pyts","rb");
 assert(world&&atlas);
 PydpPack maps;PytAtlas tiles;
 assert(pydp_open(&maps,io,world)&&pydp_verify(&maps));
 assert(pyt_open(&tiles,io,atlas)&&pyt_verify(&tiles));
 original_case(&maps,&tiles,51,"ViridianForest","forest",3);
 original_case(&maps,&tiles,12,"Route1","overworld",0);
 uint8_t tile=255,allowed=255,n=0;
 assert(pyt_block_count(&tiles,1,&n)&&n==19);
 assert(!pyt_tile_id(&tiles,1,19,0,0,&tile));
 assert(!pyt_tile_id(&tiles,3,128,0,0,&tile));
 assert(!pyt_tile_id(&tiles,3,0,4,0,&tile));
 assert(!pyt_tile_id(&tiles,3,0,0,4,&tile));
 assert(!pyt_map_tile_id(&tiles,&maps,255,0,0,0,0,&tile));
 assert(pyt_collision_allowed(&tiles,3,0x1e,&allowed)&&allowed==1);
 assert(pyt_collision_allowed(&tiles,3,0x00,&allowed)&&allowed==0);
 assert(pyt_collision_allowed(&tiles,0,0x00,&allowed)&&allowed==1);
 assert(pyt_collision_allowed(&tiles,0,0xff,&allowed)&&allowed==0);
 unsigned linked=0,missing=0;
 for(unsigned i=0;i<249;i++) {
  uint8_t id=255;
  if(pyt_map_tileset(&tiles,(uint8_t)i,&id)) {
   linked++;assert(id<25);
  } else missing++;
 }
 assert(linked==223&&missing==26);
 fclose(atlas);fclose(world);
 puts("SOURCE_FOREST_OVERWORLD_PYTS_TILES_COLLISION_ID_PASS");
 return 0;
}
