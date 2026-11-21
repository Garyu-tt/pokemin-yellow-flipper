#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "pydp_reader.h"
#include "pyts_reader.h"
#include "pytg_reader.h"
#include "../pydp_reader.c"
#include "../pyts_reader.c"
#include "../pytg_reader.c"
static int from_file(void* ctx,uint32_t offset,uint8_t* out,size_t size) {
 if(!size||size>128||fseek((FILE*)ctx,(long)offset,SEEK_SET))return 0;
 return fread(out,1,size,(FILE*)ctx)==size;
}
int main(void) {
 FILE* db=fopen("../../../bulk-pokeyellow-r01/pokeyellow_data.pydb","rb");
 FILE* ts=fopen("../../../tileset-source-r01/pokeyellow_tilesets.pyts","rb");
 FILE* tg=fopen("../../../tileset-graphics-r01/pokeyellow_tiles_1bpp.pytg","rb");
 assert(db&&ts&&tg);
 PydpPack maps;PytAtlas tiles;PytgAtlas pixels;
 assert(pydp_open(&maps,from_file,db)&&pydp_verify(&maps));
 assert(pyt_open(&tiles,from_file,ts)&&pyt_verify(&tiles));
 assert(pytg_open(&pixels,from_file,tg)&&pytg_verify(&pixels));
 uint8_t out[8]={0},tile=255,block=255,tid=255;
 assert(pytg_map_tile(&pixels,&tiles,&maps,12,0,0,0,0,out));
 const uint8_t route1[8]={170,84,171,17,170,68,170,17};
 assert(!memcmp(out,route1,8));
 assert(pydp_map_block_at(&maps,12,0,0,&block)&&block==10);
 assert(pyt_map_tileset(&tiles,12,&tid)&&tid==0);
 assert(pyt_tile_id(&tiles,tid,block,0,0,&tile)&&tile==44);
 assert(pytg_tile(&pixels,tid,tile,out)&&!memcmp(out,route1,8));
 assert(pytg_map_tile(&pixels,&tiles,&maps,51,0,0,0,0,out));
 const uint8_t forest[8]={42,68,168,7,138,84,144,17};
 assert(!memcmp(out,forest,8));
 assert(pydp_map_block_at(&maps,51,0,0,&block)&&block==55);
 assert(pyt_map_tileset(&tiles,51,&tid)&&tid==3);
 assert(pyt_tile_id(&tiles,tid,block,0,0,&tile)&&tile==2);
 assert(pytg_tile(&pixels,tid,tile,out)&&!memcmp(out,forest,8));
 assert(!pytg_tile(&pixels,11,32,out)); /* 32 original underground tiles */
 assert(!pytg_tile(&pixels,3,96,out));
 assert(!pytg_tile(&pixels,25,0,out));
 assert(!pytg_tile(&pixels,3,0,NULL));
 assert(!pytg_map_tile(&pixels,&tiles,&maps,255,0,0,0,0,out));
 assert(!pytg_map_tile(&pixels,&tiles,&maps,51,0,0,4,0,out));
 fclose(tg);fclose(ts);fclose(db);
 puts("ORIGINAL_SOURCE_1BPP_PIXELS_FOREST_OVERWORLD_AND_BOUNDS_PASS");
 return 0;
}
