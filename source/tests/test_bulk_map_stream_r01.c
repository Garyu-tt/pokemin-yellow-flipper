#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "pydp_reader.h"
#include "../pydp_reader.c"
static int read_at(void* ctx,uint32_t pos,uint8_t* out,size_t len) {
 FILE* f=(FILE*)ctx;
 if(!len||len>128||fseek(f,(long)pos,SEEK_SET))return 0;
 return fread(out,1,len,f)==len;
}
static void compare_original(const PydpPack* p,uint8_t map_id,const char* name) {
 char path[220];
 snprintf(path,sizeof(path),"../upstream/pokeyellow/maps/%s.blk",name);
 FILE *original=fopen(path,"rb");assert(original);
 PydpMap map;assert(pydp_map(p,map_id,&map));
 assert(map.layout_bytes==(uint16_t)(map.width*map.height));
 for(uint8_t y=0;y<map.height;y++) {
  for(uint8_t x=0;x<map.width;x++) {
   uint8_t block=0;
   assert(pydp_map_block_at(p,map_id,x,y,&block));
   assert((int)block==fgetc(original));
  }
 }
 assert(fgetc(original)==EOF);
 fclose(original);
 uint8_t invalid=0xFF;
 assert(!pydp_map_block_at(p,map_id,map.width,0,&invalid));
 assert(!pydp_map_block_at(p,map_id,0,map.height,&invalid));
}
int main(void) {
 FILE *file=fopen("../../../bulk-pokeyellow-r01/pokeyellow_data.pydb","rb");
 assert(file);
 PydpPack pack;assert(pydp_open(&pack,read_at,file));
 assert(pydp_verify(&pack));
 PydpMap forest;assert(pydp_map(&pack,51,&forest));
 assert(forest.width==17 && forest.height==24 && forest.layout_bytes==408);
 compare_original(&pack,51,"ViridianForest");
 compare_original(&pack,12,"Route1");
 /* Count and validate EVERY source-derived map with dimension-safe geometry. */
 uint16_t valid=0,excluded=0;
 for(unsigned id=0;id<249;id++) {
  PydpMap map;assert(pydp_map(&pack,(uint8_t)id,&map));
  uint8_t value=0;
  int supported=pydp_map_block_at(&pack,(uint8_t)id,0,0,&value);
  if(map.layout_offset==0xffffffffu ||
     map.layout_bytes==0 ||
     (uint32_t)map.width*map.height!=map.layout_bytes) {
   assert(!supported);
   excluded++;
  } else {
   assert(supported);
   assert(pydp_map_block_at(&pack,(uint8_t)id,
        (uint8_t)(map.width-1),(uint8_t)(map.height-1),&value));
   valid++;
  }
 }
 assert(valid==222 && excluded==27); /* 26 missing + 1 mismatched */
 assert(!pydp_map_block_at(&pack,255,0,0,(uint8_t[1]){0}));
 assert(!pydp_map_block_at(&pack,51,0,0,NULL));
 puts("PYDP_222_VERIFIED_MAPS_SINGLE_BLOCK_STREAM_WITH_ORIGINAL_BYTES_PASS");
 fclose(file);return 0;
}
