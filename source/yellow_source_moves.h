#ifndef YELLOW_SOURCE_MOVES_H
#define YELLOW_SOURCE_MOVES_H
#include <stdint.h>
#include <stdbool.h>
typedef struct {
 uint8_t id,power,type,accuracy,pp,effect;
} YSourceMove;
bool y_source_move(uint8_t original_id,YSourceMove* out);
bool y_source_native_move(uint8_t native_index,YSourceMove* out);
uint8_t y_source_supported_native_moves(void);
bool y_source_native_index(uint8_t original_id,uint8_t* native_index);
#endif
