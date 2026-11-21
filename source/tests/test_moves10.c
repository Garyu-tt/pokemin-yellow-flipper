#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"

static void setup(YellowGame* g) {
    yellow_init(g);
    g->screen=YScreenWorld;
    g->story=YStoryPikachuFollows;
    g->map=YMapRoute1;g->x=10;g->y=28;g->has_pikachu=1;
    g->hp=yellow_player_max_hp(g);
    g->follower_valid=0;
}

static void verify_learnsets(void) {
    uint8_t moves[4];
    y_build_moveset(YK_PIKACHU,5,moves);
    assert(moves[0]==YM_THUNDERSHOCK&&moves[1]==YM_GROWL&&moves[2]==YM_NONE);
    y_build_moveset(YK_PIKACHU,8,moves);
    assert(moves[2]==YM_TAIL_WHIP && moves[3]==YM_THUNDER_WAVE);
    y_build_moveset(YK_PIKACHU,11,moves);
    assert(moves[0]==YM_GROWL&&moves[1]==YM_TAIL_WHIP);
    assert(moves[2]==YM_THUNDER_WAVE&&moves[3]==YM_QUICK_ATTACK);
    y_build_moveset(YK_PIKACHU,15,moves);
    assert(moves[0]==YM_TAIL_WHIP&&moves[3]==YM_DOUBLE_TEAM);
    y_build_moveset(YK_PIKACHU,20,moves);
    assert(moves[0]==YM_THUNDER_WAVE&&moves[3]==YM_SLAM);
    y_build_moveset(YK_PIKACHU,26,moves);
    assert(moves[0]==YM_QUICK_ATTACK&&moves[3]==YM_THUNDERBOLT);
    y_build_moveset(YK_PIDGEY,12,moves);
    assert(moves[0]==YM_GUST&&moves[1]==YM_SAND_ATTACK&&moves[2]==YM_QUICK_ATTACK);
    y_build_moveset(YK_RATTATA,14,moves);
    assert(moves[0]==YM_TACKLE&&moves[1]==YM_TAIL_WHIP);
    assert(moves[2]==YM_QUICK_ATTACK&&moves[3]==YM_HYPER_FANG);
    assert(y_move_available(YK_PIKACHU,8,YM_THUNDER_WAVE));
    assert(!y_move_available(YK_PIKACHU,7,YM_THUNDER_WAVE));
    assert(!y_move_available(YK_PIDGEY,6,YM_HYPER_FANG));
    assert(y_move_info(YM_THUNDER_WAVE)->pp==20);
    assert(y_move_info(YM_DOUBLE_TEAM)->pp==15);
    assert(y_move_info(YM_SLAM)->power==80);
    assert(y_move_info(YM_THUNDERBOLT)->power==95);
    assert(y_move_info(YM_HYPER_FANG)->power==80);
    puts("R10_SOURCE_LEARNSET_AND_FOUR_SLOTS_PASS");
}
static void exp_learning(void) {
    YellowGame g;setup(&g);
    g.pika_level=7;g.experience=y_exp_threshold(YK_PIKACHU,7);
    y_build_moveset(YK_PIKACHU,7,g.pika_moves);
    for(unsigned i=0;i<4;i++)g.pika_pp[i]=y_max_pp(g.pika_moves[i]);
    g.hp=yellow_player_max_hp(&g);
    g.foe_kind=YK_PIDGEY;g.foe_level=30;g.participants_mask=1;
    grant_exp(&g);
    assert(g.pika_level==8&&g.pika_moves[3]==YM_THUNDER_WAVE&&g.pika_pp[3]==20);
    g.pika_level=10;g.experience=y_exp_threshold(YK_PIKACHU,10);
    y_build_moveset(YK_PIKACHU,10,g.pika_moves);
    for(unsigned i=0;i<4;i++)g.pika_pp[i]=y_max_pp(g.pika_moves[i]);
    g.pika_pp[1]=7; /* preserve PP of pre-existing moves */
    g.hp=yellow_player_max_hp(&g);
    g.foe_level=50;
    grant_exp(&g);
    assert(g.pika_level==11&&g.pika_moves[0]==YM_THUNDERSHOCK);
    assert(g.learn_count==1&&g.learn_moves[0]==YM_QUICK_ATTACK);
    open_dialog(&g,wild_victory,1,YPendingEndWildBattle);
    yellow_input(&g,YOk);
    assert(g.screen==YScreenMoveForget);
    /* Replace slot 0 with Quick Attack, preserve slot 1 PP. */
    yellow_input(&g,YOk);
    assert(g.screen==YScreenWorld);
    assert(g.pika_moves[0]==YM_QUICK_ATTACK && g.pika_pp[0]==30);
    assert(g.pika_pp[1]==7 && g.learn_count==0);
    puts("R10_EXP_LEARN_THUNDER_WAVE_QUICK_ATTACK_PASS");
}
static void verify_electric_and_evasion(void) {
    YellowGame g;setup(&g);
    g.pika_level=8;g.experience=y_exp_threshold(YK_PIKACHU,8);
    y_build_moveset(YK_PIKACHU,8,g.pika_moves);
    for(unsigned i=0;i<4;i++)g.pika_pp[i]=y_max_pp(g.pika_moves[i]);
    g.hp=yellow_player_max_hp(&g);
    yellow_begin_battle(&g,YK_RATTATA,3);
    g.selected=3;
    int done=0;
    for(uint32_t seed=1;seed<1000&&!done;seed++) {
        g.foe_status=YStatusNone;g.pika_pp[3]=20;
        g.rng=seed;g.screen=YScreenRivalBattle;
        execute_player_action(&g,YM_THUNDER_WAVE);
        if(g.foe_status==YStatusParalyzed)done=1;
    }
    assert(done && g.pika_pp[3]==19);
    g.pika_level=15;g.experience=y_exp_threshold(YK_PIKACHU,15);
    y_build_moveset(YK_PIKACHU,15,g.pika_moves);
    for(unsigned i=0;i<4;i++)g.pika_pp[i]=y_max_pp(g.pika_moves[i]);
    g.selected=3;g.player_evasion_stage=0;
    g.rng=1;
    execute_player_action(&g,YM_DOUBLE_TEAM);
    assert(g.player_evasion_stage==1);
    assert(g.pika_pp[3]==14);
    assert(y_move_accuracy_threshold(YM_TACKLE,0,g.player_evasion_stage)<
           y_move_accuracy_threshold(YM_TACKLE,0,0));
    puts("R10_THUNDER_WAVE_DOUBLE_TEAM_EFFECTS_PASS");
}
static void verify_hyper_fang(void) {
    YellowGame g;setup(&g);
    YCapturedMon* mon=&g.caught[0];
    memset(mon,0,sizeof(*mon));
    mon->species=YK_RATTATA;mon->level=14;
    mon->experience=y_exp_threshold(YK_RATTATA,14);
    mon->hp=y_calc_stats(mon->species,mon->level).hp;
    y_build_moveset(mon->species,mon->level,mon->moves);
    for(unsigned i=0;i<4;i++)mon->pp[i]=y_max_pp(mon->moves[i]);
    g.caught_count=1;g.active_party=1;
    yellow_begin_battle(&g,YK_PIDGEY,2);
    g.foe_hp=200;g.foe_max_hp=200;
    int flinched=0;
    for(uint32_t seed=1;seed<6000&&!flinched;seed++) {
        g.rng=seed;g.foe_hp=200;g.foe_flinched=0;g.selected=3;
        g.caught[0].pp[3]=15;
        g.screen=YScreenRivalBattle;
        execute_player_action(&g,YM_HYPER_FANG);
        if(g.foe_flinched)flinched=1;
    }
    assert(flinched&&g.caught[0].pp[3]==14);
    puts("R10_HYPER_FANG_FLINCH_CHANCE_PASS");
}
static void persistence_validation(void) {
    YellowGame g;setup(&g);
    g.pika_level=11;g.experience=y_exp_threshold(YK_PIKACHU,11);
    g.hp=yellow_player_max_hp(&g);
    y_build_moveset(YK_PIKACHU,11,g.pika_moves);
    for(unsigned i=0;i<4;i++)g.pika_pp[i]=y_max_pp(g.pika_moves[i]);
    g.pika_pp[0]=3;
    YellowSave sv;yellow_save_create(&g,&sv);
    assert(sv.version==13);
    YellowGame copy;
    assert(yellow_save_load(&copy,&sv));
    assert(copy.pika_moves[3]==YM_QUICK_ATTACK && copy.pika_pp[0]==3);
    YellowSave bad=sv;
    bad.pika_moves[0]=YM_HYPER_FANG;bad.checksum=checksum(&bad);
    assert(!yellow_save_load(&copy,&bad));
    bad=sv;bad.pika_moves[2]=YM_NONE;bad.checksum=checksum(&bad);
    assert(!yellow_save_load(&copy,&bad));
    bad=sv;bad.pika_moves[3]=bad.pika_moves[0];bad.checksum=checksum(&bad);
    assert(!yellow_save_load(&copy,&bad));
    bad=sv;bad.pika_pp[1]=255;bad.checksum=checksum(&bad);
    assert(!yellow_save_load(&copy,&bad));
    puts("R10_MOVE_SAVE_RESTORE_AND_CORRUPTION_REJECTION_PASS");
}
int main(void) {
    verify_learnsets();
    exp_learning();
    verify_electric_and_evasion();
    verify_hyper_fang();
    persistence_validation();
    puts("R10_ALL_TESTS_PASS");
    return 0;
}
