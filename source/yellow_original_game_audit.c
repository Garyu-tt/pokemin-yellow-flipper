/* Executable read-only scope inventory. These counters are NOT progress toward
 * completion and intentionally never set success/evidence flags. */
#include "yellow_original_game_audit.h"
#include "yellow_pallet_audio.h"
#include "yellow_assets.h"
#include <string.h>
bool yellow_original_game_gaps(YellowOriginalGameGaps* out) {
    if(!out)return false;
    YellowOriginalGameGaps r={0};
    r.original_map_identifiers=YELLOW_ORIGINAL_MAP_IDENTIFIERS;
    r.original_map_geometries=YELLOW_ORIGINAL_MAP_GEOMETRIES;
    r.original_species=YELLOW_ORIGINAL_SPECIES_IDS;
    r.original_moves=YELLOW_ORIGINAL_MOVE_IDS;
    r.pokedex_slots=YELLOW_DEX_SPECIES;
    r.pc_slots=YELLOW_PC_CAPACITY;
    r.pinned_pallet_ch1_excerpt_notes=YELLOW_PALLET_NOTE_COUNT;
    for(unsigned id=0;id<YELLOW_GYM_MAP_COUNT;id++) {
        const YellowLevelAsset* a=yellow_scene_asset((uint8_t)id);
        if(a && a->w && a->h)r.registered_native_map_scenes++;
    }
    for(unsigned id=0;id<255;id++)if(y_species_info((uint8_t)id)) {
        if(id<=YK_SANDSHREW)r.native_battle_species++;
        else r.extended_native_battle_species++;
    }
    for(unsigned id=0;id<255;id++)if(y_move_info((uint8_t)id)) {
        if(id<=YM_SCREECH)r.native_move_info_entries++;
        else r.extended_native_move_info_entries++;
    }
    /* No controller milestone or module count can prove these external
       end-to-end facts. They remain false in the ROM-independent runtime. */
    r.original_story_to_credits_verified=false;
    r.original_all_maps_events_verified=false;
    r.full_original_multichannel_audio_verified=false;
    r.original_game_mechanics_parity_verified=false;
    r.physical_flipper_full_playthrough_verified=false;
    *out=r;
    return true;
}
bool yellow_original_release_ready(const YellowOriginalGameGaps* snapshot) {
    /* This API is intentionally a hard safety gate. A real release must be
       approved by the independent controller's external signed evidence,
       never by editable gameplay fields or merely reaching the end of a
       finite development queue. */
    (void)snapshot;
    return false;
}
