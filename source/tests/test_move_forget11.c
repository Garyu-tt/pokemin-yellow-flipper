#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"

static void ready(YellowGame* g,uint8_t level) {
    yellow_init(g);
    g->story=YStoryPikachuFollows;g->has_pikachu=1;
    g->map=YMapRoute1;g->x=10;g->y=28;
    g->screen=YScreenWorld;g->follower_valid=0;
    g->pika_level=level;g->experience=y_exp_threshold(YK_PIKACHU,level);
    g->hp=yellow_player_max_hp(g);
    y_build_moveset(YK_PIKACHU,level,g->pika_moves);
    for(unsigned i=0;i<4;i++)g->pika_pp[i]=y_max_pp(g->pika_moves[i]);
}
static void prompt(YellowGame* g) {
    open_dialog(g,wild_victory,1,YPendingEndWildBattle);
    yellow_input(g,YOk);
    assert(g->screen==YScreenMoveForget);
}
static void test_replace_explicit_slot(void) {
    YellowGame g;ready(&g,20);
    uint8_t preserved=g.pika_moves[0];
    uint8_t preserved_pp=g.pika_pp[0];
    g.pika_pp[1]=3;
    yellow_learn_for_party(&g,0,YM_GROWL);
    assert(g.learn_count==1 && g.learn_moves[0]==YM_GROWL);
    assert(g.pika_moves[0]==preserved);
    prompt(&g);
    yellow_input(&g,YDown);
    assert(g.selected==1);
    yellow_input(&g,YOk);
    assert(g.screen==YScreenWorld);
    assert(g.pika_moves[0]==preserved && g.pika_pp[0]==preserved_pp);
    assert(g.pika_moves[1]==YM_GROWL && g.pika_pp[1]==40);
    assert(g.learn_count==0 && g.learn_index==0);
    YellowSave save;yellow_save_create(&g,&save);
    YellowGame copy;assert(yellow_save_load(&copy,&save));
    assert(copy.pika_moves[1]==YM_GROWL);
    puts("R11_MANUAL_REPLACE_SELECTED_MOVE_AND_PP_PASS");
}
static void test_skip_on_back_and_skip_option(void) {
    YellowGame g;ready(&g,20);
    uint8_t before[4],pp[4];
    memcpy(before,g.pika_moves,sizeof(before));
    memcpy(pp,g.pika_pp,sizeof(pp));
    yellow_learn_for_party(&g,0,YM_GROWL); /* potentially already learned? no, overwritten */
    assert(g.learn_count==1);
    prompt(&g);
    yellow_input(&g,YBack);
    assert(g.screen==YScreenWorld);
    assert(!memcmp(g.pika_moves,before,sizeof(before)));
    assert(!memcmp(g.pika_pp,pp,sizeof(pp)));
    yellow_learn_for_party(&g,0,YM_GROWL);
    prompt(&g);
    for(unsigned i=0;i<4;i++)yellow_input(&g,YDown);
    assert(g.selected==4);
    yellow_input(&g,YOk);
    assert(g.screen==YScreenWorld);
    assert(!memcmp(g.pika_moves,before,sizeof(before)));
    puts("R11_BACK_AND_KEEP_OLD_SKIP_PASS");
}
static void test_queue_multiple(void) {
    YellowGame g;ready(&g,20);
    yellow_learn_for_party(&g,0,YM_GROWL);
    yellow_learn_for_party(&g,0,YM_TAIL_WHIP);
    assert(g.learn_count==2);
    prompt(&g);
    yellow_input(&g,YOk); /* replace slot 0 with GROWL */
    assert(g.screen==YScreenMoveForget);
    assert(g.learn_index==1 && g.selected==0);
    assert(g.pika_moves[0]==YM_GROWL);
    yellow_input(&g,YDown);
    yellow_input(&g,YDown);
    yellow_input(&g,YOk); /* replace slot 2 with TAIL WHIP */
    assert(g.screen==YScreenWorld);
    assert(g.pika_moves[2]==YM_TAIL_WHIP);
    assert(g.pika_moves[0]==YM_GROWL);
    assert(g.learn_count==0);
    YellowSave s;yellow_save_create(&g,&s);
    YellowGame copy;assert(yellow_save_load(&copy,&s));
    s.pika_moves[2]=YM_HYPER_FANG;s.checksum=checksum(&s);
    assert(!yellow_save_load(&copy,&s));
    puts("R11_MULTIPLE_QUEUED_PROMPTS_AND_RULE_GUARD_PASS");
}
static void test_no_auto_forget_and_no_duplicate(void) {
    YellowGame g;ready(&g,11);
    uint8_t b[4];memcpy(b,g.pika_moves,sizeof(b));
    yellow_learn_for_party(&g,0,YM_DOUBLE_TEAM);
    assert(g.learn_count==1);
    assert(!memcmp(b,g.pika_moves,sizeof(b)));
    yellow_learn_for_party(&g,0,YM_QUICK_ATTACK); /* already knows it */
    assert(g.learn_count==1);
    prompt(&g);
    yellow_input(&g,YBack);
    assert(g.screen==YScreenWorld);
    puts("R11_NO_SILENT_FORGET_OR_DUPLICATE_PASS");
}
static void test_legacy_migration(void) {
    YellowGame g;ready(&g,11);
    g.pika_pp[0]=4;g.balls=6;
    YellowSave old;yellow_save_create(&g,&old);
    old.version=10;old.checksum=checksum(&old);
    YellowGame copy;
    assert(!yellow_save_load(&copy,&old)); /* strict new loader */
    assert(yellow_save_migrate_r10(&copy,&old));
    assert(copy.pika_level==11&&copy.pika_pp[0]==4&&copy.balls==6);
    YellowSave upgraded;yellow_save_create(&copy,&upgraded);
    assert(upgraded.version==13);
    assert(yellow_save_load(&copy,&upgraded));
    assert(old.version==10); /* original untouched */
    old.checksum^=1;
    assert(!yellow_save_migrate_r10(&copy,&old));
    old.checksum^=1;old.magic=0;
    assert(!yellow_save_migrate_r10(&copy,&old));
    old.magic=YELLOW_SAVE_MAGIC;old.version=9;old.checksum=checksum(&old);
    assert(!yellow_save_migrate_r10(&copy,&old));
    puts("R11_GUARDED_R10_TO_R11_SAVE_MIGRATION_PASS");
}
static void test_battle_victory_exp_prompt(void) {
    YellowGame g;ready(&g,10);
    g.experience=y_exp_threshold(YK_PIKACHU,11)-1;
    g.foe_kind=YK_PIDGEY;g.foe_level=3;
    g.foe_hp=1;g.foe_max_hp=15;
    g.participants_mask=1;
    finalize_battle(&g,true);
    assert(g.pika_level==11 && g.learn_count==1);
    assert(g.pika_moves[0]==YM_THUNDERSHOCK);
    yellow_input(&g,YOk);
    assert(g.screen==YScreenMoveForget && g.learn_moves[0]==YM_QUICK_ATTACK);
    yellow_input(&g,YOk);
    assert(g.screen==YScreenWorld && g.pika_moves[0]==YM_QUICK_ATTACK);
    puts("R11_ACTUAL_BATTLE_LEVELUP_FORGET_PROMPT_PASS");
}
int main(void) {
    test_replace_explicit_slot();
    test_skip_on_back_and_skip_option();
    test_queue_multiple();
    test_no_auto_forget_and_no_duplicate();
    test_legacy_migration();
    test_battle_victory_exp_prompt();
    puts("R11_ALL_TESTS_PASS");
    return 0;
}
