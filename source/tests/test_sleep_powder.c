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

static void test_sleep_powder_application(void) {
    YellowGame g;
    ready(&g);
    
    // Test that sleep powder applies sleep status correctly
    g.rng = 1;
    battle_action_pipeline(&g, true, YM_SLEEP_POWDER, false);
    
    // Enemy should be put to sleep (YStatusSleep1)
    assert(g.foe_status == YStatusSleep1);
    // A sleeping foe must lose its action before it can cast Sleep Powder.
    g.rng = 1;
    battle_action_pipeline(&g, false, YM_SLEEP_POWDER, false);
    assert(g.pika_status == YStatusNone);
    assert(g.foe_status == YStatusNone); // wakes after skipped Sleep1 turn
    // Once awake, the foe can cast Sleep Powder on its next turn.
    g.rng = 1;
    battle_action_pipeline(&g, false, YM_SLEEP_POWDER, false);
    assert(g.pika_status == YStatusSleep1);

    // Test that sleep powder doesn't apply if enemy already has a status
    ready(&g);
    g.foe_status = YStatusParalyzed;
    g.rng = 1;
    battle_action_pipeline(&g, true, YM_SLEEP_POWDER, false);
    
    // Enemy should still be paralyzed, not slept
    assert(g.foe_status == YStatusParalyzed);
    
    // Test that sleep powder applies to player's party
    ready(&g);
    g.rng = 1;
    battle_action_pipeline(&g, false, YM_SLEEP_POWDER, false);
    
    // Player's Pikachu should be put to sleep (YStatusSleep1)
    assert(g.pika_status == YStatusSleep1);
    
    puts("SLEEP_POWDER_APPLICATION_TEST_PASS");
}

int main(void) {
    test_sleep_powder_application();
    puts("ALL_TESTS_PASS");
    return 0;
}