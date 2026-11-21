#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"

/* Source: pret/pokeyellow engine/battle/effects.asm CheckDefrost:
 * Fire-type moves with BURN_SIDE_EFFECT1/2 thaw a frozen target.
 * FIRE_SPIN uses TRAPPING_EFFECT and does not thaw.
 * Ember is now a sourced real move in the limited catalog.
 */
static void setup(YellowGame* g) {
    yellow_init(g);
    g->story=YStoryPikachuFollows;
    g->has_pikachu=1; g->screen=YScreenWorld;
    g->map=YMapRoute1; g->x=10; g->y=28;
    g->hp=yellow_player_max_hp(g);
    yellow_begin_battle(g,YK_RATTATA,3);
}

static void player_ember_defrosts_on_hit(void) {
    YellowGame g;
    setup(&g);
    g.foe_status=YStatusFrozen;
    g.rng=1u;
    uint8_t hp=g.foe_hp;
    battle_action_pipeline(&g,true,YM_EMBER,false);
    assert(g.foe_hp<hp && g.foe_hp>0);
    assert(g.foe_status==YStatusNone);
    assert(g.screen==YScreenRivalBattle);
}

static void foe_ember_defrosts_active_party(void) {
    YellowGame g;
    setup(&g);
    g.pika_status=YStatusFrozen;
    g.rng=1u;
    uint8_t hp=yellow_party_hp(&g,0);
    battle_action_pipeline(&g,false,YM_EMBER,false);
    assert(yellow_party_hp(&g,0)<hp && yellow_party_hp(&g,0)>0);
    assert(g.pika_status==YStatusNone);
    assert(g.screen==YScreenRivalBattle);
}

static void miss_and_nonfire_do_not_defrost(void) {
    YellowGame g;
    setup(&g);
    g.foe_status=YStatusFrozen;
    g.player_accuracy_stage=-6;
    g.rng=2u;
    YellowGame expected=g;
    uint8_t roll=(uint8_t)next_rng(&expected);
    assert(!y_move_hits(YM_EMBER,-6,0,roll));
    uint8_t hp=g.foe_hp;
    battle_action_pipeline(&g,true,YM_EMBER,false);
    assert(g.foe_status==YStatusFrozen);
    assert(g.foe_hp==hp);

    setup(&g);
    g.foe_status=YStatusFrozen;
    g.rng=1u;
    battle_action_pipeline(&g,true,YM_THUNDERSHOCK,false);
    assert(g.foe_status==YStatusFrozen);
}

static void fire_thaw_preserves_other_status(void) {
    YellowGame g;
    setup(&g);
    g.foe_status=YStatusPoisoned;
    g.rng=1u;
    battle_action_pipeline(&g,true,YM_EMBER,false);
    assert(g.foe_status==YStatusPoisoned);
    setup(&g);
    g.pika_status=YStatusParalyzed;
    g.rng=1u;
    battle_action_pipeline(&g,false,YM_EMBER,false);
    assert(g.pika_status==YStatusParalyzed);
}

/* A frozen battler loses its action without spending PP or drawing RNG.
 * Once an opposing Ember lands, its next action is legal. */
static void frozen_action_then_ember_recovery(void) {
    YellowGame g;
    setup(&g);
    g.pika_status=YStatusFrozen;
    g.selected=0;
    g.rng=1u;
    uint32_t rng=g.rng;
    uint8_t pp=g.pika_pp[0];
    uint8_t hp=g.foe_hp;
    battle_action_pipeline(&g,true,YM_THUNDERSHOCK,false);
    assert(g.pika_status==YStatusFrozen);
    assert(g.pika_pp[0]==pp);
    assert(g.rng==rng);
    assert(g.foe_hp==hp);
    battle_action_pipeline(&g,false,YM_EMBER,false);
    assert(g.pika_status==YStatusNone);
    assert(g.hp>0);
    battle_action_pipeline(&g,true,YM_THUNDERSHOCK,false);
    assert(g.pika_pp[0]==(uint8_t)(pp-1u));
}

int main(void) {
    assert(y_move_info(YM_EMBER)->type==YT_FIRE);
    assert(y_move_info(YM_EMBER)->power==40);
    assert(y_move_info(YM_EMBER)->accuracy==100);
    assert(y_move_info(YM_EMBER)->pp==25);
    frozen_action_then_ember_recovery();
    player_ember_defrosts_on_hit();
    foe_ember_defrosts_active_party();
    miss_and_nonfire_do_not_defrost();
    fire_thaw_preserves_other_status();
    puts("GEN1_FIRE_EMBER_THAW_PASS");
    return 0;
}
