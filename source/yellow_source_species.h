#ifndef YELLOW_SOURCE_SPECIES_H
#define YELLOW_SOURCE_SPECIES_H
#include <stdint.h>
#include <stdbool.h>
/* Must include yellow_battle.h first for YBattleStats. */
typedef struct {
 uint8_t dex,hp,attack,defense,speed,special,type1,type2,catch_rate,base_exp,growth;
} YSourceSpecies;
bool y_source_species(uint8_t dex,YSourceSpecies* out);
bool y_source_calc_stats(uint8_t dex,uint8_t level,YBattleStats* output);
uint32_t y_source_growth_threshold(uint8_t dex,uint8_t level);
#endif
