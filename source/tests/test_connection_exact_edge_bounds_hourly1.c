/* Independent regression: only a one-tile boundary crossing is a warp.
 * An invalid restored/off-grid coordinate must not teleport the player.
 * The original candidate test covers only normal on-grid crossings. */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"

static void ready(YellowGame* g,uint8_t map,int x,int y){
    yellow_init(g);
    g->running=true;g->screen=YScreenWorld;
    g->story=YStoryPikachuFollows;g->has_pikachu=1;
    g->follower_valid=0;g->map=map;
    g->x=(int8_t)x;g->y=(int8_t)y;
}
int main(void){
    uint8_t to=231,tx=232,ty=233;
    /* Valid single-step crossings retain exact reciprocal offsets. */
    assert(yellow_route3_horizontal_connection(YMapPewterCity,40,16,&to,&tx,&ty));
    assert(to==YMapRoute3&&tx==0&&ty==8);
    assert(yellow_route3_horizontal_connection(YMapRoute3,-1,8,&to,&tx,&ty));
    assert(to==YMapPewterCity&&tx==39&&ty==16);
    assert(yellow_route4_vertical_connection(YMapRoute3,57,-1,&to,&tx,&ty));
    assert(to==YMapRoute4&&tx==7&&ty==17);
    assert(yellow_route4_vertical_connection(YMapRoute4,7,18,&to,&tx,&ty));
    assert(to==YMapRoute3&&tx==57&&ty==0);
    /* Values >1 step outside a scene are invalid, even in a warp lane. */
    struct {uint8_t map;int x,y;int horizontal;} bad[]={
        {YMapPewterCity,41,16,1},{YMapPewterCity,127,19,1},
        {YMapRoute3,-2,8,1},{YMapRoute3,-128,11,1},
        {YMapRoute3,57,-2,0},{YMapRoute3,63,-128,0},
        {YMapRoute4,7,19,0},{YMapRoute4,13,127,0},
    };
    for(unsigned i=0;i<sizeof(bad)/sizeof(bad[0]);i++){
        to=231;tx=232;ty=233;
        bool warped=bad[i].horizontal?
            yellow_route3_horizontal_connection(bad[i].map,bad[i].x,bad[i].y,&to,&tx,&ty):
            yellow_route4_vertical_connection(bad[i].map,bad[i].x,bad[i].y,&to,&tx,&ty);
        if(warped || to!=231 || tx!=232 || ty!=233){
            fprintf(stderr,"INVALID_EDGE_WARP map=%u x=%d y=%d i=%u\n",
                    bad[i].map,bad[i].x,bad[i].y,i);
            return 40;
        }
    }
    /* Real yellow_input path: do not turn a corrupted off-grid x into warp. */
    YellowGame g;
    ready(&g,YMapPewterCity,40,16);
    yellow_input(&g,YRight);
    if(g.map!=YMapPewterCity){
        fprintf(stderr,"INVALID_RUNTIME_WARP pewter x40->41 map=%u\n",g.map);
        return 41;
    }
    ready(&g,YMapRoute3,57,-1);
    yellow_input(&g,YUp);
    if(g.map!=YMapRoute3){
        fprintf(stderr,"INVALID_RUNTIME_WARP route3 y-1->-2 map=%u\n",g.map);
        return 42;
    }
    puts("EXACT_ONE_TILE_EDGE_CROSSINGS_ONLY_PASS");
    return 0;
}
