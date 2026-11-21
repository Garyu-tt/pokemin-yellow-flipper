#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
static void check_original_scope_and_negative_release(void) {
    assert(!yellow_original_game_gaps(NULL));
    YellowOriginalGameGaps status={0};
    assert(yellow_original_game_gaps(&status));
    assert(status.original_map_identifiers==249);
    assert(status.original_map_geometries==227);
    assert(status.registered_native_map_scenes==13);
    assert(status.original_species==151);
    assert(status.native_battle_species==13);
    assert(status.original_moves==165);
    assert(status.native_move_info_entries==21);
    assert(status.pokedex_slots==151 && status.pc_slots==20);
    assert(status.pinned_pallet_ch1_excerpt_notes==48);
    assert(!status.original_story_to_credits_verified);
    assert(!status.original_all_maps_events_verified);
    assert(!status.full_original_multichannel_audio_verified);
    assert(!status.original_game_mechanics_parity_verified);
    assert(!status.physical_flipper_full_playthrough_verified);
    assert(!yellow_original_release_ready(&status));
    assert(!yellow_original_release_ready(NULL));
    status.original_story_to_credits_verified=true;
    status.original_all_maps_events_verified=true;
    status.full_original_multichannel_audio_verified=true;
    status.original_game_mechanics_parity_verified=true;
    status.physical_flipper_full_playthrough_verified=true;
    status.registered_native_map_scenes=249;
    status.native_battle_species=151;
    status.native_move_info_entries=165;
    assert(!yellow_original_release_ready(&status));
}
static void existing_save_and_world_stay_unaffected(void) {
    YellowGame g;yellow_init(&g);
    assert(sizeof(YellowSave)==132);
    YellowSave a,b;
    yellow_save_create(&g,&a);
    YellowOriginalGameGaps audit;
    assert(yellow_original_game_gaps(&audit));
    yellow_save_create(&g,&b);
    assert(a.checksum==b.checksum);
    assert(a.version==b.version);
    assert(a.map==b.map);
    assert(YELLOW_MAP_COUNT==11 && YELLOW_GYM_MAP_COUNT==13);
    assert(yellow_scene_asset(YMapPewterGym));
    assert(!yellow_scene_asset(YELLOW_GYM_MAP_COUNT));
}
int main(void) {
    check_original_scope_and_negative_release();
    existing_save_and_world_stay_unaffected();
    puts("ORIGINAL_249_MAP_IDS_151_SPECIES_165_MOVES_NEGATIVE_RELEASE_V13_PASS");
    return 0;
}
