#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"

static void route(YellowGame* g,int tx,int ty){
    const int width=yellow_map_w(g->map),height=yellow_map_h(g->map);
    typedef struct {int x,y,parent,dir;} Node;
    Node node[1440];
    int seen[1440];
    assert(width*height<=1440);
    for(int i=0;i<1440;i++)seen[i]=-1;
    node[0]=(Node){g->x,g->y,-1,0};int count=1,end=-1;
    seen[g->y*width+g->x]=0;
    for(int cur=0;cur<count;cur++){
        Node n=node[cur];
        if(n.x==tx&&n.y==ty){end=cur;break;}
        static const int d[][2]={{0,-1},{0,1},{-1,0},{1,0}};
        for(int dir=0;dir<4;dir++){
            int x=n.x+d[dir][0],y=n.y+d[dir][1];
            if(x<0||y<0||x>=width||y>=height)continue;
            if(seen[y*width+x]>=0||!yellow_walkable(g,x,y))continue;
            assert(count<1440);
            seen[y*width+x]=count;
            node[count++]=(Node){x,y,cur,dir};
        }
    }
    if(end<0)fprintf(stderr,"ROUTE MISSING map=%d %d,%d -> %d,%d\n",g->map,g->x,g->y,tx,ty);
    assert(end>=0);
    int path[1440],n=0;
    while(node[end].parent>=0) {path[n++]=node[end].dir;end=node[end].parent;}
    /* disable wild encounters during navigation-only tests */
    uint8_t has=g->has_pikachu;g->has_pikachu=0;
    while(n--){
        yellow_input(g,(YellowKey)path[n]);
        assert(g->screen==YScreenWorld);
    }
    g->has_pikachu=has;
    assert(g->x==tx&&g->y==ty);
}
static void setup(YellowGame* g){
    yellow_init(g);g->screen=YScreenWorld;
    g->story=YStoryPikachuFollows;g->has_pikachu=1;
    g->map=YMapPallet;g->x=10;g->y=2;g->follower_valid=0;g->facing=YUp;
}
static void map_connections(void){
    YellowGame g;setup(&g);
    route(&g,10,0);
    yellow_input(&g,YUp);
    assert(g.map==YMapRoute1&&g.x==10&&g.y==35);
    printf("MAP_ROUTEPALLET_ENTER PASS\n");
    g.follower_valid=0;
    route(&g,10,0);
    yellow_input(&g,YUp);
    assert(g.map==YMapViridian&&g.x==20&&g.y==35);
    printf("MAP_ROUTE_VIRIDIAN_ENTER PASS\n");
    yellow_input(&g,YDown);
    assert(g.map==YMapRoute1&&g.x==10&&g.y==0);
    printf("MAP_VIRIDIAN_ROUTE_RETURN PASS\n");
    g.follower_valid=0;
    route(&g,10,35);
    yellow_input(&g,YDown);
    assert(g.map==YMapPallet&&g.x==10&&g.y==0);
    printf("MAP_ROUTE_PALLET_RETURN PASS\n");
}
static void encounter_table(void){
    assert(yellow_is_grass(YMapRoute1,10,34));
    assert(!yellow_is_grass(YMapPallet,10,1));
    int sum=0;
    for(int i=0;i<10;i++){
        sum+=encounter_weights[i];
        assert(route1_levels[i]>=2&&route1_levels[i]<=7);
        assert(route1_kinds[i]==1||route1_kinds[i]==2);
    }
    assert(sum==256);
    printf("ROUTE1_WEIGHTED_SLOTS_PASS 256\n");
}
static void wild_loop(void){
    YellowGame g;setup(&g);
    g.map=YMapRoute1;g.x=10;g.y=35;g.facing=YUp;
    int found=0;
    for(uint32_t seed=1;seed<1000&&!found;seed++){
        g.rng=seed;g.screen=YScreenWorld;g.x=10;g.y=35;g.follower_valid=0;
        yellow_input(&g,YUp);
        if(g.screen==YScreenRivalBattle)found=1;
    }
    assert(found && g.foe_kind>=1 && g.foe_kind<=2);
    assert(g.foe_level>=2&&g.foe_level<=7);
    assert(g.foe_hp==g.foe_max_hp&&g.foe_max_hp>10);
    printf("ROUTE1_ENCOUNTER_PASS kind=%d level=%d hp=%d\n",g.foe_kind,g.foe_level,g.foe_hp);
    int count=0;
    while(g.screen==YScreenRivalBattle && count++<20){
        yellow_input(&g,YOk);
        assert(g.screen==YScreenFightMenu);
        yellow_input(&g,YOk);
    }
    assert(count<20&&g.screen==YScreenDialog);
    while(g.screen==YScreenDialog)yellow_input(&g,YOk);
    assert(g.screen==YScreenWorld&&g.story==YStoryPikachuFollows);
    printf("ROUTE1_WILD_BATTLE_PASS\n");
}
static void city_healing(void){
    YellowGame g;setup(&g);
    g.map=YMapViridian;g.x=23;g.y=26;g.hp=1;g.follower_valid=0;
    g.facing=YUp;
    yellow_input(&g,YUp); /* enter via real warp x23,25 */
    assert(g.map==YMapViridianCenter&&g.x==3&&g.y==6);
    g.follower_valid=0;
    route(&g,3,3);
    yellow_input(&g,YUp);
    yellow_input(&g,YOk); /* talk to Nurse */
    assert(g.screen==YScreenDialog&&g.hp==YELLOW_MAX_HP);
    yellow_input(&g,YOk);yellow_input(&g,YOk);
    assert(g.screen==YScreenWorld);
    route(&g,3,6);
    yellow_input(&g,YDown); /* real center exit */
    assert(g.map==YMapViridian&&g.x==23&&g.y==26);
    YellowSave save;yellow_save_create(&g,&save);
    YellowGame copy;
    assert(yellow_save_load(&copy,&save)&&copy.map==YMapViridian);
    printf("VIRIDIAN_POKECENTER_NURSE_HEAL_SAVE_PASS\n");
}

int main(){
    map_connections();
    encounter_table();
    wild_loop();
    city_healing();
    puts("R04_ALL_TESTS_PASS");
    return 0;
}
