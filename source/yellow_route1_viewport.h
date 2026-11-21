#ifndef YELLOW_ROUTE1_VIEWPORT_H
#define YELLOW_ROUTE1_VIEWPORT_H
#include <stdint.h>
#include <stdbool.h>
/* Reader dependency injection keeps viewport testable without Flipper SDK. */
typedef bool (*YellowRoute1TileReader)(void *ctx,uint16_t col,
                                       uint16_t row,uint8_t rows[8]);
#define ROUTE1_CACHE_COLS 17u
#define ROUTE1_CACHE_ROWS 9u
#define ROUTE1_CACHE_SIZE (ROUTE1_CACHE_COLS*ROUTE1_CACHE_ROWS)
typedef struct {
 int16_t x,y;
 uint8_t rows[8];
 uint8_t valid;
} YellowRoute1Tile;
typedef struct {
 YellowRoute1Tile tile[ROUTE1_CACHE_SIZE];
 uint8_t initialized;
} YellowRoute1Viewport;
/* One source-derived tile fetch; 8-byte output is Flipper XBM little-bit-endian. */
bool yellow_source_tile(YellowRoute1TileReader reader,void* ctx,
                        uint16_t tile_x,uint16_t tile_y,
                        uint16_t max_cols,uint16_t max_rows,
                        uint8_t result[8]);
bool yellow_route1_tile(YellowRoute1TileReader reader,void* ctx,uint16_t tile_x,
                        uint16_t tile_y,uint8_t result[8]);
/* Fill at most budget missing tiles per call, never block the draw callback. */
bool yellow_route1_prefetch(YellowRoute1Viewport* viewport,
                            YellowRoute1TileReader reader,void* ctx,
                            int first_col,int first_row,
                            int world_cols,int world_rows,
                            unsigned budget);
/* Bitmap accessor is pure, never calls storage from render callback. */
const uint8_t* yellow_route1_cached_tile(const YellowRoute1Viewport* viewport,
                                          int col,int row);
#endif
