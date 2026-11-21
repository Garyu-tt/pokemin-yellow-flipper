#include "pyts_reader.h"
#include <string.h>
#define PYTS_MAP_OFF 16u
#define PYTS_COUNT_OFF 265u
#define PYTS_BLOCK_OFF 290u
#define PYTS_COLL_OFF 51490u
#define PYTS_BYTES 52290u
static uint16_t ts_u16(const uint8_t* v) {return (uint16_t)(v[0]|((uint16_t)v[1]<<8));}
static uint32_t ts_u32(const uint8_t* v) {
 return (uint32_t)v[0]|((uint32_t)v[1]<<8)|((uint32_t)v[2]<<16)|((uint32_t)v[3]<<24);
}
static uint32_t crc32_update(uint32_t crc,const uint8_t* b,size_t n) {
 size_t i;unsigned bit;
 for(i=0;i<n;i++) {
  crc^=b[i];
  for(bit=0;bit<8;bit++)crc=(crc>>1)^(0xedb88320u&(0u-(crc&1u)));
 }
 return crc;
}
int pyt_open(PytAtlas* a,PydpRead cb,void* ctx) {
 uint8_t hdr[16];
 if(!a||!cb)return 0;
 memset(a,0,sizeof(*a));
 if(!cb(ctx,0,hdr,sizeof(hdr)))return 0;
 if(memcmp(hdr,"PYTS",4)!=0||ts_u16(hdr+4)!=1||ts_u16(hdr+6)!=25||
    ts_u16(hdr+8)!=249||ts_u16(hdr+10)!=0)return 0;
 a->read=cb;a->ctx=ctx;a->crc32=ts_u32(hdr+12);a->available=1;
 return 1;
}
int pyt_verify(const PytAtlas* a) {
 uint8_t bytes[128];uint32_t crc=0xffffffffu,where=16,remaining=PYTS_BYTES-16;
 if(!a||!a->available||!a->read)return 0;
 while(remaining) {
  uint32_t n=remaining>sizeof(bytes)?sizeof(bytes):remaining;
  if(!a->read(a->ctx,where,bytes,n))return 0;
  crc=crc32_update(crc,bytes,n);
  where+=n;remaining-=n;
 }
 return (crc^0xffffffffu)==a->crc32;
}
int pyt_map_tileset(const PytAtlas* a,uint8_t map,uint8_t* tileset) {
 uint8_t t;
 if(!a||!a->available||!tileset||map>=249||
    !a->read(a->ctx,PYTS_MAP_OFF+map,&t,1)||t>=25)return 0;
 *tileset=t;return 1;
}
int pyt_block_count(const PytAtlas* a,uint8_t tileset,uint8_t* count) {
 uint8_t n;
 if(!a||!a->available||!count||tileset>=25||
    !a->read(a->ctx,PYTS_COUNT_OFF+tileset,&n,1)||n<1||n>128)return 0;
 *count=n;return 1;
}
int pyt_tile_id(const PytAtlas* a,uint8_t tileset,uint8_t block,
                uint8_t tx,uint8_t ty,uint8_t* id) {
 uint8_t n;
 if(!id||tx>=4||ty>=4||!pyt_block_count(a,tileset,&n)||block>=n)return 0;
 return a->read(a->ctx,PYTS_BLOCK_OFF+(uint32_t)tileset*2048+
                (uint32_t)block*16+(uint32_t)ty*4+tx,id,1);
}
int pyt_map_tile_id(const PytAtlas* a,const PydpPack* maps,
                    uint8_t map,uint8_t bx,uint8_t by,
                    uint8_t tx,uint8_t ty,uint8_t* out) {
 uint8_t tileset=255,block=255;
 if(!out||!pyt_map_tileset(a,map,&tileset)||
    !pydp_map_block_at(maps,map,bx,by,&block))return 0;
 return pyt_tile_id(a,tileset,block,tx,ty,out);
}
int pyt_collision_allowed(const PytAtlas* a,uint8_t tileset,
                          uint8_t tile,uint8_t* allowed) {
 uint8_t bits;
 if(!a||!a->available||!allowed||tileset>=25||
    !a->read(a->ctx,PYTS_COLL_OFF+(uint32_t)tileset*32+tile/8,&bits,1))
    return 0;
 *allowed=(uint8_t)((bits>>(tile&7))&1u);return 1;
}
