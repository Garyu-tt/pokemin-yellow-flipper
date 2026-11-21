#include <assert.h>
#include <stdio.h>
#include "yellow_owner_pewter_source.h"
#include "pydp_reader.h"
#include "pyts_reader.h"
#include "../yellow_owner_pewter_source.c"
#include "../pydp_reader.c"
#include "../pyts_reader.c"
static int source_file(void* context,uint32_t off,uint8_t* dest,size_t n) {
 FILE* fp=(FILE*)context;
 if(!n || n>128 || fseek(fp,(long)off,SEEK_SET))return 0;
 return fread(dest,1,n,fp)==n;
}
int main(void) {
 FILE* maps=fopen("../../../bulk-pokeyellow-r01/pokeyellow_data.pydb","rb");
 FILE* tiles=fopen("../../../tileset-source-r01/pokeyellow_tilesets.pyts","rb");
 assert(maps && tiles);
 PydpPack original;PytAtlas atlas;
 assert(pydp_open(&original,source_file,maps) && pydp_verify(&original));
 assert(pyt_open(&atlas,source_file,tiles) && pyt_verify(&atlas));
 const uint8_t ids[2]={2,54};
 const uint8_t widths[2]={20,5}, heights[2]={18,7};
 const unsigned walk_count[2]={879,90};
 unsigned verified=0;
 for(unsigned i=0;i<2;i++) {
  PydpMap original_map;uint8_t width=0,height=0,ts=255;
  assert(pydp_map(&original,ids[i],&original_map));
  assert(y_pewter_geometry(ids[i],&width,&height));
  assert(width==original_map.width && height==original_map.height);
  assert(width==widths[i] && height==heights[i]);
  assert(pyt_map_tileset(&atlas,ids[i],&ts));
  unsigned allowed_count=0;
  for(unsigned y=0;y<2u*height;y++)for(unsigned x=0;x<2u*width;x++) {
   uint8_t b=255,expected_b=254;
   assert(y_pewter_block_at(ids[i],(uint8_t)(x/2),(uint8_t)(y/2),&b));
   assert(pydp_map_block_at(&original,ids[i],(uint8_t)(x/2),(uint8_t)(y/2),&expected_b));
   assert(b==expected_b);
   uint8_t id=0,allowed=0;
   assert(pyt_map_tile_id(&atlas,&original,ids[i],
      (uint8_t)(x/2),(uint8_t)(y/2),
      (uint8_t)((2*x)%4),(uint8_t)((2*y+1)%4),&id));
   assert(pyt_collision_allowed(&atlas,ts,id,&allowed));
   bool native=false;
   assert(y_pewter_passable(ids[i],(int)x,(int)y,&native));
   assert(native==(allowed!=0));
   allowed_count+=(unsigned)native;
   verified++;
  }
  assert(allowed_count==walk_count[i]);
  bool bad=false;
  assert(!y_pewter_passable(ids[i],-1,0,&bad));
  assert(!y_pewter_passable(ids[i],0,-1,&bad));
  assert(!y_pewter_passable(ids[i],2*width,0,&bad));
  assert(!y_pewter_passable(ids[i],0,2*height,&bad));
  assert(!y_pewter_block_at(ids[i],width,0,&(uint8_t){0}));
  assert(!y_pewter_block_at(ids[i],0,height,&(uint8_t){0}));
 }
 assert(verified==1580);
 uint8_t w=0,h=0;
 assert(!y_pewter_geometry(255,&w,&h));
 assert(!y_pewter_geometry(2,NULL,&h));
 assert(!y_pewter_passable(54,4,4,NULL));
 YPewterWarp warp={0};
 assert(y_pewter_warp_at(2,16,17,&warp) && warp.target_map==54 && warp.target_warp==1);
 assert(y_pewter_warp_at(54,4,13,&warp) && warp.target_map==2 && warp.target_warp==3);
 assert(y_pewter_warp_at(54,5,13,&warp) && warp.target_map==2);
 assert(!y_pewter_warp_at(2,19,5,&warp)); /* Museum not yet implemented */
 assert(!y_pewter_warp_at(2,10,10,&warp));
 assert(!y_pewter_warp_at(54,4,13,NULL));
 assert(!y_pewter_warp_at(54,-1,13,&warp));
 assert(!y_pewter_warp_at(54,257,13,&warp));
 PydpEncounter expected[6];size_t count=0;
 YPewterTrainerParty jr,brock;
 assert(y_pewter_trainer(0,&jr) && jr.trainer_class==4 && jr.party_number==1 && jr.count==2);
 assert(pydp_trainer_party(&original,4,1,expected,6,&count) && count==jr.count);
 for(unsigned i=0;i<count;i++) assert(expected[i].dex==jr.mon[i].dex && expected[i].level==jr.mon[i].level);
 assert(y_pewter_trainer(1,&brock) && brock.trainer_class==33 && brock.party_number==1 && brock.count==2);
 assert(pydp_trainer_party(&original,33,1,expected,6,&count) && count==brock.count);
 for(unsigned i=0;i<count;i++) assert(expected[i].dex==brock.mon[i].dex && expected[i].level==brock.mon[i].level);
 assert(brock.mon[0].dex==74 && brock.mon[0].level==10);
 assert(brock.mon[1].dex==95 && brock.mon[1].level==12);
 assert(!y_pewter_trainer(2,&brock));
 assert(!y_pewter_trainer(1,NULL));
 fclose(maps);fclose(tiles);
 puts("PINNED_PEWTER_1580_TILE_COLLISION_PARITY_3_WARPS_2_PARTIES_PASS");
 return 0;
}
