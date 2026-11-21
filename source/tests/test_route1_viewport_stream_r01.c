#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "yellow_route1_viewport.h"
#include "pydp_reader.h"
#include "pyts_reader.h"
#include "pytg_reader.h"
#include "../pydp_reader.c"
#include "../pyts_reader.c"
#include "../pytg_reader.c"
typedef struct { PydpPack pack; PytAtlas tileset_atlas;
                 PytgAtlas graphics_atlas; bool graphics_ready; } Fixture;
static bool host_pixel_read(void* ctx,uint16_t col,uint16_t row,
                            uint8_t original[8]) {
 Fixture* m=(Fixture*)ctx;
 if(!m || !m->graphics_ready)return false;
 return pytg_map_tile(&m->graphics_atlas,&m->tileset_atlas,&m->pack,
                     12,(uint8_t)(col/4),(uint8_t)(row/4),
                     (uint8_t)(col%4),(uint8_t)(row%4),original)!=0;
}
#include "../yellow_route1_viewport.c"
static int source_read(void* ctx,uint32_t offset,uint8_t* buf,size_t size) {
 FILE* f=(FILE*)ctx;
 if(!size||size>128||fseek(f,(long)offset,SEEK_SET))return 0;
 return fread(buf,1,size,f)==size;
}
int main(void) {
 FILE* world=fopen("../../../bulk-pokeyellow-r01/pokeyellow_data.pydb","rb");
 FILE* atlas=fopen("../../../tileset-source-r01/pokeyellow_tilesets.pyts","rb");
 FILE* pixels=fopen("../../../tileset-graphics-r01/pokeyellow_tiles_1bpp.pytg","rb");
 assert(world&&atlas&&pixels);
 Fixture m={0};
 assert(pydp_open(&m.pack,source_read,world) && pydp_verify(&m.pack));
 assert(pyt_open(&m.tileset_atlas,source_read,atlas) && pyt_verify(&m.tileset_atlas));
 assert(pytg_open(&m.graphics_atlas,source_read,pixels) && pytg_verify(&m.graphics_atlas));
 m.graphics_ready=true;
 uint8_t tile[8]={0};
 assert(yellow_route1_tile(host_pixel_read,&m,0,0,tile));
 static const uint8_t original_reversed[8]={85,42,213,136,85,34,85,136};
 assert(!memcmp(tile,original_reversed,8));
 assert(!yellow_route1_tile(host_pixel_read,&m,40,0,tile));
 assert(!yellow_route1_tile(host_pixel_read,&m,0,72,tile));
 assert(!yellow_route1_tile(NULL,NULL,0,0,tile));
 assert(!yellow_route1_tile(host_pixel_read,&m,0,0,NULL));
 YellowRoute1Viewport cache={0};
 assert(yellow_route1_cached_tile(&cache,0,0)==NULL);
 assert(!yellow_route1_prefetch(&cache,host_pixel_read,&m,0,0,40,72,0));
 assert(yellow_route1_cached_tile(&cache,0,0)==NULL);
 for(unsigned k=0;k<16;k++) {
  if(yellow_route1_prefetch(&cache,host_pixel_read,&m,0,0,40,72,12))break;
 }
 assert(yellow_route1_prefetch(&cache,host_pixel_read,&m,0,0,40,72,12));
 const uint8_t* rendered=yellow_route1_cached_tile(&cache,0,0);
 assert(rendered && !memcmp(rendered,original_reversed,8));
 assert(yellow_route1_cached_tile(&cache,17,0)==NULL);
 assert(!yellow_route1_prefetch(&cache,host_pixel_read,&m,1,0,40,72,12));
 assert(yellow_route1_prefetch(&cache,host_pixel_read,&m,1,0,40,72,12));
 assert(yellow_route1_cached_tile(&cache,17,0)!=NULL);
 assert(yellow_route1_cached_tile(&cache,0,0)==NULL);
 m.graphics_ready=false;
 assert(!yellow_route1_tile(host_pixel_read,&m,0,0,tile));
 assert(!yellow_route1_tile(host_pixel_read,&m,0,0,tile));
 fclose(world);fclose(atlas);fclose(pixels);
 puts("ROUTE1_NATIVE_VIEWPORT_ORIGINAL_PYTG_CACHE_FALLBACK_PASS");
 return 0;
}
