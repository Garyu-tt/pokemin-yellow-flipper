#ifndef POKELLOW_PYTG_READER_H
#define POKELLOW_PYTG_READER_H
#include <stdint.h>
#include "pydp_reader.h"
#include "pyts_reader.h"
typedef struct {
 PydpRead read;
 void* ctx;
 uint32_t crc32;
 uint8_t ready;
} PytgAtlas;
/* PYTG/1: source four-shade PNG converted with deterministic Bayer 4x4
   into per-tile 1bpp rows. Data only; not yet a native world renderer. */
int pytg_open(PytgAtlas* a,PydpRead cb,void* ctx);
int pytg_verify(const PytgAtlas* a);
int pytg_tile(const PytgAtlas* a,uint8_t tileset,uint8_t tile,uint8_t out[8]);
int pytg_map_tile(const PytgAtlas* gfx,const PytAtlas* tiles,
                  const PydpPack* maps,uint8_t map,uint8_t bx,uint8_t by,
                  uint8_t tx,uint8_t ty,uint8_t out[8]);
#endif
