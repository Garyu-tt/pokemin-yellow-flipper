#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
static void ready(YellowGame* g,uint8_t map,int x,int y) {
    yellow_init(g);
    g->running=true;g->screen=YScreenWorld;g->story=YStoryPikachuFollows;
    g->has_pikachu=1;g->follower_valid=0;g->map=map;
    g->x=(int8_t)x;g->y=(int8_t)y;
}
static void cave_original_map_and_collision(void) {
    assert(YELLOW_MTMOON_MAP_COUNT==16 && YMapMtMoon1F==15);
    assert(YELLOW_ROUTE4_MAP_COUNT==15);
    assert(!yellow_world_scene_asset(YMapMtMoon1F));
    assert(!yellow_extended_scene_asset(YMapMtMoon1F));
    const YellowLevelAsset* cave=yellow_full_scene_asset(YMapMtMoon1F);
    assert(cave && cave->w==20 && cave->h==18);
    assert(cave->blocks_count==128 && cave->tiles_count==80);
    assert(yellow_map_w(YMapMtMoon1F)==40 && yellow_map_h(YMapMtMoon1F)==36);
    assert(!yellow_full_scene_asset(YELLOW_MTMOON_MAP_COUNT));
    for(unsigned i=0;i<360;i++)assert(ymap_mtmoon1f[i]<128);
    for(int y=0;y<36;y++)for(int x=0;x<40;x++) {
        uint8_t bid=ymap_mtmoon1f[(y/2)*20+x/2];
        uint8_t tile=yblocks_cavern[(unsigned)bid*16u+
                                  (y%2*2+1)*4+(x%2)*2];
        bool walk=(tile<80&&ycollision_cavern[tile]!=0);
        assert((bool)yellow_mtmoon1f_passable(x,y)==walk);
        assert(yellow_world_tile_id(YMapMtMoon1F,x*2,y*2)<80);
    }
    assert(!yellow_mtmoon1f_passable(-1,35));
    assert(!yellow_mtmoon1f_passable(40,35));
    assert(!yellow_mtmoon1f_passable(14,36));
    assert(yellow_mtmoon1f_tile_id(-1,0)==0xff);
    assert(yellow_mtmoon1f_tile_id(80,0)==0xff);
    assert(yellow_tile_id(YMapMtMoon1F,0,0)==0xff);
}
static void canonical_route4_cave_entrance_return(void) {
    YellowGame g;ready(&g,YMapRoute4,18,6);
    uint8_t target,tx,ty;
    assert(yellow_mtmoon1f_source_warp(YMapRoute4,18,5,&target,&tx,&ty));
    assert(target==YMapMtMoon1F && tx==14 && ty==35);
    yellow_input(&g,YUp);
    assert(g.map==YMapMtMoon1F && g.x==14 && g.y==35 && g.steps==1);
    assert(g.screen==YScreenWorld);
    assert(yellow_mtmoon1f_passable(14,34));
    yellow_input(&g,YUp);
    assert(g.map==YMapMtMoon1F && g.x==14 && g.y==34 && g.steps==2);
    yellow_input(&g,YDown);
    yellow_input(&g,YDown);
    assert(g.map==YMapRoute4 && g.x==18 && g.y==5 && g.steps==3);
    assert(g.screen==YScreenWorld);
    ready(&g,YMapMtMoon1F,15,34);g.facing=YDown;
    yellow_input(&g,YDown);
    assert(g.map==YMapRoute4 && g.x==18 && g.y==5);
    assert(!yellow_mtmoon1f_source_warp(YMapMtMoon1F,5,5,&target,&tx,&ty));
    assert(!yellow_mtmoon1f_source_warp(YMapRoute4,24,5,&target,&tx,&ty));
    assert(!yellow_mtmoon1f_source_warp(YMapRoute3,18,5,&target,&tx,&ty));
    assert(!yellow_mtmoon1f_source_warp(YMapRoute4,18,5,NULL,&tx,&ty));
}
static void no_unknown_map_encoded_in_old_saves(void) {
    YellowGame g,loaded;YellowSave s;
    ready(&g,YMapMtMoon1F,14,34);
    yellow_save_create(&g,&s);
    assert(s.version==13 && sizeof(s)==132);
    assert(s.map==YMapRoute2 && s.x==9 && s.y==0);
    assert(!s.follower_valid && s.follower_map==YMapRoute2);
    assert(yellow_save_load(&loaded,&s));
    assert(loaded.map==YMapRoute2 && loaded.x==9 && loaded.y==0);
    assert(!yellow_original_release_ready(NULL));
}
int main(void) {
    cave_original_map_and_collision();
    canonical_route4_cave_entrance_return();
    no_unknown_map_encoded_in_old_saves();
    puts("MTMOON1F_SOURCE_CAVERN_1440_COL_CELLS_ROUTE4_ENTRANCE_RETURN_V13_PASS");
    return 0;
}
