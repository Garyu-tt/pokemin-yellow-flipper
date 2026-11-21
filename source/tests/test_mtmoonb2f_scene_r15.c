/* Independently tested pinned Mt Moon B2F geometry, no fabricated IDs. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_core.h"
#include "yellow_assets.c"
#include "yellow_battle.c"
#include "yellow_core.c"
int main(void) {
    assert(YMapMtMoonB2F==17 && YMapMtMoonB1F==16);
    assert(!yellow_gameplay_scene_asset(YMapMtMoonB2F));
    yellow_b2f_set_enabled(true);
    const YellowLevelAsset* a=yellow_gameplay_scene_asset(YMapMtMoonB2F);
    assert(a && a->w==20 && a->h==18);
    assert(a->blocks_count==128 && a->tiles_count==80);
    assert(a->map==ymap_mtmoonb2f && a->blocks==yblocks_cavern);
    assert(a->tiles==ytiles_cavern && a->collision==ycollision_cavern);
    assert(yellow_map_w(YMapMtMoonB2F)==40 && yellow_map_h(YMapMtMoonB2F)==36);
    uint32_t fnv=2166136261u;
    for(unsigned i=0;i<360;i++)fnv=(fnv^ymap_mtmoonb2f[i])*16777619u;
    assert(fnv==0xbf0a29edu);
    unsigned walkable=0;
    for(int y=0;y<36;y++)for(int x=0;x<40;x++){
        uint8_t block=ymap_mtmoonb2f[(y/2)*20+x/2];
        assert(block<128);
        uint8_t tile=yblocks_cavern[(unsigned)block*16u+((y%2)*2+1)*4+(x%2)*2];
        const bool expected=(tile<80 && ycollision_cavern[tile]!=0);
        assert((bool)yellow_mtmoonb2f_passable(x,y)==expected);
        assert(yellow_world_tile_id(YMapMtMoonB2F,x*2,y*2)<80);
        if(expected)walkable++;
    }
    assert(!yellow_mtmoonb2f_passable(-1,0));
    assert(!yellow_mtmoonb2f_passable(40,0));
    assert(!yellow_mtmoonb2f_passable(0,36));
    assert(yellow_mtmoonb2f_tile_id(80,0)==0xff);
    uint8_t target=255,tx=255,ty=255;
    /* Unknown LAST_MAP remains invalid even after B2F activation. */
    assert(!destination(YMapMtMoonB1F,27,3,&target,&tx,&ty));
    printf("B2F_SCENE_SOURCE_VERIFIED_1440_CELLS_PASS walkable=%u\n",walkable);
    return 0;
}
