#ifndef YELLOW_BLACKOUT_DESTINATION_H
#define YELLOW_BLACKOUT_DESTINATION_H
#include <stdbool.h>
#include <stdint.h>
/* Pinned pret/pokeyellow engine/overworld/special_warps.asm:
 * BIT_ESCAPE_WARP routes wLastBlackoutMap via FlyWarpDataPtr.
 * .ViridianCity: fly_warp VIRIDIAN_CITY, 23, 26.
 * NOT the interior VIRIDIAN_POKECENTER. Original map ID is 0x01.
 * This module is PREWORK only; it does not move/heal/debit the player.
 */
typedef struct { uint8_t native_map, x, y; } YellowBlackoutSpawn;
bool yellow_resolve_blackout_destination(uint8_t original_map,
    bool verified_anchor, bool all_party_fainted, bool corrupted,
    uint8_t* out_map);
bool yellow_resolve_blackout_spawn(uint8_t original_map,
    bool verified_anchor, bool all_party_fainted, bool corrupted,
    YellowBlackoutSpawn* out);
#endif
