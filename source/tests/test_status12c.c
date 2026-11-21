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
static void verify_damage_formula(void) {
    assert(y_residual_damage(1,YStatusPoisoned)==1);
    assert(y_residual_damage(15,YStatusBurned)==1);
    assert(y_residual_damage(16,YStatusBurned)==1);
    assert(y_residual_damage(95,YStatusPoisoned)==5);
    assert(y_residual_damage(160,YStatusBurned)==10);
    assert(y_residual_damage(160,YStatusNone)==0);
    assert(y_residual_damage(160,YStatusParalyzed)==0);
    YDamage ordinary=y_calc_damage_status_crit(YK_RATTATA,35,YK_PIDGEY,20,
        YM_TACKLE,0,0,0,38,false,YStatusNone);
    YDamage burned=y_calc_damage_status_crit(YK_RATTATA,35,YK_PIDGEY,20,
        YM_TACKLE,0,0,0,38,false,YStatusBurned);
    assert(burned.hit && ordinary.hit && burned.damage<ordinary.damage);
    YDamage regular_special=y_calc_damage_status_crit(YK_PIKACHU,35,YK_PIDGEY,20,
        YM_THUNDERSHOCK,0,0,0,38,false,YStatusNone);
    YDamage burn_special=y_calc_damage_status_crit(YK_PIKACHU,35,YK_PIDGEY,20,
        YM_THUNDERSHOCK,0,0,0,38,false,YStatusBurned);
    assert(burn_special.damage==regular_special.damage);
    puts("R12C_GEN1_BURN_POISON_HP_AND_ATTACK_PASS");
}
static void residual_faint_and_victory(void) {
    YellowGame g;ready(&g);
    g.foe_status=YStatusPoisoned;
    g.foe_hp=1;
    g.participants_mask=1;
    assert(!apply_residual_after_action(&g,false));
    assert(g.screen==YScreenDialog);
    assert(g.last_exp_award==y_exp_award(YK_RATTATA,3,false));

    ready(&g);
    g.hp=1;g.pika_status=YStatusBurned;
    assert(!apply_residual_after_action(&g,true));
    assert(g.screen==YScreenDialog && g.hp==yellow_player_max_hp(&g));
    assert(g.map==YMapRedsHouse);

    ready(&g);
    YCapturedMon* mon=&g.caught[0];memset(mon,0,sizeof(*mon));
    mon->species=YK_PIDGEY;mon->level=5;
    mon->experience=y_exp_threshold(YK_PIDGEY,5);
    mon->hp=y_calc_stats(mon->species,mon->level).hp;
    y_build_moveset(mon->species,mon->level,mon->moves);
    for(unsigned i=0;i<4;i++)mon->pp[i]=y_max_pp(mon->moves[i]);
    g.caught_count=1;g.hp=1;g.pika_status=YStatusPoisoned;
    assert(!apply_residual_after_action(&g,true));
    assert(g.hp==0 && g.screen==YScreenParty && g.force_switch);
    assert(g.selected==1);
    puts("R12C_POISON_OR_BURN_FAINT_WHITEOUT_SWITCH_PASS");
}
static void captured_status_save_and_heal(void) {
    YellowGame g;ready(&g);
    g.pika_status=YStatusBurned;
    g.foe_status=YStatusPoisoned;
    g.foe_hp=1;g.balls=3;
    g.screen=YScreenRivalBattle;
    yellow_input(&g,YOk);assert(g.screen==YScreenFightMenu);
    yellow_input(&g,YDown);yellow_input(&g,YDown);yellow_input(&g,YOk);
    assert(g.screen==YScreenDialog && g.caught_count==1);
    assert(g.caught[0].status==YStatusPoisoned);
    while(g.screen==YScreenDialog)yellow_input(&g,YOk);
    YellowSave s;yellow_save_create(&g,&s);
    assert(s.version==13);
    YellowGame copy;assert(yellow_save_load(&copy,&s));
    assert(copy.pika_status==YStatusBurned);
    assert(copy.caught[0].status==YStatusPoisoned);
    s.caught[0].status=27;s.checksum=checksum(&s);
    assert(!yellow_save_load(&copy,&s));
    yellow_heal_pikachu(&g);
    assert(g.pika_status==YStatusNone && g.caught[0].status==YStatusNone);
    puts("R12C_CAPTURE_STATUS_PERSISTENCE_VALIDATION_AND_CURE_PASS");
}
static void end_of_each_own_action(void) {
    YellowGame g;ready(&g);
    g.pika_status=YStatusBurned;
    g.foe_status=YStatusPoisoned;
    g.rng=1;
    uint8_t old_hp=g.hp;
    uint8_t old_foe=g.foe_hp;
    /* Non-damaging move: both active Pokémon act, then take 1 tick each. */
    resolve_attack_turn(&g,YM_GROWL);
    assert(g.screen==YScreenRivalBattle);
    assert(g.hp<old_hp);
    assert(g.foe_hp<old_foe);
    puts("R12C_OWN_ACTION_RESIDUAL_TURN_PASS");
}
static void status_catch_bonus(void) {
    assert(!y_gen1_pokeball_capture_status(45,150,150,50,0,YStatusNone));
    assert(y_gen1_pokeball_capture_status(45,150,150,50,0,YStatusBurned));
    assert(y_gen1_pokeball_capture_status(45,150,150,50,0,YStatusPoisoned));
    assert(y_gen1_pokeball_capture_status(45,150,150,50,0,YStatusParalyzed));
    puts("R12C_GEN1_STATUS_CATCH_BONUS_PASS");
}
int main(void) {
    verify_damage_formula();
    residual_faint_and_victory();
    captured_status_save_and_heal();
    end_of_each_own_action();
    status_catch_bonus();
    puts("R12C_ALL_TESTS_PASS");
    return 0;
}
