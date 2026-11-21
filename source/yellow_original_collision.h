#ifndef YELLOW_ORIGINAL_COLLISION_H
#define YELLOW_ORIGINAL_COLLISION_H
#include <stdbool.h>
#include <stdint.h>
/* Strictly pinned original Gen1 tile-ID walkability; first-party gameplay.
   Returns false when a map is unrepresented or coordinates invalid.
   NO external I/O, generated directly from PYDP+PYTS source catalogs. */
bool yellow_original_collision(uint8_t native_map,int x,int y,bool* walkable);
#endif
