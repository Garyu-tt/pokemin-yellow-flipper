#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"

static void prepare(YellowGame* g) {
    yellow_init(g);
    g->map=YMapRoute1;g->x=10;g->y=28;
    g->screen=YScreenWorld;g->story=YStoryPikachuFollows;
    g->has_pikachu=1;g->pika_level=5;g->hp=yellow_player_max_hp(g);
    g->follower_valid=0;
    YCapturedMon* mon=&g->caught[0];
    memset(mon,0,sizeof(*mon));
    mon->species=YK_PIDGEY;mon->level=5;
    mon->experience=y_exp_threshold(YK_PIDGEY,5);
    mon->hp=y_calc_stats(YK_PIDGEY,5).hp;
    y_build_moveset(mon->species,mon->level,mon->moves);
    for(unsigned j=0;j<4;j++)mon->pp[j]=y_max_pp(mon->moves[j]);
    mon->pp[1]=y_max_pp(YM_SAND_ATTACK);
    g->caught_count=1;g->active_party=0;
}
static void test_voluntary_switch_exp(void) {
    YellowGame g;prepare(&g);
    yellow_begin_battle(&g,YK_RATTATA,7);
    assert(g.participants_mask==1);
    yellow_input(&g,YOk);
    assert(g.screen==YScreenFightMenu);
    /* at Lv5: ThunderShock / Growl / Ball / Run / Switch */
    for(int i=0;i<4;i++)yellow_input(&g,YDown);
    assert(g.selected==4);
    yellow_input(&g,YOk);
    assert(g.screen==YScreenParty&&g.party_battle&&!g.force_switch);
    assert(!yellow_can_switch(&g,0)&&yellow_can_switch(&g,1));
    yellow_input(&g,YDown);
    assert(g.selected==1);
    yellow_input(&g,YOk);
    assert(g.active_party==1&&g.participants_mask==3);
    assert(g.screen==YScreenRivalBattle);
    assert(g.caught[0].hp>0);
    assert(yellow_active_move_count(&g)==2);
    g.foe_hp=1; /* controlled deterministic terminal strike */
    yellow_input(&g,YOk);
    assert(g.screen==YScreenFightMenu);
    uint8_t before=g.caught[0].pp[0];
    yellow_input(&g,YOk);
    assert(g.screen==YScreenDialog);
    assert(g.caught[0].pp[0]==before-1);
    uint16_t award=y_exp_award(YK_RATTATA,7,false);
    assert(g.last_exp_award==award);
    assert(g.experience==125+(award/2)+(award%2));
    assert(g.caught[0].experience==y_exp_threshold(YK_PIDGEY,5)+award/2);
    while(g.screen==YScreenDialog)yellow_input(&g,YOk);
    assert(g.screen==YScreenWorld);
    puts("R07_SWITCH_EXP_SPLIT_AND_INDEPENDENT_PP_PASS");
}
static void test_forced_switch_and_defeat(void) {
    YellowGame g;prepare(&g);
    yellow_begin_battle(&g,YK_RATTATA,7);
    g.hp=1;
    for(int n=0;n<35&&g.screen==YScreenRivalBattle;n++)enemy_turn(&g);
    assert(g.hp==0);
    assert(g.screen==YScreenParty&&g.party_battle&&g.force_switch);
    assert(g.selected==1);
    yellow_input(&g,YBack);
    assert(g.screen==YScreenParty); /* cannot close forced switch */
    uint8_t hp=g.caught[0].hp;
    yellow_input(&g,YOk);
    assert(g.active_party==1&&g.screen==YScreenRivalBattle);
    assert(g.caught[0].hp==hp); /* forced switch has no retaliatory enemy turn */
    /* Pika fainted: surviving caught mon receives all XP. */
    g.foe_hp=1;
    yellow_input(&g,YOk);yellow_input(&g,YOk);
    assert(g.screen==YScreenDialog);
    assert(g.caught[0].experience==y_exp_threshold(YK_PIDGEY,5)+
       y_exp_award(YK_RATTATA,7,false));
    while(g.screen==YScreenDialog)yellow_input(&g,YOk);
    yellow_heal_pikachu(&g);
    assert(g.hp>0&&g.caught[0].hp==y_calc_stats(YK_PIDGEY,g.caught[0].level).hp);
    puts("R07_FORCED_SWITCH_FAINT_EXP_AND_HEAL_PASS");
}
static void test_total_party_defeat(void) {
    YellowGame g;prepare(&g);
    g.caught[0].hp=0;g.hp=1;
    yellow_begin_battle(&g,YK_RATTATA,7);
    for(int n=0;n<35&&g.screen==YScreenRivalBattle;n++)enemy_turn(&g);
    assert(g.screen==YScreenDialog);
    assert(g.hp==yellow_player_max_hp(&g));
    assert(g.caught[0].hp==y_calc_stats(YK_PIDGEY,5).hp);
    assert(g.map==YMapRedsHouse&&g.active_party==0);
    puts("R07_TOTAL_FAINT_WHITEOUT_PASS");
}
static void test_save_and_corrupt_active(void) {
    YellowGame g;prepare(&g);
    g.active_party=1;
    g.caught[0].pp[0]=13;
    YellowSave s;yellow_save_create(&g,&s);
    assert(s.version==13&&s.active_party==1);
    YellowGame copy;
    assert(yellow_save_load(&copy,&s));
    assert(copy.active_party==1 && copy.caught[0].pp[0]==13);
    s.active_party=4;s.checksum=checksum(&s);
    assert(!yellow_save_load(&copy,&s));
    s.active_party=1;s.caught[0].hp=0;s.checksum=checksum(&s);
    assert(yellow_save_load(&copy,&s));
    assert(copy.active_party==0); /* recovery from fainted active after relaunch */
    puts("R07_ACTIVE_SAVE_RESTORE_VALIDATION_PASS");
}
static void test_overworld_active_selection(void) {
    YellowGame g;prepare(&g);
    yellow_input(&g,YBack);assert(g.screen==YScreenMenu);
    yellow_input(&g,YOk);assert(g.screen==YScreenParty);
    yellow_input(&g,YDown);yellow_input(&g,YOk);
    assert(g.active_party==1&&g.screen==YScreenParty);
    yellow_input(&g,YBack);assert(g.screen==YScreenMenu);
    puts("R07_OVERWORLD_ACTIVE_PARTY_SELECTION_PASS");
}
int main(void) {
    test_voluntary_switch_exp();
    test_forced_switch_and_defeat();
    test_total_party_defeat();
    test_save_and_corrupt_active();
    test_overworld_active_selection();
    puts("R07_ALL_TESTS_PASS");
    return 0;
}
