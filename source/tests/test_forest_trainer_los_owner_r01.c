#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
static void ready(YellowGame* g,int x,int y) {
 yellow_init(g);g->story=YStoryPikachuFollows;g->screen=YScreenWorld;
 g->map=YMapViridianForest;g->has_pikachu=1;g->hp=0;g->follower_valid=0;
 g->x=x;g->y=y;
}
int main(void) {
 YellowGame g;
 ready(&g,28,33);g.facing=YDown;
 assert(yellow_walkable(&g,29,33));
 yellow_input(&g,YRight);
 assert(g.x==29&&g.y==33&&g.screen==YScreenDialog);
 assert(g.dialog_lines==forest_trainer&&g.pending==YPendingNone);
 yellow_input(&g,YOk);
 assert(g.screen==YScreenWorld);
 ready(&g,27,33);g.facing=YDown;
 yellow_input(&g,YRight);
 assert(g.x==28&&g.y==33&&g.screen==YScreenDialog);
 ready(&g,31,33);g.facing=YDown;
 assert(yellow_walkable(&g,31,32));
 yellow_input(&g,YUp);
 assert(g.x==31&&g.y==32&&g.screen==YScreenWorld);
 ready(&g,16,44);g.facing=YDown;
 yellow_input(&g,YUp);
 assert(g.screen==YScreenWorld);
 puts("FOREST_TRAINER_LOS_OWNER_ASSISTED_RUNTIME_PASS");
 return 0;
}
