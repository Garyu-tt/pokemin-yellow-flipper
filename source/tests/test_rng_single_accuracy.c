#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"

/* Validate exactly one pre-effect accuracy roll in the shared battle pipeline.
   Gen1 ParalyzeEffect_ calls MoveHitTest once; no critical/damage RNG.
   This test is NEW and does not alter the trusted seed regression suite. */
static void setup(YellowGame* g) {
    yellow_init(g);
    g->story=YStoryPikachuFollows;
    g->has_pikachu=1;
    g->screen=YScreenWorld;
    g->map=YMapRoute1;
    g->x=10; g->y=28;
    g->hp=yellow_player_max_hp(g);
    g->follower_valid=0;
    yellow_begin_battle(g,YK_RATTATA,3);
}

static void source_pinned_single_accuracy_advance(void) {
    YellowGame g;
    setup(&g);
    g.rng=1u;
    YellowGame hit_probe=g;
    uint8_t hit_roll=(uint8_t)next_rng(&hit_probe);
    assert(y_move_hits(YM_STUN_SPORE,0,0,hit_roll));
    YellowGame predict=g;
    next_rng(&predict); /* accuracy */
    battle_action_pipeline(&g,true,YM_STUN_SPORE,false);
    assert(g.foe_status==YStatusParalyzed);
    assert(g.rng==predict.rng);
    setup(&g);
    g.rng=1u;
    predict=g;
    next_rng(&predict); /* exact one MoveHitTest */
    battle_action_pipeline(&g,false,YM_STUN_SPORE,false);
    assert(g.pika_status==YStatusParalyzed);
    assert(g.rng==predict.rng);
}
static void existing_status_not_replaced(void) {
    YellowGame g;
    setup(&g);
    g.rng=1u;
    g.foe_status=YStatusPoisoned;
    uint32_t rng0=g.rng;
    battle_action_pipeline(&g,true,YM_STUN_SPORE,false);
    assert(g.rng==rng0);
    assert(g.foe_status==YStatusPoisoned);
    setup(&g);
    g.rng=1u;
    g.pika_status=YStatusPoisoned;
    uint32_t rng1=g.rng;
    battle_action_pipeline(&g,false,YM_STUN_SPORE,false);
    assert(g.rng==rng1);
    assert(g.pika_status==YStatusPoisoned);
}
int main(void) {
    source_pinned_single_accuracy_advance();
    existing_status_not_replaced();
    puts("GEN1_STUN_SPORE_SINGLE_ACCURACY_RNG_PASS");
    return 0;
}
