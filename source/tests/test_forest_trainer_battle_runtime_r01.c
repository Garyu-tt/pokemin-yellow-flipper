#include <assert.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
int main(void){
 YellowGame g;yellow_init(&g);
 g.story=YStoryPikachuFollows;g.map=YMapViridianForest;
 g.screen=YScreenWorld;g.has_pikachu=1;g.follower_valid=0;
 g.x=28;g.y=33;g.facing=YDown;
 yellow_input(&g,YRight);
 assert(g.screen==YScreenDialog && g.x==29 && g.y==33);
 yellow_input(&g,YOk);
 assert(g.screen==YScreenWorld);
 yellow_input(&g,YOk);
 assert(g.screen==YScreenRivalBattle);
 assert(g.forest_active_trainer==0 && g.foe_kind==YK_CATERPIE && g.foe_level==7);
 finalize_battle(&g,true);
 assert(g.forest_party_index==1 && g.foe_kind==YK_CATERPIE && g.forest_defeated_mask==0);
 finalize_battle(&g,true);
 assert(g.forest_active_trainer==255 && (g.forest_defeated_mask&1));
 yellow_input(&g,YOk);
 assert(g.screen==YScreenWorld);
 puts("FOREST_TRAINER_BATTLE_RUNTIME_PASS");
 return 0;
}
