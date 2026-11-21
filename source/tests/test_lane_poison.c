#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
static void ready(YellowGame* g) {
 yellow_init(g);g->story=YStoryPikachuFollows;g->has_pikachu=1;
 g->screen=YScreenWorld;g->map=YMapRoute1;g->x=10;g->y=28;
 g->hp=yellow_player_max_hp(g);yellow_begin_battle(g,YK_RATTATA,3);
}
int main(void) {
 YellowGame g;ready(&g);
 g.rng=1;g.selected=0;battle_action_pipeline(&g,true,YM_POISONPOWDER,false);
 assert(g.foe_status==YStatusPoisoned);
 ready(&g);g.rng=1;battle_action_pipeline(&g,false,YM_POISONPOWDER,false);
 assert(g.pika_status==YStatusPoisoned);
 ready(&g);g.foe_status=YStatusBurned;g.rng=1;
 battle_action_pipeline(&g,true,YM_POISONPOWDER,false);
 assert(g.foe_status==YStatusBurned);
 ready(&g);g.pika_status=YStatusBurned;g.rng=1;
 battle_action_pipeline(&g,false,YM_POISONPOWDER,false);
 assert(g.pika_status==YStatusBurned);
 puts("LANE_POISON_INDEPENDENT_STATUS_PASS");
 return 0;
}
