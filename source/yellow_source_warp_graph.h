#ifndef YELLOW_SOURCE_WARP_GRAPH_H
#define YELLOW_SOURCE_WARP_GRAPH_H
#include <stdbool.h>
#include <stdint.h>
typedef struct {
 uint8_t src, x, y, target, target_x, target_y;
 uint8_t source_warp_id, target_warp_id;
} YOriginalWarp;
uint8_t yellow_original_warp_count(void);
bool yellow_original_warp_record(uint8_t index,YOriginalWarp* out);
bool yellow_original_warp_at(uint8_t map,int x,int y,
                             uint8_t* target,uint8_t* tx,uint8_t* ty);
#endif
