#ifndef YELLOW_OAK_PARCEL_SLOT_H
#define YELLOW_OAK_PARCEL_SLOT_H
#include <stdbool.h>
#include <stdint.h>
#include "yellow_oak_parcel_sidecar.h"

/* Optional, per-generation SD record. Native v13 (132 bytes) and
 * YellowAtomicSnapshot v1 layout are NEVER modified.
 * Write this record to the inactive slot BEFORE sealing its parent.
 * A stale record from a previous generation cannot be applied.
 */
#define Y_OAK_PARCEL_SLOT_MAGIC 0x31534b4fu /* OKS1 */
#define Y_OAK_PARCEL_SLOT_SEAL 0x434b4f53u  /* SOKC */
typedef struct {
 uint32_t magic;
 uint32_t generation;
 YOakParcelSidecar parcel;
 uint32_t checksum;
 uint32_t seal;
} YOakParcelSlot;
bool y_oak_parcel_runtime_safe(uint8_t flags,bool integrity_error);
bool y_oak_parcel_slot_make(uint8_t flags,uint32_t parent_checksum,
                            uint32_t generation,YOakParcelSlot* out);
bool y_oak_parcel_slot_restore(uint8_t* flags,uint32_t parent_checksum,
                               uint32_t generation,const YOakParcelSlot* record);
#endif
