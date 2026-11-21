#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"

#define MAX_CELLS 4096
static void init_world(YellowGame* g,uint8_t map,int x,int y) {
    yellow_init(g);
    g->running=true;g->screen=YScreenWorld;
    g->story=YStoryPikachuFollows;
    /* This test intentionally disables wild battles/follower while exercising
       real map movement through yellow_input. */
    g->has_pikachu=0;g->follower_valid=0;
    g->map=map;g->x=(int8_t)x;g->y=(int8_t)y;
}
static int walk_path(YellowGame* g,int target_x,int target_y) {
    const uint8_t map=g->map;
    int w=yellow_map_w(map),h=yellow_map_h(map);
    assert(w*h<=MAX_CELLS);
    assert(target_x>=0&&target_x<w&&target_y>=0&&target_y<h);
    int start=g->y*w+g->x,target=target_y*w+target_x;
    uint16_t queue[MAX_CELLS],prev[MAX_CELLS];
    uint8_t step[MAX_CELLS];
    for(int i=0;i<w*h;i++)prev[i]=UINT16_MAX;
    int front=0,back=0;
    prev[start]=(uint16_t)start;
    queue[back++]=(uint16_t)start;
    const int dx[]={0,0,-1,1},dy[]={-1,1,0,0};
    while(front<back && prev[target]==UINT16_MAX) {
        int here=queue[front++],x=here%w,y=here/w;
        for(int k=0;k<4;k++) {
            int nx=x+dx[k],ny=y+dy[k];
            if(nx<0||ny<0||nx>=w||ny>=h)continue;
            int next=ny*w+nx;
            if(prev[next]!=UINT16_MAX||!yellow_walkable(g,nx,ny))continue;
            uint8_t tm,tx,ty;
            if(next!=target && destination(map,nx,ny,&tm,&tx,&ty))continue;
            prev[next]=(uint16_t)here;
            step[next]=(uint8_t)k;
            queue[back++]=(uint16_t)next;
            assert(back<=MAX_CELLS);
        }
    }
    if(prev[target]==UINT16_MAX)return -1;
    uint8_t path[MAX_CELLS];int n=0;
    for(int cur=target;cur!=start;cur=prev[cur])path[n++]=step[cur];
    for(int i=n-1;i>=0;i--) {
        uint16_t old=g->steps;
        yellow_input(g,(YellowKey)path[i]);
        assert(g->steps==(uint16_t)(old+1));
        assert(g->screen==YScreenWorld);
        if(i>0)assert(g->map==map);
    }
    return n;
}
static void original_assets(void) {
    assert(YELLOW_MAP_COUNT==11 && YELLOW_LEVELS_COUNT==11);
    assert(yellow_map_w(YMapRoute2)==20 && yellow_map_h(YMapRoute2)==72);
    assert(yellow_map_w(YMapViridianForest)==34 && yellow_map_h(YMapViridianForest)==48);
    assert(yellow_map_w(YMapForestSouthGate)==10 && yellow_map_h(YMapForestSouthGate)==8);
    assert(yellow_map_w(YMapForestNorthGate)==10 && yellow_map_h(YMapForestNorthGate)==8);
    for(int m=0;m<YELLOW_MAP_COUNT;m++) {
        assert(yellow_levels[m].map && yellow_levels[m].blocks && yellow_levels[m].tiles);
        assert(yellow_levels[m].collision);
        for(int y=0;y<yellow_map_h(m)*2;y++)
          for(int x=0;x<yellow_map_w(m)*2;x++)
            assert(yellow_tile_id(m,x,y)!=0xff);
    }
    assert(yellow_tile_id(YMapViridianForest,-1,0)==0xff);
    assert(yellow_tile_id(YELLOW_MAP_COUNT,0,0)==0xff);
    puts("R13_PINNED_MAP_DIMENSIONS_AND_ALL_TILES_PASS");
}
static void route2_city_connection(void) {
    YellowGame g;init_world(&g,YMapViridian,18,0);
    yellow_input(&g,YUp);
    assert(g.map==YMapRoute2 && g.x==8 && g.y==71);
    assert(yellow_walkable(&g,g.x,g.y));
    assert(g.steps==1);
    yellow_input(&g,YDown);
    assert(g.map==YMapViridian && g.x==18 && g.y==0);
    assert(yellow_walkable(&g,g.x,g.y));
    init_world(&g,YMapRoute1,10,0);
    yellow_input(&g,YUp);
    assert(g.map==YMapViridian && g.x==20 && g.y==35);
    puts("R13_REAL_CITY_ROUTE2_AND_LEGACY_ROUTE1_CONNECTION_PASS");
}
static void forest_gate_roundtrip(void) {
    YellowGame g;init_world(&g,YMapRoute2,8,71);
    int steps=walk_path(&g,3,43);
    assert(steps>0 && g.map==YMapForestSouthGate && g.x==4 && g.y==6);
    assert(walk_path(&g,4,0)>0);
    assert(g.map==YMapViridianForest && g.x==16 && g.y==46);
    /* Forest is traversed by actual directional inputs, not direct map writes. */
    int forest_steps=walk_path(&g,1,0);
    assert(forest_steps>0 && g.map==YMapForestNorthGate);
    assert(g.x==4 && g.y==6);
    assert(walk_path(&g,4,0)>0);
    assert(g.map==YMapRoute2 && g.x==3 && g.y==10);
    assert(yellow_walkable(&g,g.x,g.y));
    /* Return via north gate and the forest's northern exit. */
    assert(walk_path(&g,3,11)>0);
    assert(g.map==YMapForestNorthGate && g.y==6);
    assert(walk_path(&g,4,7)>0);
    assert(g.map==YMapViridianForest && g.x==1 && g.y==1);
    /* Walk all the way back through the forest to the southern gate. */
    assert(walk_path(&g,16,47)>0);
    assert(g.map==YMapForestSouthGate && g.y==1);
    assert(walk_path(&g,4,7)>0);
    assert(g.map==YMapRoute2 && g.x==3 && g.y==44);
    puts("R13_ROUTE2_BOTH_GATES_FOREST_PATH_AND_BACKTRACK_PASS");
}
static void original_objects_and_dialogue(void) {
    YellowGame g;init_world(&g,YMapViridianForest,16,44);
    const YellowNpc* n=0;
    for(int i=0;i<yellow_npc_count;i++)
        if(yellow_npcs[i].map==YMapViridianForest &&
           yellow_npcs[i].x==16 && yellow_npcs[i].y==43)n=&yellow_npcs[i];
    assert(n && n->type==YActorYoungster);
    assert(yellow_npc_visible(&g,n));
    assert(!yellow_walkable(&g,16,43));
    g.facing=YUp;yellow_input(&g,YOk);
    assert(g.screen==YScreenDialog && g.dialog_lines==forest_trainer);
    yellow_input(&g,YOk);assert(g.screen==YScreenWorld);
    init_world(&g,YMapRoute2,13,55);g.facing=YUp;
    yellow_input(&g,YOk);
    assert(g.screen==YScreenDialog && g.dialog_lines==route2_item);
    assert(g.balls==0); /* A placeholder must never mint an unimplemented item. */
    int forest=0,route2=0,south=0,north=0;
    for(int i=0;i<yellow_npc_count;i++) {
        forest+=yellow_npcs[i].map==YMapViridianForest;
        route2+=yellow_npcs[i].map==YMapRoute2;
        south+=yellow_npcs[i].map==YMapForestSouthGate;
        north+=yellow_npcs[i].map==YMapForestNorthGate;
    }
    assert(forest==10 && route2==2 && south==2 && north==2);
    puts("R13_PINNED_NPC_COORDS_COLLISION_DIALOGUE_PLACEHOLDERS_PASS");
}
static void old_save_and_new_map_roundtrip(void) {
    YellowGame g,copy;
    init_world(&g,YMapViridianForest,16,46);
    /* A save is legal only if all existing invariants are valid. */
    g.has_pikachu=1;g.follower_valid=0;
    g.hp=yellow_player_max_hp(&g);
    YellowSave s;yellow_save_create(&g,&s);
    assert(yellow_save_load(&copy,&s));
    assert(copy.map==YMapViridianForest && copy.x==16 && copy.y==46);
    s.map=YELLOW_MAP_COUNT;s.checksum=checksum(&s);
    assert(!yellow_save_load(&copy,&s));
    init_world(&g,YMapViridian,18,0);
    g.has_pikachu=1;g.hp=yellow_player_max_hp(&g);
    yellow_save_create(&g,&s);
    assert(yellow_save_load(&copy,&s));
    assert(copy.map==YMapViridian);
    puts("R13_SAVE_V13_NEW_MAP_AND_OLD_MAP_VALIDATION_PASS");
}
int main(void) {
    setvbuf(stdout,NULL,_IONBF,0);
    original_assets();
    route2_city_connection();
    forest_gate_roundtrip();
    original_objects_and_dialogue();
    old_save_and_new_map_roundtrip();
    puts("R13_INDEPENDENT_BEHAVIORAL_NAVIGATION_ALL_PASS");
    return 0;
}
