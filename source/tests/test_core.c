#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c" /* one translation unit for tcc -run */

static void advance(YellowGame* g) {
    assert(g->screen==YScreenDialog);
    unsigned count=g->dialog_count;
    for(unsigned i=0;i<count;i++)yellow_input(g,YOk);
}
typedef struct {int x,y,from,dir;} Node;
static void go_to(YellowGame* g,int tx,int ty) {
    assert(g->screen==YScreenWorld);
    assert(g->x>=0&&g->y>=0);
    const int w=yellow_map_w(g->map),h=yellow_map_h(g->map);
    assert(tx>=0&&ty>=0&&tx<w&&ty<h);
    Node nodes[400];
    int visited[400];
    for(int i=0;i<400;i++)visited[i]=-1;
    int n=1;
    nodes[0]=(Node){g->x,g->y,-1,-1};
    visited[g->y*w+g->x]=0;
    int end=-1;
    for(int current=0;current<n;current++) {
        Node v=nodes[current];
        if(v.x==tx&&v.y==ty){end=current;break;}
        static const int d[][2]={{0,-1},{0,1},{-1,0},{1,0}};
        for(int k=0;k<4;k++){
            int x=v.x+d[k][0],y=v.y+d[k][1];
            if(x<0||y<0||x>=w||y>=h||visited[y*w+x]>=0)continue;
            if(!yellow_walkable(g,x,y))continue;
            assert(n<400);
            visited[y*w+x]=n;
            nodes[n++]=(Node){x,y,current,k};
        }
    }
    if(end<0){fprintf(stderr,"route missing map=%u (%d,%d)->(%d,%d)\n",g->map,g->x,g->y,tx,ty);assert(end>=0);}
    int steps[400],num=0;
    while(nodes[end].from>=0){
        steps[num++]=nodes[end].dir;
        end=nodes[end].from;
    }
    while(num--){
        yellow_input(g,(YellowKey)steps[num]);
        assert(g->screen==YScreenWorld);
    }
    assert(g->x==tx&&g->y==ty);
}
static void select_menu(YellowGame* g,uint8_t item) {
    assert(g->screen==YScreenMenu);
    while(g->selected<item)yellow_input(g,YDown);
    while(g->selected>item)yellow_input(g,YUp);
    yellow_input(g,YOk);
}
int main(void) {
    YellowGame g;
    yellow_init(&g);
    assert(g.map==YMapRedsHouse && g.story==YStoryBeforeOak);
    printf("R03_INIT map=%u xy=%d,%d\n",g.map,g.x,g.y);
    advance(&g);
    assert(g.screen==YScreenWorld);
    go_to(&g,2,6);
    yellow_input(&g,YDown); /* original exit warp */
    assert(g.map==YMapPallet && g.x==5 && g.y==6);
    printf("R03_HOME_WARP_OK\n");
    go_to(&g,10,1);
    yellow_input(&g,YUp);
    assert(g.screen==YScreenDialog);
    advance(&g);
    assert(g.screen==YScreenOakEncounter && g.story==YStoryOakEncounter);
    printf("R03_OAK_TRIGGER_OK\n");
    yellow_input(&g,YOk);
    advance(&g);
    assert(g.screen==YScreenDialog && g.map==YMapOaksLab &&
           g.story==YStoryChooseStarter);
    advance(&g);
    assert(g.screen==YScreenWorld);
    printf("R03_CAPTURE_AND_LAB_OK\n");
    go_to(&g,7,4);
    yellow_input(&g,YUp);
    yellow_input(&g,YOk);
    assert(g.screen==YScreenDialog);
    advance(&g); /* rival grabs Eevee, Oak offers Pikachu */
    assert(g.has_pikachu==1 && g.story==YStoryRivalBattle);
    assert(g.screen==YScreenDialog);
    advance(&g);
    assert(g.screen==YScreenRivalBattle && g.foe_hp==YELLOW_RIVAL_MAX_HP);
    printf("R03_STARTER_AND_RIVAL_OK\n");
    int turns=0;
    while(g.screen==YScreenRivalBattle && turns++<15) {
        yellow_input(&g,YOk);
        assert(g.screen==YScreenFightMenu);
        yellow_input(&g,YOk); /* ThunderShock */
    }
    assert(turns<=15 && g.screen==YScreenDialog);
    advance(&g);
    assert(g.screen==YScreenDialog && g.story==YStoryPikachuFollows);
    advance(&g);
    assert(g.screen==YScreenWorld && g.has_pikachu && g.follower_valid);
    printf("R03_BATTLE_AND_FOLLOWER_OK HP=%u turns=%d\n",g.hp,turns);
    int oldx=g.x,oldy=g.y;
    yellow_input(&g,YDown); /* first opposite press turns to face Pikachu */
    assert(g.x==oldx && g.y==oldy && g.facing==YDown);
    yellow_input(&g,YOk); /* real follower interaction, not tile movement */
    assert(g.screen==YScreenDialog);
    advance(&g);
    assert(g.screen==YScreenWorld && g.affection>0);
    yellow_input(&g,YDown); /* second press swaps around follower */
    assert(g.y==oldy+1 && g.x==oldx);
    assert(g.follower_valid && g.follower_x==oldx && g.follower_y==oldy);
    printf("R03_FOLLOW_MOVEMENT_OK\n");
    go_to(&g,4,7);

    /* Save/load and corruption resistance */
    yellow_input(&g,YBack);
    select_menu(&g,2);
    assert(g.request_save);
    YellowSave s;
    yellow_save_create(&g,&s);
    yellow_input(&g,YBack);
    YellowGame restored;
    assert(yellow_save_load(&restored,&s));
    assert(restored.story==YStoryPikachuFollows);
    assert(restored.has_pikachu && restored.follower_valid);
    assert(restored.map==g.map && restored.x==g.x && restored.y==g.y);
    printf("R03_SAVE_RESTORE_OK\n");
    s.checksum++;
    assert(!yellow_save_load(&restored,&s));
    printf("R03_CHECKSUM_REJECT_OK\n");
    puts("R03_ALL_TESTS_PASS");
    return 0;
}
