#include <assert.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
static void ready(YellowGame* g,uint8_t kind){
 yellow_init(g);g->story=YStoryPikachuFollows;g->has_pikachu=1;g->screen=YScreenWorld;
 g->map=YMapRoute1;g->x=10;g->y=28;g->hp=yellow_player_max_hp(g);
 yellow_begin_battle(g,kind,5);g->pika_moves[0]=YM_TOXIC;g->pika_pp[0]=10;
 g->selected=0;g->active_party=0;g->rng=1;
}
int main(void){YellowGame g;uint32_t rng;
 ready(&g,YK_ZUBAT);rng=g.rng;battle_action_pipeline(&g,true,YM_TOXIC,false);
 assert(g.foe_status==YStatusNone && g.rng==rng && g.pika_pp[0]==9);
 ready(&g,YK_RATTATA);g.foe_status=YStatusParalyzed;rng=g.rng;
 battle_action_pipeline(&g,true,YM_TOXIC,false);
 assert(g.foe_status==YStatusParalyzed && g.rng==rng && g.pika_pp[0]==9);
 ready(&g,YK_RATTATA);rng=g.rng;battle_action_pipeline(&g,true,YM_TOXIC,false);
 assert(g.foe_status==YStatusToxic1 && g.rng!=rng && g.pika_pp[0]==9);
 puts("TOXIC_PREACCURACY_RNG_PASS");return 0;}
