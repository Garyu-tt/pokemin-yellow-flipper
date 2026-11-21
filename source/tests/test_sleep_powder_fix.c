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
    g->story=YStoryPikachuFollows;g->has_pikachu=1;
    g->map=YMapRoute1;g->x=10;g->y=28;g->follower_valid=0;
    g->screen=YScreenWorld;g->hp=yellow_player_max_hp(g);
    yellow_begin_battle(g,YK_RATTATA,3);
}

static void test_sleep_powder_application(void) {
    YellowGame g;
    
    // Test 1: Sleep Powder should work on foe with no status
    ready(&g);
    g.rng = 1;
    battle_action_pipeline(&g, true, YM_SLEEP_POWDER, false);
    assert(g.foe_status == YStatusSleep1);
    
    // Test 2: Sleep Powder should NOT work on foe that already has sleep status
    ready(&g);
    g.foe_status = YStatusSleep1; // Already sleeping
    g.rng = 1;
    battle_action_pipeline(&g, true, YM_SLEEP_POWDER, false);
    assert(g.foe_status == YStatusSleep1); // Should remain sleeping
    
    // Test 3: Sleep Powder should work on player's party member with no status
    ready(&g);
    g.rng = 1;
    battle_action_pipeline(&g, false, YM_SLEEP_POWDER, false);
    assert(g.pika_status == YStatusSleep1);
    
    // Test 4: Sleep Powder should NOT work on player's party member that already has sleep status
    ready(&g);
    g.pika_status = YStatusSleep2; // Already sleeping
    g.rng = 1;
    battle_action_pipeline(&g, false, YM_SLEEP_POWDER, false);
    assert(g.pika_status == YStatusSleep2); // Should remain sleeping
    
    puts("SLEEP_POWDER_FIX_TEST_PASS");
}

int main(void) {
    test_sleep_powder_application();
    puts("ALL_TESTS_PASS");
    return 0;
}