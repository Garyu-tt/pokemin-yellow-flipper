#ifndef YELLOW_FLIPPER_SMOKE_CONTRACT_H
#define YELLOW_FLIPPER_SMOKE_CONTRACT_H
#include <stdint.h>
#include <stdbool.h>
typedef struct {
    uint16_t screen_width;
    uint16_t screen_height;
    uint16_t native_save_v13_bytes;
    uint16_t native_scene_identifiers;
    uint16_t pokedex_names;
    uint16_t pc_capacity;
    bool requires_owner_device_approval;
    bool physical_flipper_verified;
    bool real_input_save_roundtrip_verified;
    bool full_original_game_playthrough_verified;
} YellowFlipperSmokeContract;
/* Read-only sanity probe. Not a substitute for any physical device test. */
bool yellow_flipper_smoke_contract(const YellowGame* game,YellowFlipperSmokeContract* out);
#endif
