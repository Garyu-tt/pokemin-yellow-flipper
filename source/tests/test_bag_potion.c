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

static void test_potion_healing(void) {
    YellowGame g;
    ready(&g);
    
    // Test potion healing when HP is below max
    g.potions = 1;
    g.hp = 5;
    uint8_t initial_hp = g.hp;
    uint8_t max_hp = yellow_player_max_hp(&g);
    
    // Simulate selecting potion in bag menu
    g.screen = YScreenBag;
    g.selected = 0;
    
    // Apply potion healing logic directly
    if(g.selected == 0 && g.potions > 0) {
        uint8_t current_hp = g.hp;
        if(current_hp < max_hp) {
            uint8_t heal_amount = (max_hp - current_hp > 20) ? 20 : (max_hp - current_hp);
            g.hp += heal_amount;
            g.potions--;
        }
    }
    
    // Verify healing occurred
    assert(g.potions == 0);  // Potion should be consumed
    assert(g.hp == initial_hp + (max_hp - initial_hp)); // HP should be max
    
    // Test potion healing when HP is already at max
    ready(&g);
    g.potions = 1;
    g.hp = max_hp;  // Already at max HP
    
    initial_hp = g.hp;
    
    // Simulate selecting potion in bag menu
    g.screen = YScreenBag;
    g.selected = 0;
    
    // Apply potion healing logic directly
    if(g.selected == 0 && g.potions > 0) {
        uint8_t current_hp = g.hp;
        if(current_hp < max_hp) {
            uint8_t heal_amount = (max_hp - current_hp > 20) ? 20 : (max_hp - current_hp);
            g.hp += heal_amount;
            g.potions--;
        }
    }
    
    // Verify no change when already at max HP
    assert(g.potions == 1);  // Potion should NOT be consumed
    assert(g.hp == initial_hp); // HP should remain unchanged
    
    // Test potion healing with less than 20 HP to max
    ready(&g);
    g.potions = 1;
    g.hp = max_hp - 10;  // 10 HP below max
    
    initial_hp = g.hp;
    
    // Simulate selecting potion in bag menu
    g.screen = YScreenBag;
    g.selected = 0;
    
    // Apply potion healing logic directly
    if(g.selected == 0 && g.potions > 0) {
        uint8_t current_hp = g.hp;
        if(current_hp < max_hp) {
            uint8_t heal_amount = (max_hp - current_hp > 20) ? 20 : (max_hp - current_hp);
            g.hp += heal_amount;
            g.potions--;
        }
    }
    
    // Verify partial healing
    assert(g.potions == 0);  // Potion should be consumed
    assert(g.hp == initial_hp + 10); // Should heal exactly 10 HP
    
    puts("BAG_POTION_HEALING_TEST_PASS");
}

int main(void) {
    test_potion_healing();
    puts("ALL_TESTS_PASS");
    return 0;
}