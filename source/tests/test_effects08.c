#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"

static void prepare(YellowGame* g) {
    yellow_init(g);
    g->story=YStoryPikachuFollows;
    g->has_pikachu=1;
    g->screen=YScreenWorld;
    g->map=YMapRoute1;g->x=10;g->y=28;
    g->balls=5;
    g->follower_valid=0;
}
static void test_hit_stages(void) {
    assert(y_move_accuracy_threshold(YM_GUST,0,0)==255);
    assert(y_move_hits(YM_GUST,0,0,254));
    assert(!y_move_hits(YM_GUST,0,0,255));
    uint8_t lowered=y_move_accuracy_threshold(YM_GUST,-1,0);
    assert(lowered==168);
    assert(y_move_hits(YM_GUST,-1,0,167));
    assert(!y_move_hits(YM_GUST,-1,0,168));
    assert(y_move_accuracy_threshold(YM_GUST,-6,0)==63);
    assert(y_move_accuracy_threshold(YM_GUST,-7,0)==63);
    assert(y_move_accuracy_threshold(YM_TACKLE,0,+1)<y_move_accuracy_threshold(YM_TACKLE,0,0));
    puts("R08_ACCURACY_MODIFIERS_SOURCE_TABLE_PASS");
}
static void test_sand_attack_integration(void) {
    YellowGame g;prepare(&g);
    YCapturedMon* c=&g.caught[0];memset(c,0,sizeof(*c));
    c->species=YK_PIDGEY;c->level=5;c->experience=y_exp_threshold(YK_PIDGEY,5);
    c->hp=y_calc_stats(YK_PIDGEY,5).hp;
    y_build_moveset(c->species,c->level,c->moves);
    c->pp[0]=35;c->pp[1]=15;
    g.caught_count=1;g.active_party=1;
    yellow_begin_battle(&g,YK_RATTATA,3);
    g.rng=1; /* deterministic hit */
    yellow_input(&g,YOk);
    yellow_input(&g,YDown);
    uint8_t original_pp=g.caught[0].pp[1];
    yellow_input(&g,YOk);
    assert(g.foe_accuracy_stage==-1);
    assert(g.caught[0].pp[1]==original_pp-1);
    uint8_t old_acc=y_move_accuracy_threshold(YM_TACKLE,0,0);
    assert(y_move_accuracy_threshold(YM_TACKLE,g.foe_accuracy_stage,0)<old_acc);
    for(int i=0;i<9;i++) {
        if(g.screen==YScreenDialog || g.screen==YScreenParty)break;
        if(g.screen==YScreenRivalBattle)yellow_input(&g,YOk);
        else if(g.screen==YScreenFightMenu) {
            yellow_input(&g,YDown);
            yellow_input(&g,YOk);
        }
    }
    assert(g.foe_accuracy_stage>=-6);
    puts("R08_SAND_ATTACK_INTEGRATION_PASS");
}
static void test_thundershock_paralysis(void) {
    assert(y_thundershock_paralyzes(0,YStatusNone));
    assert(y_thundershock_paralyzes(25,YStatusNone));
    assert(!y_thundershock_paralyzes(26,YStatusNone));
    assert(!y_thundershock_paralyzes(0,YStatusParalyzed));
    assert(y_full_paralysis(62));
    assert(!y_full_paralysis(63));
    assert(y_effective_speed(YK_PIKACHU,5,YStatusParalyzed)
         < y_effective_speed(YK_PIKACHU,5,YStatusNone));
    YellowGame g;
    int found=0;
    for(uint32_t seed=1;seed<4000&&!found;seed++) {
        prepare(&g);
        yellow_begin_battle(&g,YK_PIDGEY,7);
        g.rng=seed;
        yellow_input(&g,YOk);
        yellow_input(&g,YOk); /* Thundershock */
        if(g.foe_status==YStatusParalyzed)found=1;
    }
    assert(found);
    assert(g.foe_hp>0);
    uint8_t foe_hp=g.foe_hp;
    int skipped=0;
    for(uint32_t seed=1;seed<5000&&!skipped;seed++) {
        g.rng=seed;
        if(y_full_paralysis((uint8_t)next_rng(&g))) {
            g.rng=seed;g.screen=YScreenRivalBattle;
            uint8_t my_hp=yellow_party_hp(&g,g.active_party);
            enemy_turn(&g);
            assert(g.foe_hp==foe_hp);
            assert(yellow_party_hp(&g,g.active_party)==my_hp);
            assert(g.screen==YScreenRivalBattle);
            skipped=1;
        }
    }
    assert(skipped);
    puts("R08_THUNDERSHOCK_STATUS_AND_SKIP_PASS");
}
static void test_capture_status_and_heal(void) {
    assert(!y_gen1_pokeball_capture_status(45,150,150,50,0,YStatusNone));
    assert(y_gen1_pokeball_capture_status(45,150,150,50,0,YStatusParalyzed));
    YellowGame g;prepare(&g);
    yellow_begin_battle(&g,YK_RATTATA,3);
    g.foe_hp=1;
    g.foe_status=YStatusParalyzed;
    yellow_input(&g,YOk);
    yellow_input(&g,YDown);yellow_input(&g,YDown);
    yellow_input(&g,YOk);
    assert(g.caught_count==1);
    assert(g.caught[0].status==YStatusParalyzed);
    assert(g.screen==YScreenDialog);
    while(g.screen==YScreenDialog)yellow_input(&g,YOk);
    g.pika_status=YStatusParalyzed;
    YellowSave save;yellow_save_create(&g,&save);
    assert(save.version==13);
    YellowGame copy;
    assert(yellow_save_load(&copy,&save));
    assert(copy.pika_status==YStatusParalyzed);
    assert(copy.caught[0].status==YStatusParalyzed);
    save.caught[0].status=27;save.checksum=checksum(&save);
    assert(!yellow_save_load(&copy,&save));
    save.caught[0].status=YStatusParalyzed;
    save.pika_status=27;save.checksum=checksum(&save);
    assert(!yellow_save_load(&copy,&save));
    yellow_heal_pikachu(&g);
    assert(g.pika_status==YStatusNone&&g.caught[0].status==YStatusNone);
    puts("R08_PARALYSIS_CAPTURE_HEAL_AND_SAVE_PASS");
}
static void test_player_full_paralysis(void) {
    YellowGame g;prepare(&g);
    yellow_begin_battle(&g,YK_RATTATA,2);
    g.pika_status=YStatusParalyzed;
    g.foe_status=YStatusParalyzed; /* both slowed, Pikachu still acts first */
    unsigned suppressed=0;
    for(uint32_t seed=1;seed<10000&&!suppressed;seed++) {
        /* battle consumes 1 RNG for paralysis roll after opening move screen */
        g.rng=seed;g.screen=YScreenRivalBattle;
        uint8_t hp=g.foe_hp;
        uint8_t pp=g.pika_pp[0];
        /* R0.9 first selects an enemy move and resolves turn priority;
           Pikachu is faster than Lv2 Rattata, so its paralysis roll is
           the THIRD draw from the deterministic random stream. */
        (void)choose_enemy_move(&g);
        (void)next_rng(&g);
        if(!y_full_paralysis((uint8_t)next_rng(&g)))continue;
        g.rng=seed;
        yellow_input(&g,YOk);
        yellow_input(&g,YOk);
        assert(g.foe_hp==hp);
        assert(g.pika_pp[0]==pp-1);
        suppressed=1;
    }
    assert(suppressed);
    puts("R08_ACTIVE_PARALYSIS_SUPPRESSION_PASS");
}
static void test_brock_onix_move_selection(void) {
    YellowGame g;prepare(&g);
    yellow_begin_battle(&g,YK_ONIX,12);
    unsigned tackle=0,screech=0;
    for(uint32_t seed=1;seed<=256;seed++) {
        g.rng=seed;
        uint8_t move=choose_enemy_move(&g);
        assert(move==YM_TACKLE || move==YM_SCREECH);
        tackle+=(move==YM_TACKLE);
        screech+=(move==YM_SCREECH);
    }
    assert(tackle && screech);
    puts("BROCK_ONIX_TWO_SOURCE_MOVES_PASS");
}
int main(void) {
    test_brock_onix_move_selection();
    test_hit_stages();
    test_sand_attack_integration();
    test_thundershock_paralysis();
    test_capture_status_and_heal();
    test_player_full_paralysis();
    puts("R08_ALL_TESTS_PASS");
    return 0;
}
