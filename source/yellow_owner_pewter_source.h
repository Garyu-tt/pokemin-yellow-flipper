#ifndef YELLOW_OWNER_PEWTER_SOURCE_H
#define YELLOW_OWNER_PEWTER_SOURCE_H
#include <stdbool.h>
#include <stdint.h>
typedef struct {uint8_t dex,level;} YPewterPokemon;
typedef struct {
 uint8_t original_map,x,y,target_map,target_warp;
} YPewterWarp;
typedef struct {
 uint8_t trainer_class,party_number,count;
 YPewterPokemon mon[2];
} YPewterTrainerParty;
/* Original global map id 2=PewterCity, 54=PewterGym.
   Compact data adapter, not a registered native YellowMap scene. */
bool y_pewter_geometry(uint8_t original_map,uint8_t* width,uint8_t* height);
bool y_pewter_block_at(uint8_t original_map,uint8_t x,uint8_t y,uint8_t* block);
bool y_pewter_passable(uint8_t original_map,int x,int y,bool* allowed);
bool y_pewter_warp_at(uint8_t original_map,int x,int y,YPewterWarp* result);
bool y_pewter_trainer(uint8_t trainer_slot,YPewterTrainerParty* result);
#endif
