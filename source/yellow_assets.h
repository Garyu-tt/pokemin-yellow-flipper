#pragma once
#include <stdint.h>
typedef struct { uint8_t w,h; uint16_t blocks_count,tiles_count; const uint8_t* map; const uint8_t* blocks; const uint8_t* tiles; const uint8_t* collision; } YellowLevelAsset;
#define YELLOW_LEVELS_COUNT 11
extern const YellowLevelAsset yellow_levels[YELLOW_LEVELS_COUNT];
extern const uint8_t ypokemon_pikachu_front[200];
extern const uint8_t ypokemon_eevee_front[200];
extern const uint8_t ypokemon_pikachu_back[128];
extern const uint8_t ypokemon_pidgey_back[128];
extern const uint8_t ypokemon_rattata_back[128];
extern const uint8_t ypokemon_pidgey_front[200];
extern const uint8_t ypokemon_rattata_front[200];
extern const uint8_t ysprite_red[192];
extern const uint8_t ysprite_oak[192];
extern const uint8_t ysprite_little_girl[192];
extern const uint8_t ysprite_fisher[192];
extern const uint8_t ysprite_mom[192];
extern const uint8_t ysprite_girl[192];
extern const uint8_t ysprite_blue[192];
extern const uint8_t ysprite_pikachu[192];
extern const uint8_t ysprite_daisy[192];
extern const uint8_t ysprite_pokedex[192];
extern const uint8_t ysprite_poke_ball[192];
extern const uint8_t ysprite_scientist[192];
extern const uint8_t ysprite_youngster[192];
extern const uint8_t ysprite_gambler[192];
extern const uint8_t ysprite_nurse[192];
extern const uint8_t ysprite_gentleman[192];
extern const uint8_t ysprite_cooltrainer_m[192];
extern const uint8_t ysprite_link_receptionist[192];
extern const uint8_t ysprite_chansey[192];
