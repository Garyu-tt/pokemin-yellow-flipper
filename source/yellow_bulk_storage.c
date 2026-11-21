#include "yellow_bulk_storage.h"
#include "yellow_bulk_data.h"
#include <string.h>

static int flipper_pydp_read(void* ctx,uint32_t offset,uint8_t* dst,size_t len) {
    File* file=(File*)ctx;
    if(!file || !dst || !len || len>128)return 0;
    if(!storage_file_seek(file,offset,true))return 0;
    return storage_file_read(file,dst,len)==len;
}
bool yellow_bulk_mount(Storage* storage,YellowBulkMount* mount) {
    if(!storage || !mount)return false;
    memset(mount,0,sizeof(*mount));
    File* file=storage_file_alloc(storage);
    if(!file)return false;
    mount->file=file;
    if(!storage_file_open(file,YELLOW_BULK_PATH,FSAM_READ,FSOM_OPEN_EXISTING)) {
        yellow_bulk_unmount(mount);return false;
    }
    if(!pydp_open(&mount->pack,flipper_pydp_read,file) ||
       !pydp_verify(&mount->pack) ||
       !yellow_bulk_prepare(&mount->pack,&mount->wild) ||
       !yellow_bulk_prepare_lass(&mount->pack,&mount->lass) ||
       !pydp_map_block_at(&mount->pack,51,0,0,
                          &mount->original_forest_block00)) {
        yellow_bulk_unmount(mount);return false;
    }
    mount->ready=true;
    /* Secondary source tile atlas is optional. A bad/missing atlas must
       not prevent existing saved gameplay from loading. */
    File* tile_file=storage_file_alloc(storage);
    if(tile_file) {
        mount->tileset_file=tile_file;
        uint8_t probe=0;
        if(storage_file_open(tile_file,YELLOW_TILESET_PATH,
                             FSAM_READ,FSOM_OPEN_EXISTING) &&
           pyt_open(&mount->tileset_atlas,flipper_pydp_read,tile_file) &&
           pyt_verify(&mount->tileset_atlas) &&
           pyt_map_tile_id(&mount->tileset_atlas,&mount->pack,51,
                           0,0,0,0,&probe)) {
            mount->tileset_ready=true;
        } else {
            if(storage_file_is_open(tile_file))storage_file_close(tile_file);
            storage_file_free(tile_file);
            mount->tileset_file=NULL;
            memset(&mount->tileset_atlas,0,sizeof(mount->tileset_atlas));
        }
    }
    /* Last optional pack: original PNG source rendered to deterministic
       monochrome 8x8 rows. Failure never touches old gameplay or saves. */
    if(mount->tileset_ready) {
        File* pixels=storage_file_alloc(storage);
        if(pixels) {
            mount->graphics_file=pixels;
            uint8_t check[8];
            if(storage_file_open(pixels,YELLOW_GRAPHICS_PATH,
                                 FSAM_READ,FSOM_OPEN_EXISTING) &&
               pytg_open(&mount->graphics_atlas,flipper_pydp_read,pixels) &&
               pytg_verify(&mount->graphics_atlas) &&
               pytg_map_tile(&mount->graphics_atlas,&mount->tileset_atlas,
                             &mount->pack,51,0,0,0,0,check)) {
                mount->graphics_ready=true;
            } else {
                if(storage_file_is_open(pixels))storage_file_close(pixels);
                storage_file_free(pixels);
                mount->graphics_file=NULL;
                memset(&mount->graphics_atlas,0,sizeof(mount->graphics_atlas));
            }
        }
    }
    return true;
}
void yellow_bulk_unmount(YellowBulkMount* mount) {
    if(!mount)return;
    if(mount->graphics_file) {
        if(storage_file_is_open(mount->graphics_file))
            storage_file_close(mount->graphics_file);
        storage_file_free(mount->graphics_file);
    }
    if(mount->tileset_file) {
        if(storage_file_is_open(mount->tileset_file))
            storage_file_close(mount->tileset_file);
        storage_file_free(mount->tileset_file);
    }
    if(mount->file) {
        if(storage_file_is_open(mount->file))storage_file_close(mount->file);
        storage_file_free(mount->file);
    }
    memset(mount,0,sizeof(*mount));
}

bool yellow_bulk_map_block(const YellowBulkMount* mount,uint8_t map_id,
                           uint8_t bx,uint8_t by,uint8_t* out) {
    if(!mount || !mount->ready || !out)return false;
    return pydp_map_block_at(&mount->pack,map_id,bx,by,out)!=0;
}

bool yellow_bulk_original_tile_id(const YellowBulkMount* mount,
                                  uint8_t map,uint8_t bx,uint8_t by,
                                  uint8_t tx,uint8_t ty,uint8_t* out) {
    if(!mount || !mount->ready || !mount->tileset_ready)return false;
    return pyt_map_tile_id(&mount->tileset_atlas,&mount->pack,
                           map,bx,by,tx,ty,out)!=0;
}
bool yellow_bulk_original_collision_id(const YellowBulkMount* mount,
                                       uint8_t map,uint8_t tile,
                                       uint8_t* allowed) {
    uint8_t tileset;
    if(!mount || !mount->ready || !mount->tileset_ready || !allowed ||
       !pyt_map_tileset(&mount->tileset_atlas,map,&tileset))return false;
    return pyt_collision_allowed(&mount->tileset_atlas,tileset,tile,allowed)!=0;
}

bool yellow_bulk_original_tile_pixels(const YellowBulkMount* mount,
                                      uint8_t map,uint8_t bx,uint8_t by,
                                      uint8_t tx,uint8_t ty,uint8_t rows[8]) {
    if(!mount || !mount->ready || !mount->tileset_ready ||
       !mount->graphics_ready || !rows)return false;
    return pytg_map_tile(&mount->graphics_atlas,&mount->tileset_atlas,
                         &mount->pack,map,bx,by,tx,ty,rows)!=0;
}
