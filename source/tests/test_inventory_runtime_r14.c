#include <assert.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"

/* R0.14: live button-driven inventory, not a self-written simulation of Potion. */
static void ready(YellowGame* g) {
    yellow_init(g);
    g->story=YStoryPikachuFollows;
    g->has_pikachu=1;
    g->screen=YScreenWorld;
    g->map=YMapRoute1;
    g->x=10;
    g->y=28;
    g->follower_valid=0;
    g->active_party=0;
}
static void open_bag(YellowGame* g) {
    g->screen=YScreenMenu;
    g->selected=1;
    yellow_input(g,YOk);
    assert(g->screen==YScreenBag);
    assert(g->selected==0);
}
static void real_healing_and_depletion(void) {
    YellowGame g;ready(&g);
    uint8_t max=yellow_player_max_hp(&g);
    assert(max>5);
    g.hp=5;g.potions=2;g.balls=4;
    uint32_t initial_rng=g.rng;
    open_bag(&g);
    yellow_input(&g,YOk);
    assert(g.hp==(max-5>20?25:max));
    assert(g.potions==1);
    assert(g.balls==4 && g.rng==initial_rng);
    yellow_input(&g,YOk);
    assert(g.hp==max);
    assert(g.potions==1); /* No consumption when already at max */
    yellow_input(&g,YDown);
    assert(g.selected==1);
    yellow_input(&g,YOk);
    assert(g.balls==4 && g.potions==1); /* Balls are battle-only */
    yellow_input(&g,YBack);
    assert(g.screen==YScreenMenu && g.selected==1);
    open_bag(&g);
    g.hp=1;
    yellow_input(&g,YOk);
    assert(g.hp>1 && g.potions==0);
    g.hp=1;
    yellow_input(&g,YOk);
    assert(g.hp==1 && g.potions==0);
}
static void fainted_cannot_heal(void) {
    YellowGame g;ready(&g);
    g.hp=0;g.potions=2;
    open_bag(&g);
    yellow_input(&g,YOk);
    assert(g.hp==0 && g.potions==2);
}
static void save_round_trip_without_new_schema(void) {
    YellowGame g;ready(&g);
    g.potions=3;
    g.hp=5;
    open_bag(&g);
    yellow_input(&g,YOk);
    yellow_input(&g,YBack);
    YellowSave saved;
    yellow_save_create(&g,&saved);
    assert(saved.version==13 && saved.potions==2);
    YellowGame loaded;
    assert(yellow_save_load(&loaded,&saved));
    assert(loaded.potions==g.potions && loaded.hp==g.hp);
    assert(loaded.balls==g.balls);
}
int main(void) {
    real_healing_and_depletion();
    fainted_cannot_heal();
    save_round_trip_without_new_schema();
    puts("R14_REAL_BAG_POTION_RUNTIME_PASS");
    return 0;
}
