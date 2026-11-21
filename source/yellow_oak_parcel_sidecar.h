#ifndef YELLOW_OAK_PARCEL_SIDECAR_H
#define YELLOW_OAK_PARCEL_SIDECAR_H
#include <stdbool.h>
#include <stdint.h>
/* Original pret/pokeyellow event flags are distinct:
 * EVENT_GOT_OAKS_PARCEL (ViridianMart.asm) and
 * EVENT_OAK_GOT_PARCEL (OaksLab.asm).
 * This is ONLY a bounded persistent prerequisite, not inventory or NPC logic.
 */
#define Y_OAK_PARCEL_MAGIC 0x31504b4fu /* "OKP1" little endian */
#define Y_OAK_PARCEL_VERSION 1u
enum {
 YOakParcelGot = 1u<<0,       /* EVENT_GOT_OAKS_PARCEL */
 YOakParcelDelivered = 1u<<1, /* EVENT_OAK_GOT_PARCEL */
 YOakParcelAll = YOakParcelGot|YOakParcelDelivered
};
typedef struct {
 uint32_t magic;
 uint16_t version;
 uint8_t flags,reserved;
 uint32_t parent_checksum;
 uint32_t checksum;
} YOakParcelSidecar;
bool y_oak_parcel_valid(uint8_t flags);
/* View-only special key item: no grant, no mutation, no inventory fiction. */
bool y_oak_parcel_in_bag(uint8_t flags,bool integrity_error);
bool y_oak_parcel_record_got(uint8_t* flags);
bool y_oak_parcel_record_delivered(uint8_t* flags);
bool y_oak_parcel_save(uint8_t flags,uint32_t parent_checksum,YOakParcelSidecar* out);
bool y_oak_parcel_restore(uint8_t* target,uint32_t parent_checksum,const YOakParcelSidecar* saved);
#endif
