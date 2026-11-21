/* Read-only preflight contract, never returns a physical hardware pass.
 * No USB, NFC, GPIO, disk, radio, firmware or speaker writes. */
#include "yellow_flipper_smoke_contract.h"
#include <string.h>
bool yellow_flipper_smoke_contract(const YellowGame* game,YellowFlipperSmokeContract* out) {
    if(!game||!out)return false;
    if(game->map>=YELLOW_GYM_MAP_COUNT||!yellow_scene_asset(game->map))return false;
    if(game->has_pikachu && !game->pika_level)return false;
    if(sizeof(YellowSave)!=132u || YELLOW_PC_CAPACITY!=20u ||
       YELLOW_DEX_SPECIES!=151u)return false;
    YellowFlipperSmokeContract status={0};
    status.screen_width=128;
    status.screen_height=64;
    status.native_save_v13_bytes=sizeof(YellowSave);
    status.native_scene_identifiers=YELLOW_GYM_MAP_COUNT;
    status.pokedex_names=YELLOW_DEX_SPECIES;
    status.pc_capacity=YELLOW_PC_CAPACITY;
    status.requires_owner_device_approval=true;
    status.physical_flipper_verified=false;
    status.real_input_save_roundtrip_verified=false;
    status.full_original_game_playthrough_verified=false;
    *out=status;
    return true;
}
