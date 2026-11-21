#include "yellow_oak_parcel_slot.h"
#include <string.h>
/* Canonical field serialization, no struct padding or native endian hashing. */
static uint32_t oak_slot_byte(uint32_t h,uint8_t b){return (h^(uint32_t)b)*16777619u;}
static uint32_t oak_slot_u32(uint32_t h,uint32_t x){
 for(unsigned j=0;j<4;j++)h=oak_slot_byte(h,(uint8_t)(x>>(8u*j)));
 return h;
}
static uint32_t oak_slot_checksum(const YOakParcelSlot* p){
 uint32_t h=2166136261u;
 h=oak_slot_u32(h,p->magic);
 h=oak_slot_u32(h,p->generation);
 h=oak_slot_u32(h,p->parcel.magic);
 h=oak_slot_byte(h,(uint8_t)p->parcel.version);
 h=oak_slot_byte(h,(uint8_t)(p->parcel.version>>8u));
 h=oak_slot_byte(h,p->parcel.flags);
 h=oak_slot_byte(h,p->parcel.reserved);
 h=oak_slot_u32(h,p->parcel.parent_checksum);
 h=oak_slot_u32(h,p->parcel.checksum);
 return h;
}
bool y_oak_parcel_runtime_safe(uint8_t flags,bool integrity_error){
 return !integrity_error && y_oak_parcel_valid(flags);
}
bool y_oak_parcel_slot_make(uint8_t flags,uint32_t parent_checksum,
                            uint32_t generation,YOakParcelSlot* out){
 if(!out || !generation)return false;
 YOakParcelSlot tmp={0};
 tmp.magic=Y_OAK_PARCEL_SLOT_MAGIC;
 tmp.generation=generation;
 if(!y_oak_parcel_save(flags,parent_checksum,&tmp.parcel))return false;
 tmp.checksum=oak_slot_checksum(&tmp);
 tmp.seal=Y_OAK_PARCEL_SLOT_SEAL;
 *out=tmp;
 return true;
}
bool y_oak_parcel_slot_restore(uint8_t* flags,uint32_t parent_checksum,
                               uint32_t generation,const YOakParcelSlot* record){
 if(!flags || !record || !generation ||
    record->magic!=Y_OAK_PARCEL_SLOT_MAGIC ||
    record->generation!=generation ||
    record->seal!=Y_OAK_PARCEL_SLOT_SEAL ||
    record->checksum!=oak_slot_checksum(record))return false;
 uint8_t verified=0;
 if(!y_oak_parcel_restore(&verified,parent_checksum,&record->parcel))return false;
 *flags=verified;
 return true;
}
