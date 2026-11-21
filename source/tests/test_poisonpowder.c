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

static void poison_powder_status_application(void) {
    YellowGame g;ready(&g);
    
    // Test 1: Player uses Poison Powder on foe
    g.rng=1;
    battle_action_pipeline(&g,true,YM_POISONPOWDER,false);
    assert(g.foe_status==YStatusPoisoned);
    
    // Test 2: Foe uses Poison Powder on player
    ready(&g);
    g.rng=1;
    battle_action_pipeline(&g,false,YM_POISONPOWDER,false);
    assert(g.pika_status==YStatusPoisoned);
    
    // Test 3: Poison Powder fails due to accuracy
    ready(&g);
    g.rng=255; // High RNG roll to miss
    battle_action_pipeline(&g,true,YM_POISONPOWDER,false);
    assert(g.foe_status==YStatusNone);
    
    // Test 4: Poison Powder fails if foe already has status
    ready(&g);
    g.foe_status=YStatusBurned;
    g.rng=1;
    battle_action_pipeline(&g,true,YM_POISONPOWDER,false);
    assert(g.foe_status==YStatusBurned); // Should remain burned
    
    puts("R12B_POISONPOWDER_STATUS_APPLICATION_PASS");
}

int main() {
    poison_powder_status_application();
    return 0;
}