#include "yellow_oak_parcel_sidecar.h"
#include <string.h>
/* Explicit canonical field-by-field FNV-1a, never hash padding or host endian.
 * The sidecar is NOT a cryptographic signature; this detects accidental
 * corruption and cross-snapshot reuse. The v13 parent must be validated
 * independently by yellow_save_load before this sidecar is applied.
 */
static uint32_t y_oak_parcel_checksum(const YOakParcelSidecar* p) {
 uint32_t h=2166136261u;
 const uint32_t words[3]={p->magic,
   (uint32_t)p->version|((uint32_t)p->flags<<16)|((uint32_t)p->reserved<<24),
   p->parent_checksum};
 for(unsigned i=0;i<3;i++)
  for(unsigned j=0;j<4;j++){
   h^=(uint8_t)(words[i]>>(j*8u));h*=16777619u;
  }
 return h;
}
bool y_oak_parcel_valid(uint8_t flags){
 return (flags & (uint8_t)~YOakParcelAll)==0 &&
        (!(flags&YOakParcelDelivered) || (flags&YOakParcelGot));
}
bool y_oak_parcel_in_bag(uint8_t flags,bool integrity_error){
 /* Gen1 OAKS_PARCEL is a unique key item held only between acquisition
  * and delivery. Never show an untrusted corrupt sidecar as an item. */
 return !integrity_error && y_oak_parcel_valid(flags) && flags==YOakParcelGot;
}
bool y_oak_parcel_record_got(uint8_t* flags){
 if(!flags || !y_oak_parcel_valid(*flags) || *flags!=0)return false;
 *flags=YOakParcelGot;return true;
}
bool y_oak_parcel_record_delivered(uint8_t* flags){
 if(!flags || !y_oak_parcel_valid(*flags) || *flags!=YOakParcelGot)return false;
 *flags=YOakParcelAll;return true;
}
bool y_oak_parcel_save(uint8_t flags,uint32_t parent_checksum,YOakParcelSidecar* out){
 if(!out || !y_oak_parcel_valid(flags))return false;
 YOakParcelSidecar result={0};
 result.magic=Y_OAK_PARCEL_MAGIC;
 result.version=Y_OAK_PARCEL_VERSION;
 result.flags=flags;
 result.parent_checksum=parent_checksum;
 result.checksum=y_oak_parcel_checksum(&result);
 *out=result;return true;
}
bool y_oak_parcel_restore(uint8_t* target,uint32_t parent_checksum,const YOakParcelSidecar* saved){
 if(!target || !saved || saved->magic!=Y_OAK_PARCEL_MAGIC ||
    saved->version!=Y_OAK_PARCEL_VERSION || saved->reserved!=0 ||
    !y_oak_parcel_valid(saved->flags) ||
    saved->parent_checksum!=parent_checksum ||
    saved->checksum!=y_oak_parcel_checksum(saved))return false;
 *target=saved->flags;return true;
}
