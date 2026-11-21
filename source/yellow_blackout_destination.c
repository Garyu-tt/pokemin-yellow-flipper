#include "yellow_blackout_destination.h"
#include "yellow_core.h"
#include <stddef.h>

/* Original VIRIDIAN_CITY=0x01 is not a native enum. Only this city's
 * original fly-warp coordinates are source-pinned for the bounded prework.
 * Source: engine/events/black_out.asm -> special_warps.asm (BIT_ESCAPE_WARP)
 * -> data/maps/special_warps.asm (.ViridianCity: 23,26).
 */
static bool valid(uint8_t map, bool anchor, bool fainted, bool corrupted) {
    return map==0x01u && anchor && fainted && !corrupted;
}
bool yellow_resolve_blackout_destination(uint8_t map, bool anchor,
    bool fainted, bool corrupted, uint8_t* out_map) {
    if(!out_map || !valid(map,anchor,fainted,corrupted))return false;
    *out_map=(uint8_t)YMapViridian;
    return true;
}
bool yellow_resolve_blackout_spawn(uint8_t map, bool anchor,
    bool fainted, bool corrupted, YellowBlackoutSpawn* out) {
    if(!out || !valid(map,anchor,fainted,corrupted))return false;
    const YellowBlackoutSpawn spawn={(uint8_t)YMapViridian,23u,26u};
    *out=spawn;
    return true;
}
