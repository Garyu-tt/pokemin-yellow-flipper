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

static void test_stun_spore_application(void) {
    YellowGame g;
    
    // Test that stun spore applies paralysis correctly
    ready(&g);
    g.rng = 1;
    battle_action_pipeline(&g, true, YM_STUN_SPORE, false);
    
    // Enemy should be paralyzed (YStatusParalyzed)
    assert(g.foe_status == YStatusParalyzed);
    
    // Test that stun spore misses when accuracy check fails
    ready(&g);
    g.player_accuracy_stage = -3; // Very low accuracy
    g.rng = 255; // Guaranteed miss
    battle_action_pipeline(&g, true, YM_STUN_SPORE, false);
    
    // Enemy should not be paralyzed (missed)
    assert(g.foe_status == YStatusNone);
    
    // Test that stun spore doesn't apply if enemy already has a status
    ready(&g);
    g.foe_status = YStatusParalyzed;
    g.rng = 1;
    battle_action_pipeline(&g, true, YM_STUN_SPORE, false);
    
    // Enemy should still be paralyzed, not affected by stun spore
    assert(g.foe_status == YStatusParalyzed);
    
    // Test that stun spore applies to player's party
    ready(&g);
    g.rng = 1;
    battle_action_pipeline(&g, false, YM_STUN_SPORE, false);
    
    // Player's Pikachu should be paralyzed (YStatusParalyzed)
    assert(g.pika_status == YStatusParalyzed);
    
    // Test that stun spore misses for player's party
    ready(&g);
    g.foe_accuracy_stage = -3; // Very low accuracy 
    g.rng = 255; // Guaranteed miss
    battle_action_pipeline(&g, false, YM_STUN_SPORE, false);
    
    // Player's Pikachu should not be paralyzed (missed)
    assert(g.pika_status == YStatusNone);
    
    puts("STUN_SPORE_APPLICATION_TEST_PASS");
}

int main(void) {
    test_stun_spore_application();
    puts("ALL_TESTS_PASS");
    return 0;
}