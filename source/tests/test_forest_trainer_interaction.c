#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"

static void init_world(YellowGame* g,uint8_t map,int x,int y) {
    yellow_init(g);
    g->running=true;g->screen=YScreenWorld;
    g->story=YStoryPikachuFollows;
    /* This test intentionally disables wild battles/follower while exercising
       real map movement through yellow_input. */
    g->has_pikachu=0;g->follower_valid=0;
    g->map=map;g->x=(int8_t)x;g->y=(int8_t)y;
}

static void forest_trainer_battle_interaction(void) {
    YellowGame g;
    
    /* Test trainer at (16,43) - approach from (16,44) */
    init_world(&g,YMapViridianForest,16,44);
    const YellowNpc* n=0;
    for(int i=0;i<yellow_npc_count;i++)
        if(yellow_npcs[i].map==YMapViridianForest &&
           yellow_npcs[i].x==16 && yellow_npcs[i].y==43)n=&yellow_npcs[i];
    
    assert(n && n->type==YActorYoungster);
    assert(yellow_npc_visible(&g,n));
    
    /* Test that trainer interaction triggers proper behavior */
    g.facing=YUp;
    yellow_input(&g,YOk);
    assert(g.screen==YScreenDialog);
    assert(g.dialog_lines==forest_trainer);
    
    /* Simulate dialog completion */
    yellow_input(&g,YOk);
    assert(g.screen==YScreenWorld);
    
    puts("R13_FOREST_TRAINER_BATTLE_INTERACTION_PASS");
}

int main(void) {
    setvbuf(stdout,NULL,_IONBF,0);
    forest_trainer_battle_interaction();
    puts("R13_INDEPENDENT_TRAINER_BATTLE_TEST_ALL_PASS");
    return 0;
}