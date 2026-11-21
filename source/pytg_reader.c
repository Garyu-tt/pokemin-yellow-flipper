#include "pytg_reader.h"
#include <string.h>
#define PYTG_SIZE 19241u
static uint16_t gfx16(const uint8_t* a) {return (uint16_t)(a[0] | (uint16_t)a[1]<<8);}
static uint32_t gfx32(const uint8_t* a) {
 return (uint32_t)a[0]|((uint32_t)a[1]<<8)|((uint32_t)a[2]<<16)|((uint32_t)a[3]<<24);
}
static uint32_t gfx_crc(uint32_t crc,const uint8_t* b,size_t n) {
 for(size_t i=0;i<n;i++) {
  crc^=b[i];
  for(unsigned j=0;j<8;j++)crc=(crc>>1)^(0xedb88320u & (0u-(crc&1u)));
 }
 return crc;
}
int pytg_open(PytgAtlas* a,PydpRead cb,void* ctx) {
 uint8_t header[16];
 if(!a||!cb)return 0;
 memset(a,0,sizeof(*a));
 if(!cb(ctx,0,header,16))return 0;
 if(memcmp(header,"PYTG",4)||gfx16(header+4)!=1||gfx16(header+6)!=25||
    gfx16(header+8)!=96||gfx16(header+10)!=0)return 0;
 a->read=cb;a->ctx=ctx;a->crc32=gfx32(header+12);a->ready=1;return 1;
}
int pytg_verify(const PytgAtlas* a) {
 uint8_t block[128];uint32_t offset=16,remaining=PYTG_SIZE-16,crc=0xffffffffu;
 if(!a||!a->ready||!a->read)return 0;
 while(remaining) {
  uint32_t n=remaining>sizeof(block)?sizeof(block):remaining;
  if(!a->read(a->ctx,offset,block,n))return 0;
  crc=gfx_crc(crc,block,n);remaining-=n;offset+=n;
 }
 return (crc^0xffffffffu)==a->crc32;
}
int pytg_tile(const PytgAtlas* a,uint8_t tileset,uint8_t tile,uint8_t out[8]) {
 uint8_t count=0;
 if(!a||!a->ready||!out||tileset>=25 ||
    !a->read(a->ctx,16u+tileset,&count,1)||
    count==0||count>96||tile>=count)return 0;
 return a->read(a->ctx,41u+(uint32_t)tileset*768u+(uint32_t)tile*8u,out,8);
}
int pytg_map_tile(const PytgAtlas* gfx,const PytAtlas* tiles,
                  const PydpPack* maps,uint8_t map,uint8_t bx,uint8_t by,
                  uint8_t tx,uint8_t ty,uint8_t out[8]) {
 uint8_t tid=255,tile=255;
 if(!out||!pyt_map_tileset(tiles,map,&tid) ||
    !pyt_map_tile_id(tiles,maps,map,bx,by,tx,ty,&tile))return 0;
 return pytg_tile(gfx,tid,tile,out);
}
