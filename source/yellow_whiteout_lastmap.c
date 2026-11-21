#include "yellow_whiteout_lastmap.h"

/* Explicit field-by-field little-endian FNV-1a, never struct padding. */
static uint32_t add32(uint32_t hash, uint32_t value) {
    for(unsigned i=0;i<4u;i++) {
        hash ^= (uint8_t)(value>>(8u*i));
        hash *= 16777619u;
    }
    return hash;
}
static uint32_t checksum(const YWhiteoutLastMapRecord* r) {
    uint32_t h=2166136261u;
    h=add32(h,r->magic);
    h=add32(h,(uint32_t)r->version |
              ((uint32_t)r->original_last_map<<8) |
              ((uint32_t)r->reserved[0]<<16) |
              ((uint32_t)r->reserved[1]<<24));
    return add32(h,r->parent_checksum);
}
bool y_whiteout_lastmap_create(uint8_t original_last_map,
                               uint32_t parent_checksum,
                               YWhiteoutLastMapRecord* out) {
    if(!out || original_last_map!=YWHITEOUT_LASTMAP_VIRIDIAN_ORIGINAL ||
       !parent_checksum)return false;
    YWhiteoutLastMapRecord r={0};
    r.magic=YWHITEOUT_LASTMAP_MAGIC;
    r.version=YWHITEOUT_LASTMAP_VERSION;
    r.original_last_map=original_last_map;
    r.parent_checksum=parent_checksum;
    r.checksum=checksum(&r);
    *out=r;
    return true;
}
bool y_whiteout_lastmap_restore(const YWhiteoutLastMapRecord* r,
                                uint32_t expected_parent_checksum,
                                uint8_t* original_last_map_out) {
    if(!r || !original_last_map_out || !expected_parent_checksum ||
       r->magic!=YWHITEOUT_LASTMAP_MAGIC ||
       r->version!=YWHITEOUT_LASTMAP_VERSION ||
       r->original_last_map!=YWHITEOUT_LASTMAP_VIRIDIAN_ORIGINAL ||
       r->reserved[0] || r->reserved[1] ||
       r->parent_checksum!=expected_parent_checksum ||
       r->checksum!=checksum(r))return false;
    *original_last_map_out=r->original_last_map;
    return true;
}
