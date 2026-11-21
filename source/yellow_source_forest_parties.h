#ifndef YELLOW_SOURCE_FOREST_PARTIES_H
#define YELLOW_SOURCE_FOREST_PARTIES_H
#include <stdbool.h>
#include <stdint.h>
typedef struct {uint8_t species,level;} YSourceTrainerMon;
typedef struct {
 uint8_t class_id,party_number,count;
 YSourceTrainerMon mon[3];
} YSourceForestParty;
const YSourceForestParty* y_source_forest_party(uint8_t trainer);
bool y_source_forest_mon(uint8_t trainer,uint8_t member,uint8_t* species,uint8_t* level);
#endif
