#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"

static void ready(YellowGame* g) {
    yellow_init(g);
    g->story=YStoryPikachuFollows;
    g->has_pikachu=1;g->screen=YScreenWorld;
    g->map=YMapRoute1;g->x=10;g->y=28;
    g->follower_valid=0;
    g->hp=yellow_player_max_hp(g);
    yellow_begin_battle(g,YK_RATTATA,3);
}
static void enemy_status_and_player_effects(void) {
    YellowGame g;ready(&g);
    g.screen=YScreenParty; /* reproduces voluntary switch boundary */
    g.rng=1;
    battle_action_pipeline(&g,false,YM_DOUBLE_TEAM,false);
    assert(g.screen==YScreenRivalBattle);
    assert(g.foe_evasion_stage==1);
    assert(y_move_accuracy_threshold(YM_TACKLE,0,g.foe_evasion_stage)<
           y_move_accuracy_threshold(YM_TACKLE,0,0));

    for(int i=0;i<20;i++) {
        g.rng=1;
        battle_action_pipeline(&g,false,YM_SAND_ATTACK,false);
    }
    assert(g.player_accuracy_stage==-6);
    for(int i=0;i<20;i++) {
        g.rng=1;
        battle_action_pipeline(&g,false,YM_GROWL,false);
        battle_action_pipeline(&g,false,YM_TAIL_WHIP,false);
    }
    assert(g.pika_attack_stage==-6);
    assert(g.pika_defense_stage==-6);
    for(int i=0;i<20;i++) {
        g.rng=1;
        battle_action_pipeline(&g,true,YM_DOUBLE_TEAM,false);
    }
    assert(g.player_evasion_stage==6);
    puts("R12B_SHARED_STAGE_DISPATCH_BOTH_SIDES_PASS");
}
static void electric_status_direct(void) {
    YellowGame g;ready(&g);
    g.foe_status=YStatusNone;
    g.rng=1;
    battle_action_pipeline(&g,true,YM_THUNDER_WAVE,false);
    assert(g.foe_status==YStatusParalyzed);
    /* Test enemy-initiated status via same dispatcher, not yet a move
       used by the reduced Route1 enemy move-selection policy. */
    g.rng=1;
    battle_action_pipeline(&g,false,YM_THUNDER_WAVE,false);
    assert(g.pika_status==YStatusParalyzed);
    g.rng=1;
    battle_action_pipeline(&g,false,YM_DOUBLE_TEAM,false);
    assert(g.foe_evasion_stage==1);
    yellow_heal_pikachu(&g);
    assert(g.pika_status==YStatusNone && g.hp==yellow_player_max_hp(&g));
    puts("R12B_SHARED_STATUS_DISPATCH_AND_HEAL_PASS");
}
static void damage_and_faint(void) {
    YellowGame g;ready(&g);
    uint8_t start=g.foe_hp;
    g.rng=3;
    battle_action_pipeline(&g,true,YM_THUNDERSHOCK,false);
    assert(g.foe_hp<start || g.screen==YScreenDialog);
    ready(&g);
    g.hp=1;
    g.foe_attack_stage=6;
    g.rng=1;
    battle_action_pipeline(&g,false,YM_TACKLE,false);
    assert(g.hp==yellow_player_max_hp(&g));
    assert(g.screen==YScreenDialog);
    puts("R12B_SHARED_DAMAGE_FAINT_WHITEOUT_PASS");
}
static void selected_pp_and_paralysis(void) {
    YellowGame g;ready(&g);
    uint8_t orig=g.pika_pp[0];
    g.selected=0;g.rng=1;
    battle_action_pipeline(&g,true,YM_THUNDERSHOCK,true);
    assert(g.pika_pp[0]==orig-1);
    /* Confirm a fully paralyzed player still spends PP, as modeled. */
    ready(&g);g.pika_status=YStatusParalyzed;
    int skip=0;
    for(uint32_t seed=1;seed<5000&&!skip;seed++) {
        g.rng=seed;
        if(y_full_paralysis((uint8_t)next_rng(&g))) {
            g.rng=seed;
            g.selected=0;
            uint8_t foe=g.foe_hp,pp=g.pika_pp[0];
            battle_action_pipeline(&g,true,YM_THUNDERSHOCK,true);
            assert(g.foe_hp==foe && g.pika_pp[0]==pp-1);
            skip=1;
        }
    }
    assert(skip);
    puts("R12B_PP_PARALYSIS_PIPELINE_PASS");
}
int main(void) {
    enemy_status_and_player_effects();
    electric_status_direct();
    damage_and_faint();
    selected_pp_and_paralysis();
    puts("R12B_ALL_TESTS_PASS");
    return 0;
}
