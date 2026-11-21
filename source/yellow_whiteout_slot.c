#include "yellow_whiteout_slot.h"
static uint32_t add_byte(uint32_t h,uint8_t b){return (h^b)*16777619u;}
static uint32_t add_u32(uint32_t h,uint32_t x){
    for(unsigned i=0;i<4u;i++)h=add_byte(h,(uint8_t)(x>>(8u*i)));
    return h;
}
static uint32_t digest(const YWhiteoutSlot* r){
    uint32_t h=2166136261u;
    h=add_u32(h,r->magic);
    h=add_u32(h,r->generation);
    h=add_u32(h,r->parent_checksum);
    h=add_byte(h,r->original_last_map);
    for(unsigned i=0;i<3u;i++)h=add_byte(h,r->reserved[i]);
    return h;
}
bool y_whiteout_slot_make(uint8_t map,uint32_t parent,uint32_t generation,YWhiteoutSlot* out){
    if(!out || map>YWHITEOUT_SLOT_VIRIDIAN || !parent || !generation)return false;
    YWhiteoutSlot r={0};
    r.magic=YWHITEOUT_SLOT_MAGIC;r.generation=generation;
    r.parent_checksum=parent;r.original_last_map=map;
    r.checksum=digest(&r);r.seal=YWHITEOUT_SLOT_SEAL;
    *out=r;return true;
}
bool y_whiteout_slot_restore(uint8_t* map,uint32_t parent,uint32_t generation,const YWhiteoutSlot* r){
    if(!map || !r || !parent || !generation ||
       r->magic!=YWHITEOUT_SLOT_MAGIC || r->seal!=YWHITEOUT_SLOT_SEAL ||
       r->generation!=generation || r->parent_checksum!=parent ||
       r->original_last_map>YWHITEOUT_SLOT_VIRIDIAN ||
       r->reserved[0] || r->reserved[1] || r->reserved[2] ||
       r->checksum!=digest(r))return false;
    *map=r->original_last_map;return true;
}
