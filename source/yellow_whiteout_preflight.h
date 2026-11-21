#ifndef YELLOW_WHITEOUT_PREFLIGHT_H
#define YELLOW_WHITEOUT_PREFLIGHT_H
#include <stdbool.h>
#include <stdint.h>

/* Original map IDs, NOT native YellowMap IDs.  Pinned pret/pokeyellow:
 * constants/map_constants.asm: VIRIDIAN_CITY = $01.
 * engine/events/set_blackout_map.asm records wLastMap (overworld map),
 * NOT VIRIDIAN_POKECENTER = $29.
 */
enum { YWHITEOUT_ORIGINAL_VIRIDIAN_CITY = 0x01u };

typedef struct {
    uint8_t original_last_blackout_map;
    bool requires_verified_warp_anchor;
    bool executable;
} YWhiteoutPreflight;

/* Only source-valid prerequisites, NEVER authorization to warp/heal.
 * No persistent last-heal anchor exists in native YellowSave v13.
 * Returns false and leaves output unchanged when prerequisites fail. */
bool y_whiteout_preflight(uint8_t original_last_blackout_map,
                          uint8_t party_count, bool all_fainted,
                          bool scripted_movement, bool link_connected,
                          YWhiteoutPreflight* out);

/* Original black_out.asm divides the three-byte packed-BCD player money
 * by two, truncating fractional money.  Reject malformed BCD and
 * preserve output on failure.  Pure function; not hooked into gameplay. */
bool y_whiteout_half_money_bcd(const uint8_t money[3], uint8_t out[3]);
#endif
