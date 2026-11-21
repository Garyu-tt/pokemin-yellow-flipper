#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_owner_pewter_source.c"
#include "../yellow_core.c"
static void ready(YellowGame* g,uint8_t map,int x,int y) {
    yellow_init(g);
    g->running=true;g->screen=YScreenWorld;
    g->story=YStoryPikachuFollows;g->has_pikachu=1;
    g->follower_valid=0;
    g->map=map;g->x=(int8_t)x;g->y=(int8_t)y;
}
static void source_geometry_and_collision(void) {
    assert(!yellow_scene_asset(YMapRoute3)); /* legacy stage contract */
    const YellowLevelAsset* a=yellow_world_scene_asset(YMapRoute3);
    assert(YELLOW_MAP_COUNT==11 && YELLOW_GYM_MAP_COUNT==13);
    assert(YELLOW_ROUTE3_MAP_COUNT==14 && YMapRoute3==13);
    assert(a && a->w==35 && a->h==9);
    assert(yellow_map_w(YMapRoute3)==70 && yellow_map_h(YMapRoute3)==18);
    assert(!yellow_world_scene_asset(YELLOW_ROUTE3_MAP_COUNT));
    assert(!yellow_route3_passable(-1,8) && !yellow_route3_passable(70,8));
    assert(!yellow_route3_passable(0,-1) && !yellow_route3_passable(0,18));
    assert(yellow_route3_tile_id(-1,0)==0xff);
    assert(yellow_route3_tile_id(140,0)==0xff);
    assert(yellow_tile_id(YMapRoute3,0,0)==0xff); /* legacy API unchanged */
    for(unsigned by=0;by<9;by++)
        for(unsigned bx=0;bx<35;bx++)
            assert(a->map[by*35+bx]<128);
    for(int y=0;y<18;y++)for(int x=0;x<70;x++) {
        uint8_t block=ymap_route3[(y/2)*35+x/2];
        uint8_t tile=ypewter_overworld_blocks[(unsigned)block*16u+
                           (y%2*2+1)*4+(x%2)*2];
        bool expected=tile<96 && ypewter_overworld_col[tile]!=0;
        assert((bool)yellow_route3_passable(x,y)==expected);
        assert(yellow_world_tile_id(YMapRoute3,x*2,y*2)<96);
    }
}
static void original_bidirectional_corridors(void) {
    for(int y=16;y<=19;y++) {
        YellowGame g;ready(&g,YMapPewterCity,39,y);
        uint8_t to=255,tx=255,ty=255;
        assert(yellow_route3_horizontal_connection(g.map,40,y,&to,&tx,&ty));
        assert(to==YMapRoute3 && tx==0 && ty==y-8);
        assert(yellow_route3_passable(0,y-8));
        yellow_input(&g,YRight);
        assert(g.map==YMapRoute3 && g.x==0 && g.y==y-8);
        assert(g.steps==1 && g.screen==YScreenWorld);
        yellow_input(&g,YLeft);
        assert(g.map==YMapPewterCity && g.x==39 && g.y==y);
        assert(g.steps==2 && g.screen==YScreenWorld);
    }
    YellowGame invalid;ready(&invalid,YMapPewterCity,39,20);
    yellow_input(&invalid,YRight);
    assert(invalid.map==YMapPewterCity && invalid.x==39);
    ready(&invalid,YMapRoute3,0,12);
    yellow_input(&invalid,YLeft);
    assert(invalid.map==YMapRoute3 && invalid.x==0);
    ready(&invalid,YMapRoute3,0,9);
    invalid.story=YStoryBeforeOak;
    yellow_input(&invalid,YLeft);
    assert(invalid.map==YMapRoute3 && invalid.x==0);
    uint8_t to=0,tx=0,ty=0;
    assert(!yellow_route3_horizontal_connection(YMapRoute3,70,9,&to,&tx,&ty));
    assert(!yellow_route3_horizontal_connection(YMapPewterCity,40,15,&to,&tx,&ty));
    assert(!yellow_route3_horizontal_connection(YMapPewterCity,40,16,NULL,&tx,&ty));
}
static void save_and_render_compatibility(void) {
    YellowGame g,loaded;YellowSave s;
    ready(&g,YMapRoute3,8,10);
    g.follower_map=YMapRoute3;g.follower_valid=1;
    g.follower_x=8;g.follower_y=11;
    yellow_save_create(&g,&s);
    assert(s.version==13 && sizeof(s)==132);
    assert(s.map==YMapRoute2 && s.x==9 && s.y==0);
    assert(s.follower_valid==0 && s.follower_map==YMapRoute2);
    assert(yellow_save_load(&loaded,&s));
    assert(loaded.map==YMapRoute2 && loaded.x==9 && loaded.y==0);
    s.map=YMapRoute3;
    assert(!yellow_save_load(&loaded,&s));
    assert(yellow_original_release_ready(NULL)==false);
}
int main(void) {
    source_geometry_and_collision();
    original_bidirectional_corridors();
    save_and_render_compatibility();
    puts("ROUTE3_SOURCE_BLOCKS_ORIGINAL_EAST_WEST_WARPS_REAL_INPUT_AND_V13_PASS");
    return 0;
}
