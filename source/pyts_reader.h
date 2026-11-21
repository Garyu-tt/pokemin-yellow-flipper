#ifndef POKELLOW_PYTS_READER_H
#define POKELLOW_PYTS_READER_H
#include <stddef.h>
#include <stdint.h>
#include "pydp_reader.h"
/* PYTS/1 source-authenticated grayscale tile *IDs* not pixels.
   A second independent read-only SD pack, never touches save files. */
typedef struct {
 PydpRead read;
 void*ctx;
 uint32_t crc32;
 uint8_t available;
} PytAtlas;
int pyt_open(PytAtlas* a,PydpRead cb,void* ctx);
int pyt_verify(const PytAtlas* a);
int pyt_map_tileset(const PytAtlas* a,uint8_t map,uint8_t* tileset);
int pyt_block_count(const PytAtlas* a,uint8_t tileset,uint8_t* count);
int pyt_tile_id(const PytAtlas* a,uint8_t tileset,uint8_t block,
                uint8_t tx,uint8_t ty,uint8_t* id);
int pyt_map_tile_id(const PytAtlas* a,const PydpPack* maps,
                    uint8_t map,uint8_t block_x,uint8_t block_y,
                    uint8_t tile_x,uint8_t tile_y,uint8_t* out);
int pyt_collision_allowed(const PytAtlas* a,uint8_t tileset,
                          uint8_t tile,uint8_t* allowed);
#endif
