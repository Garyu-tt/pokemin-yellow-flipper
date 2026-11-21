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
    g->screen=YScreenWorld;g->map=YMapRoute1;g->x=10;g->y=28;
    g->hp=yellow_player_max_hp(g);g->follower_valid=0;
    yellow_begin_battle(g,YK_RATTATA,3);
}
static void sleep_math(void) {
    assert(y_status_valid(YStatusNone));
    assert(y_status_valid(YStatusFrozen));
    assert(y_status_valid(YStatusSleep7));
    assert(y_status_valid(YStatusToxic15));
    assert(!y_status_valid(27));
    for(uint8_t i=YStatusSleep1;i<=YStatusSleep7;i++)
        assert(y_status_is_sleep(i));
    assert(!y_status_is_sleep(YStatusToxic1));
    assert(y_sleep_after_action(YStatusSleep1)==YStatusNone);
    assert(y_sleep_after_action(YStatusSleep7)==(uint8_t)(YStatusSleep7-1u));
    assert(y_sleep_after_action(YStatusFrozen)==YStatusFrozen);
    puts("R12C2_SLEEP_AND_STATUS_ENCODING_PASS");
}
static void action_suppression(void) {
    YellowGame g;ready(&g);
    g.pika_status=YStatusSleep2;
    uint8_t pp=g.pika_pp[0],old_hp=g.foe_hp;
    g.selected=0;
    execute_player_action(&g,YM_THUNDERSHOCK);
    assert(g.pika_status==YStatusSleep1 && g.pika_pp[0]==pp);
    assert(g.foe_hp==old_hp);
    execute_player_action(&g,YM_THUNDERSHOCK);
    assert(g.pika_status==YStatusNone && g.pika_pp[0]==pp);
    assert(g.foe_hp==old_hp);
    /* Wake-up turn is still lost; the next may consume PP. */
    execute_player_action(&g,YM_THUNDERSHOCK);
    assert(g.pika_pp[0]==pp-1);
    ready(&g);
    g.foe_status=YStatusSleep2;
    uint8_t before=g.hp;
    execute_enemy_action(&g,YM_TACKLE,true);
    assert(g.foe_status==YStatusSleep1 && g.hp==before);
    execute_enemy_action(&g,YM_TACKLE,true);
    assert(g.foe_status==YStatusNone && g.hp==before);
    ready(&g);
    g.pika_status=YStatusFrozen;g.selected=0;pp=g.pika_pp[0];
    for(int i=0;i<6;i++)execute_player_action(&g,YM_THUNDERSHOCK);
    assert(g.pika_pp[0]==pp && g.pika_status==YStatusFrozen);
    g.foe_status=YStatusFrozen;before=g.hp;
    for(int i=0;i<6;i++)execute_enemy_action(&g,YM_TACKLE,true);
    assert(g.hp==before && g.foe_status==YStatusFrozen);
    yellow_heal_pikachu(&g);
    assert(g.pika_status==YStatusNone);
    puts("R12C2_SLEEP_FREEZE_TURN_GATE_AND_PP_PASS");
}
static void toxic_ticks(void) {
    assert(y_status_is_toxic(YStatusToxic1));
    assert(y_toxic_after_tick(YStatusToxic1)==YStatusToxic1+1);
    assert(y_toxic_after_tick(YStatusToxic15)==YStatusToxic15);
    assert(y_residual_damage(160,YStatusToxic1)==10);
    assert(y_residual_damage(160,YStatusToxic3 /* 3rd tick */)==30);
    assert(y_residual_damage(160,YStatusToxic15)==150);
    assert(y_residual_damage(3,YStatusToxic5)==5);
    assert(y_residual_damage(160,YStatusBurned)==10);
    YellowGame g;ready(&g);
    g.foe_max_hp=160;g.foe_hp=150;g.foe_status=YStatusToxic1;
    for(unsigned n=1;n<=3;n++) {
        uint8_t hp=g.foe_hp;
        assert(apply_residual_after_action(&g,false));
        assert(g.foe_hp==hp-10u*n);
        assert(g.foe_status==YStatusToxic1+n);
    }
    ready(&g);
    g.foe_status=YStatusToxic1;g.foe_hp=1;
    assert(!apply_residual_after_action(&g,false));
    assert(g.screen==YScreenDialog);
    puts("R12C2_TOXIC_RAMP_AND_KO_PASS");
}
static void status_capture_and_reset(void) {
    assert(!y_gen1_pokeball_capture_status(45,150,150,50,0,YStatusNone));
    assert(y_gen1_pokeball_capture_status(45,150,150,50,0,YStatusFrozen));
    assert(y_gen1_pokeball_capture_status(45,150,150,50,0,YStatusSleep4));
    assert(y_gen1_pokeball_capture_status(45,150,150,50,0,YStatusToxic2));
    YellowGame g;ready(&g);
    g.balls=5;g.foe_status=YStatusToxic3;g.foe_hp=1;
    yellow_input(&g,YOk);
    yellow_input(&g,YDown);yellow_input(&g,YDown);yellow_input(&g,YOk);
    assert(g.caught_count==1 && g.caught[0].status==YStatusPoisoned);
    assert(g.screen==YScreenDialog);
    while(g.screen==YScreenDialog)yellow_input(&g,YOk);
    g.pika_status=YStatusToxic6;
    yellow_begin_battle(&g,YK_PIDGEY,3);
    g.foe_hp=1;
    finalize_battle(&g,true);
    assert(g.pika_status==YStatusPoisoned);
    puts("R12C2_CATCH_BONUS_AND_TOXIC_NORMALIZATION_PASS");
}
static void save_migration_and_reject(void) {
    YellowGame g;ready(&g);
    g.screen=YScreenWorld;g.foe_hp=0;
    g.pika_status=YStatusSleep3;
    YellowSave latest;yellow_save_create(&g,&latest);
    assert(latest.version==13);
    YellowGame copy;assert(yellow_save_load(&copy,&latest));
    assert(copy.pika_status==YStatusSleep3);
    g.pika_status=YStatusFrozen;
    yellow_save_create(&g,&latest);
    assert(yellow_save_load(&copy,&latest));
    assert(copy.pika_status==YStatusFrozen);
    g.pika_status=YStatusToxic4;
    yellow_save_create(&g,&latest);
    assert(yellow_save_load(&copy,&latest));
    assert(copy.pika_status==YStatusToxic4);
    latest.pika_status=27;latest.checksum=checksum(&latest);
    assert(!yellow_save_load(&copy,&latest));
    g.pika_status=YStatusParalyzed;
    yellow_save_create(&g,&latest);
    YellowSave old=latest;old.version=12;old.checksum=checksum(&old);
    assert(!yellow_save_load(&copy,&old));
    assert(yellow_save_migrate_r12(&copy,&old));
    assert(copy.pika_status==YStatusParalyzed);
    assert(old.version==12);
    old.pika_status=YStatusFrozen;old.checksum=checksum(&old);
    assert(!yellow_save_migrate_r12(&copy,&old));
    old.pika_status=YStatusParalyzed;old.checksum^=1u;
    assert(!yellow_save_migrate_r12(&copy,&old));
    puts("R12C2_V13_SAVE_AND_VALIDATED_V12_MIGRATION_PASS");
}
int main(void) {
    sleep_math();
    action_suppression();
    toxic_ticks();
    status_capture_and_reset();
    save_migration_and_reject();
    puts("R12C2_ALL_TESTS_PASS");
    return 0;
}
