#ifndef YELLOW_SOURCE_CONNECTIONS_H
#define YELLOW_SOURCE_CONNECTIONS_H
#include <stdint.h>
#include <stdbool.h>
/* direction 0=north, 1=south. Caller validates the map edge crossing. */
bool yellow_source_connection(uint8_t map,uint8_t direction,int x,int y,
                              uint8_t* next,uint8_t* dest_x,uint8_t* dest_y);
#endif
