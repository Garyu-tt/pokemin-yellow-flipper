#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
static void ready(YellowGame* g,uint8_t map,int x,int y) {
    yellow_init(g);g->running=true;g->screen=YScreenWorld;
    g->story=YStoryPikachuFollows;g->has_pikachu=1;
    g->map=map;g->x=(int8_t)x;g->y=(int8_t)y;
    g->follower_valid=0;
}
static void map_source_and_full_collision(void) {
    assert(YELLOW_MAP_COUNT==11 && YELLOW_GYM_MAP_COUNT==13);
    assert(YELLOW_ROUTE3_MAP_COUNT==14 && YELLOW_ROUTE4_MAP_COUNT==15);
    assert(YMapRoute4==14);
    assert(!yellow_scene_asset(YMapRoute4));
    assert(!yellow_world_scene_asset(YMapRoute4));
    const YellowLevelAsset* a=yellow_extended_scene_asset(YMapRoute4);
    assert(a && a->w==45 && a->h==9);
    assert(yellow_map_w(YMapRoute4)==90 && yellow_map_h(YMapRoute4)==18);
    assert(!yellow_extended_scene_asset(YELLOW_ROUTE4_MAP_COUNT));
    for(unsigned i=0;i<405;i++)assert(a->map[i]<128);
    for(int y=0;y<18;y++)for(int x=0;x<90;x++) {
        uint8_t id=ymap_route4[(y/2)*45+x/2];
        uint8_t tile=ypewter_overworld_blocks[(unsigned)id*16u+
                               (y%2*2+1)*4+(x%2)*2];
        bool expected=tile<96&&ypewter_overworld_col[tile]!=0;
        assert((bool)yellow_route4_passable(x,y)==expected);
        assert(yellow_world_tile_id(YMapRoute4,x*2,y*2)!=0xff);
    }
    assert(!yellow_route4_passable(-1,0));
    assert(!yellow_route4_passable(90,0));
    assert(!yellow_route4_passable(0,-1));
    assert(!yellow_route4_passable(0,18));
    assert(yellow_route4_tile_id(-1,0)==0xff);
    assert(yellow_route4_tile_id(180,0)==0xff);
    assert(yellow_tile_id(YMapRoute4,0,0)==0xff);
}
static void seven_original_reciprocal_warps(void) {
    for(int x=57;x<=63;x++) {
        YellowGame g;ready(&g,YMapRoute3,x,0);
        uint8_t to=255,tx=255,ty=255;
        assert(yellow_route4_vertical_connection(g.map,x,-1,&to,&tx,&ty));
        assert(to==YMapRoute4 && tx==x-50 && ty==17);
        assert(yellow_route3_passable(x,0));
        assert(yellow_route4_passable(tx,ty));
        yellow_input(&g,YUp);
        assert(g.map==YMapRoute4 && g.x==x-50 && g.y==17 && g.steps==1);
        assert(g.screen==YScreenWorld);
        yellow_input(&g,YDown);
        assert(g.map==YMapRoute3 && g.x==x && g.y==0 && g.steps==2);
        assert(g.screen==YScreenWorld);
    }
    YellowGame invalid;ready(&invalid,YMapRoute3,56,0);
    yellow_input(&invalid,YUp);
    assert(invalid.map==YMapRoute3 && invalid.x==56);
    ready(&invalid,YMapRoute4,6,17);
    yellow_input(&invalid,YDown);
    assert(invalid.map==YMapRoute4 && invalid.x==6 && invalid.y==17);
    ready(&invalid,YMapRoute3,57,0);
    invalid.story=YStoryBeforeOak;
    yellow_input(&invalid,YUp);
    assert(invalid.map==YMapRoute3 && invalid.y==0);
    uint8_t to=0,tx=0,ty=0;
    assert(!yellow_route4_vertical_connection(YMapRoute3,64,-1,&to,&tx,&ty));
    assert(!yellow_route4_vertical_connection(YMapRoute4,14,18,&to,&tx,&ty));
    assert(!yellow_route4_vertical_connection(YMapRoute3,57,0,&to,&tx,&ty));
    assert(!yellow_route4_vertical_connection(YMapRoute3,57,-1,NULL,&tx,&ty));
    /* No directly reachable Mt Moon or Cerulean City is invented here. */
    assert(!yellow_extended_scene_asset(15));
}
static void legacy_parent_saved_without_unknown_map_id(void) {
    YellowGame g,loaded;YellowSave parent;
    ready(&g,YMapRoute4,18,5);
    g.follower_valid=1;g.follower_map=YMapRoute4;
    g.follower_x=18;g.follower_y=6;
    yellow_save_create(&g,&parent);
    assert(parent.version==13 && sizeof(parent)==132);
    assert(parent.map==YMapRoute2 && parent.x==9 && parent.y==0);
    assert(parent.follower_map==YMapRoute2 && !parent.follower_valid);
    assert(yellow_save_load(&loaded,&parent));
    assert(loaded.map==YMapRoute2 && loaded.x==9 && loaded.y==0);
    assert(yellow_original_release_ready(NULL)==false);
}
int main(void) {
    map_source_and_full_collision();
    seven_original_reciprocal_warps();
    legacy_parent_saved_without_unknown_map_id();
    puts("ROUTE4_405_SOURCE_BLOCKS_1620_COLLISION_CELLS_7_ORIGINAL_LINKS_V13_PASS");
    return 0;
}
