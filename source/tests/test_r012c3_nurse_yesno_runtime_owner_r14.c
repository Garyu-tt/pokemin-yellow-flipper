#include <assert.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
#include "../yellow_pewter_progress.c"
#include "../yellow_atomic_snapshot.c"
#include "../yellow_exact_position_v1.c"
static void ready(YellowGame* g){
 yellow_init(g);g->running=true;g->screen=YScreenWorld;
 g->story=YStoryPikachuFollows;g->has_pikachu=1;
 g->map=YMapViridianCenter;g->x=3;g->y=3;g->facing=YUp;
 g->native_nurse_choice_extension=true;g->hp=1;g->pika_status=YStatusPoisoned;
}
int main(void){
 YellowGame g;ready(&g);
 yellow_input(&g,YOk);
 assert(g.screen==YScreenNurseChoice && g.hp==1 && g.last_blackout_original_map==0);
 yellow_input(&g,YDown);yellow_input(&g,YOk);
 assert(g.screen==YScreenWorld && g.hp==1 && g.pika_status==YStatusPoisoned && g.last_blackout_original_map==0);
 yellow_input(&g,YOk);yellow_input(&g,YBack);
 assert(g.screen==YScreenWorld && g.hp==1 && g.last_blackout_original_map==0);
 yellow_input(&g,YOk);yellow_input(&g,YUp);yellow_input(&g,YOk);
 assert(g.screen==YScreenDialog && g.hp==yellow_player_max_hp(&g) && g.pika_status==YStatusNone);
 assert(g.last_blackout_original_map==1);
 yellow_input(&g,YOk);yellow_input(&g,YOk);
 assert(g.screen==YScreenWorld);
 ready(&g);g.last_blackout_integrity_error=true;
 yellow_input(&g,YOk);yellow_input(&g,YOk);
 assert(g.hp==yellow_player_max_hp(&g) && g.last_blackout_original_map==0 && g.last_blackout_integrity_error);
 puts("R14_NATIVE_NURSE_YES_NO_BACK_HEAL_AND_ANCHOR_PASS");return 0;
}
