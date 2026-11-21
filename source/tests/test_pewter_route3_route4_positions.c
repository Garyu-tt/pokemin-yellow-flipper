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
    g->running=true;
    g->screen=YScreenWorld;
    g->story=YStoryPikachuFollows;
    g->has_pikachu=1;
    g->follower_valid=0;
    g->map=map;
    g->x=(int8_t)x;
    g->y=(int8_t)y;
}

static void test_exact_position_restore(void) {
    // Test Pewter City to Route3 horizontal connection
    YellowGame g;
    ready(&g,YMapPewterCity,39,16);
    
    // Move right from Pewter City to Route3 - should work with exact positions
    yellow_input(&g,YRight);
    assert(g.map==YMapRoute3);
    assert(g.x==0); 
    assert(g.y==8);  // Should be y-8 as per connection logic
    
    // Move back to Pewter City
    yellow_input(&g,YLeft);
    assert(g.map==YMapPewterCity);
    assert(g.x==39);
    assert(g.y==16);
    
    // Test Route3 to Route4 vertical connection
    ready(&g,YMapRoute3,57,0);
    yellow_input(&g,YUp);
    assert(g.map==YMapRoute4);
    assert(g.x==7);  // Should be 57-50=7
    assert(g.y==17);
    
    // Move back to Route3
    yellow_input(&g,YDown);
    assert(g.map==YMapRoute3);
    assert(g.x==57);
    assert(g.y==0);
}

static void test_save_restore_positions(void) {
    YellowGame g, loaded;
    YellowSave save;
    
    // Test saving and loading exact positions
    ready(&g,YMapRoute3,10,12);
    g.follower_valid=1;
    g.follower_map=YMapRoute3;
    g.follower_x=11;
    g.follower_y=13;
    
    yellow_save_create(&g,&save);
    assert(save.version==13);
    assert(save.map==YMapRoute2);  // Should be redirected to Route2 for compatibility
    assert(save.x==9);
    assert(save.y==0);
    
    // Load the save
    assert(yellow_save_load(&loaded,&save));
    assert(loaded.map==YMapRoute2);
    assert(loaded.x==9);
    assert(loaded.y==0);
    
    // Test with Route4 positions
    ready(&g,YMapRoute4,20,5);
    g.follower_valid=1;
    g.follower_map=YMapRoute4;
    g.follower_x=21;
    g.follower_y=6;
    
    yellow_save_create(&g,&save);
    assert(save.version==13);
    assert(save.map==YMapRoute2);  // Should be redirected to Route2 for compatibility
    assert(save.x==9);
    assert(save.y==0);
}

int main(void) {
    test_exact_position_restore();
    test_save_restore_positions();
    puts("PEWTER_ROUTE3_ROUTE4_POSITIONS_EXACT_RESTORE_AND_SAVE_COMPATIBILITY");
    return 0;
}