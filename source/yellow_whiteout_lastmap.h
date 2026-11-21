#ifndef YELLOW_WHITEOUT_LASTMAP_H
#define YELLOW_WHITEOUT_LASTMAP_H
#include <stdbool.h>
#include <stdint.h>

/* Prework ONLY: source-backed original wLastBlackoutMap persistence.
 * This is NOT a native YellowMap coordinate, warp, or full whiteout recovery.
 * Versioned fixed-field checksum avoids host padding/endian dependence.
 */
enum { YWHITEOUT_LASTMAP_MAGIC = 0x59424c4du,
       YWHITEOUT_LASTMAP_VERSION = 1u,
       YWHITEOUT_LASTMAP_VIRIDIAN_ORIGINAL = 0x01u };
typedef struct {
    uint32_t magic;
    uint8_t version;
    uint8_t original_last_map;
    uint8_t reserved[2];
    uint32_t parent_checksum;
    uint32_t checksum;
} YWhiteoutLastMapRecord;

/* Only original Viridian City 0x01 is independently pinned for this prework.
 * Do not silently treat original map IDs as native YellowMap enum values. */
bool y_whiteout_lastmap_create(uint8_t original_last_map,
                               uint32_t parent_checksum,
                               YWhiteoutLastMapRecord* out);
bool y_whiteout_lastmap_restore(const YWhiteoutLastMapRecord* record,
                                uint32_t expected_parent_checksum,
                                uint8_t* original_last_map_out);
#endif
