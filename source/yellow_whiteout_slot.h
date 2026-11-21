#ifndef YELLOW_WHITEOUT_SLOT_H
#define YELLOW_WHITEOUT_SLOT_H
#include <stdbool.h>
#include <stdint.h>
/* Runtime 0 means unset/legacy, 1 means source-backed Viridian City.
 * Original Gen1 map IDs must NEVER be interpreted as native YellowMap.
 * Separate per-generation record; YellowSave v13 remains unchanged.
 */
enum { YWHITEOUT_SLOT_UNSET=0u, YWHITEOUT_SLOT_VIRIDIAN=1u };
#define YWHITEOUT_SLOT_MAGIC 0x32574c42u
#define YWHITEOUT_SLOT_SEAL  0x4c425732u
typedef struct {
    uint32_t magic;
    uint32_t generation;
    uint32_t parent_checksum;
    uint8_t original_last_map;
    uint8_t reserved[3];
    uint32_t checksum;
    uint32_t seal;
} YWhiteoutSlot;
bool y_whiteout_slot_make(uint8_t map,uint32_t parent,uint32_t generation,YWhiteoutSlot* out);
bool y_whiteout_slot_restore(uint8_t* map,uint32_t parent,uint32_t generation,const YWhiteoutSlot* in);
#endif
