#ifndef YELLOW_BULK_STORAGE_H
#define YELLOW_BULK_STORAGE_H
#include <stdbool.h>
#include <storage/storage.h>
#include "yellow_core.h"
#include "pydp_reader.h"
#include "pyts_reader.h"
#include "pytg_reader.h"

/* Optional data pack; physical device must copy separately.
   The pack is immutable and never contains the owner's save state. */
#define YELLOW_BULK_PATH "/ext/apps_data/yellow_native/pokeyellow_data.pydb"
#define YELLOW_TILESET_PATH "/ext/apps_data/yellow_native/pokeyellow_tilesets.pyts"
#define YELLOW_GRAPHICS_PATH "/ext/apps_data/yellow_native/pokeyellow_tiles_1bpp.pytg"

typedef struct {
    File* file;
    PydpPack pack;
    File* tileset_file;
    PytAtlas tileset_atlas;
    bool tileset_ready;
    File* graphics_file;
    PytgAtlas graphics_atlas;
    bool graphics_ready;
    YellowBulkWild wild;
    YellowBulkLass lass;
    uint8_t original_forest_block00;
    bool ready;
} YellowBulkMount;

bool yellow_bulk_mount(Storage* storage,YellowBulkMount* mount);
bool yellow_bulk_map_block(const YellowBulkMount* mount,uint8_t map_id,
                           uint8_t bx,uint8_t by,uint8_t* out);
bool yellow_bulk_original_tile_id(const YellowBulkMount* mount,
                                   uint8_t map,uint8_t bx,uint8_t by,
                                   uint8_t tx,uint8_t ty,uint8_t* out);
bool yellow_bulk_original_collision_id(const YellowBulkMount* mount,
                                       uint8_t map,uint8_t tile,
                                       uint8_t* allowed);
bool yellow_bulk_original_tile_pixels(const YellowBulkMount* mount,
                                      uint8_t map,uint8_t bx,uint8_t by,
                                      uint8_t tx,uint8_t ty,uint8_t rows[8]);
void yellow_bulk_unmount(YellowBulkMount* mount);
#endif
