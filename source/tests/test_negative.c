#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c" /* tcc -run single translation unit */

static void advance(YellowGame* g) {
    assert(g->screen==YScreenDialog);
    unsigned n=g->dialog_count;
    while(n--)yellow_input(g,YOk);
}
static void test_rival_defeat_and_recovery(void) {
    YellowGame g;yellow_init(&g);
    g.map=YMapOaksLab;g.x=7;g.y=4;
    g.story=YStoryRivalBattle;g.has_pikachu=1;
    yellow_begin_battle(&g,YK_EEVEE,5);
    g.hp=1;
    yellow_input(&g,YOk);
    assert(g.screen==YScreenFightMenu);
    yellow_input(&g,YDown);yellow_input(&g,YDown);yellow_input(&g,YDown);
    yellow_input(&g,YOk);
    assert(g.screen==YScreenRivalBattle); /* cannot flee introductory battle */
    for(int n=0;n<25 && g.screen==YScreenRivalBattle;n++) {
        yellow_input(&g,YOk);
        assert(g.screen==YScreenFightMenu);
        yellow_input(&g,YDown);
        yellow_input(&g,YOk); /* GROWL; eventually Eevee lands a hit */
    }
    assert(g.screen==YScreenDialog && g.hp==YELLOW_MAX_HP);
    advance(&g);assert(g.story==YStoryPikachuFollows);
    advance(&g);assert(g.screen==YScreenWorld && g.follower_valid);
    puts("R03_DEFEAT_AND_RUN_LOCK_PASS");
}
static void test_lab_and_pallet_warp_follow(void) {
    YellowGame g;yellow_init(&g);
    g.map=YMapOaksLab;g.x=4;g.y=10;
    g.story=YStoryPikachuFollows;g.has_pikachu=1;g.screen=YScreenWorld;
    set_follower_near(&g);
    assert(g.follower_valid);
    yellow_input(&g,YDown); /* face the follower first */
    yellow_input(&g,YDown); /* step through warp */
    assert(g.map==YMapPallet && g.x==12 && g.y==12);
    assert(g.follower_valid && g.follower_map==YMapPallet);
    assert(g.follower_x!=g.x || g.follower_y!=g.y);
    puts("R03_WARP_FOLLOWER_PASS");
}
static void test_invalid_saves(void) {
    YellowGame g;yellow_init(&g);
    advance(&g);
    YellowSave base;
    yellow_save_create(&g,&base);
    YellowSave s=base;s.map=200;s.checksum=checksum(&s);
    assert(!yellow_save_load(&g,&s));
    s=base;s.story=YStoryPikachuFollows;s.has_pikachu=0;s.checksum=checksum(&s);
    assert(!yellow_save_load(&g,&s));
    s=base;s.version=2;s.checksum=checksum(&s);
    assert(!yellow_save_load(&g,&s));
    s=base;s.x=255;s.checksum=checksum(&s);
    assert(!yellow_save_load(&g,&s));
    assert(yellow_save_load(&g,&base));
    puts("R03_SAVE_VALIDATION_PASS");
}
int main(void) {
    test_rival_defeat_and_recovery();
    test_lab_and_pallet_warp_follow();
    test_invalid_saves();
    puts("R03_NEGATIVE_TESTS_PASS");
    return 0;
}
