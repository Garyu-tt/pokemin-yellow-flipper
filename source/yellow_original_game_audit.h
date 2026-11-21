#ifndef YELLOW_ORIGINAL_GAME_AUDIT_H
#define YELLOW_ORIGINAL_GAME_AUDIT_H
#include <stdbool.h>
#include <stdint.h>
/* Denominators verified against pinned pret/pokeyellow:
 * 249 map IDs include 22 zero-geometry placeholders;
 * 165 non-NO_MOVE moves; 151 national species IDs.
 * These are INVENTORY counts, never completion percentages. */
#define YELLOW_ORIGINAL_MAP_IDENTIFIERS 249u
#define YELLOW_ORIGINAL_MAP_GEOMETRIES 227u
#define YELLOW_ORIGINAL_SPECIES_IDS 151u
#define YELLOW_ORIGINAL_MOVE_IDS 165u
typedef struct {
    uint16_t original_map_identifiers;
    uint16_t original_map_geometries;
    uint16_t registered_native_map_scenes;
    uint16_t original_species;
    uint16_t native_battle_species; /* frozen native v13 baseline */
    uint16_t extended_native_battle_species; /* additional playable species */
    uint16_t original_moves;
    uint16_t native_move_info_entries; /* frozen v13 base move IDs */
    uint16_t extended_native_move_info_entries; /* later native move IDs */
    uint16_t pokedex_slots;
    uint16_t pc_slots;
    uint8_t pinned_pallet_ch1_excerpt_notes;
    bool original_story_to_credits_verified;
    bool original_all_maps_events_verified;
    bool full_original_multichannel_audio_verified;
    bool original_game_mechanics_parity_verified;
    bool physical_flipper_full_playthrough_verified;
} YellowOriginalGameGaps;
bool yellow_original_game_gaps(YellowOriginalGameGaps* out);
bool yellow_original_release_ready(const YellowOriginalGameGaps* snapshot);
#endif
