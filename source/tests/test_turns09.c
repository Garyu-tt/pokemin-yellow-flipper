#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"

static void prep(YellowGame* g) {
    yellow_init(g);
    g->screen=YScreenWorld;g->map=YMapRoute1;g->x=10;g->y=28;
    g->story=YStoryPikachuFollows;g->has_pikachu=1;
    g->hp=yellow_player_max_hp(g);
    g->follower_valid=0;
}
static void priority_speed_crit_math(void) {
    assert(y_move_priority(YM_QUICK_ATTACK)==1);
    assert(y_move_priority(YM_TACKLE)==0);
    assert(y_player_moves_first(YM_QUICK_ATTACK,YM_TACKLE,3,100,255));
    assert(!y_player_moves_first(YM_TACKLE,YM_QUICK_ATTACK,100,3,0));
    assert(y_player_moves_first(YM_QUICK_ATTACK,YM_QUICK_ATTACK,16,10,255));
    assert(y_player_moves_first(YM_GUST,YM_TACKLE,20,10,255));
    assert(!y_player_moves_first(YM_GUST,YM_TACKLE,10,20,0));
    assert(y_player_moves_first(YM_GUST,YM_TACKLE,12,12,127));
    assert(!y_player_moves_first(YM_GUST,YM_TACKLE,12,12,128));
    assert(y_effective_speed(YK_PIKACHU,5,YStatusParalyzed)==3);
    assert(y_effective_speed(YK_PIKACHU,5,YStatusNone)==14);
    assert(y_critical_threshold(YK_PIKACHU,YM_THUNDERSHOCK)==45);
    assert(y_critical_threshold(YK_RATTATA,YM_QUICK_ATTACK)==36);
    assert(y_critical_threshold(YK_EEVEE,YM_TACKLE)==27);
    assert(y_critical_threshold(YK_PIKACHU,YM_GROWL)==0);
    int crits=0;
    for(int r=0;r<256;r++)crits+=y_roll_critical(YK_PIKACHU,YM_THUNDERSHOCK,(uint8_t)r);
    assert(crits==45);
    assert(!y_roll_critical(YK_PIKACHU,YM_GROWL,0));
    YDamage normal=y_calc_damage_crit(YK_RATTATA,10,YK_PIDGEY,8,YM_TACKLE,
                                       0,0,0,38,false);
    YDamage crit=y_calc_damage_crit(YK_RATTATA,10,YK_PIDGEY,8,YM_TACKLE,
                                     0,0,0,38,true);
    assert(crit.damage>normal.damage);
    YDamage crit_penalty=y_calc_damage_crit(YK_RATTATA,10,YK_PIDGEY,8,YM_TACKLE,
                                            -6,+6,0,38,true);
    assert(crit_penalty.damage==crit.damage);
    YDamage normal_penalty=y_calc_damage_crit(YK_RATTATA,10,YK_PIDGEY,8,YM_TACKLE,
                                              -6,+6,0,38,false);
    assert(normal_penalty.damage<normal.damage);
    puts("R09_QUICK_SPEED_TIE_CRITICAL_TABLE_PASS");
}
static void player_first_ko_prevents_counterattack(void) {
    YellowGame g;prep(&g);
    g.pika_level=6;g.experience=y_exp_threshold(YK_PIKACHU,6);
    g.hp=yellow_player_max_hp(&g);
    yellow_begin_battle(&g,YK_EEVEE,5);
    g.foe_hp=1;
    uint8_t hp=g.hp;
    yellow_input(&g,YOk);
    assert(g.screen==YScreenFightMenu);
    yellow_input(&g,YOk);
    assert(g.screen==YScreenDialog);
    assert(g.last_player_first==1u);
    assert(g.hp==hp);
    assert(g.pika_pp[0]==29);
    puts("R09_PLAYER_FIRST_KO_NO_ENEMY_ACTION_PASS");
}
static void quick_enemy_interrupts_pending_attack(void) {
    YellowGame g;prep(&g);
    YCapturedMon* c=&g.caught[0];memset(c,0,sizeof(*c));
    c->species=YK_PIDGEY;c->level=5;
    c->experience=y_exp_threshold(YK_PIDGEY,5);
    c->hp=y_calc_stats(YK_PIDGEY,5).hp;
    y_build_moveset(c->species,c->level,c->moves);
    c->pp[0]=y_max_pp(YM_GUST);
    g.caught_count=1;
    bool found=false;
    for(uint32_t seed=1;seed<5000&&!found;seed++) {
        g.hp=1;g.pika_pp[0]=30;
        g.active_party=0;
        yellow_begin_battle(&g,YK_RATTATA,7);
        g.rng=seed;
        yellow_input(&g,YOk);
        assert(g.screen==YScreenFightMenu);
        yellow_input(&g,YOk);
        if(g.last_enemy_move==YM_QUICK_ATTACK &&
           g.last_player_first==0 && g.screen==YScreenParty) {
            assert(g.hp==0);
            assert(g.force_switch);
            assert(g.pika_pp[0]==30); /* enemy KO happened before PP consumption */
            found=true;
        } else if(g.screen==YScreenDialog) {
            /* Whiteout shouldn't happen with living Pidgey, so fail loudly. */
            assert(0);
        }
    }
    assert(found);
    puts("R09_ENEMY_QUICK_KO_CANCELS_PENDING_MOVE_PASS");
}
static void player_quick_outspeeds_higher_speed(void) {
    YellowGame g;prep(&g);
    YCapturedMon* c=&g.caught[0];memset(c,0,sizeof(*c));
    c->species=YK_RATTATA;c->level=7;
    c->experience=y_exp_threshold(YK_RATTATA,7);
    c->hp=y_calc_stats(YK_RATTATA,7).hp;
    y_build_moveset(c->species,c->level,c->moves);
    for(uint8_t i=0;i<4;i++)c->pp[i]=y_max_pp(c->moves[i]);
    g.caught_count=1;g.active_party=1;
    yellow_begin_battle(&g,YK_PIDGEY,30);
    assert(y_effective_speed(YK_PIDGEY,30,0)>
           y_effective_speed(YK_RATTATA,7,0));
    yellow_input(&g,YOk);
    assert(g.screen==YScreenFightMenu);
    yellow_input(&g,YDown);
    yellow_input(&g,YDown);
    assert(g.selected==2);
    yellow_input(&g,YOk);
    assert(g.last_player_first==1 && g.last_enemy_move!=YM_QUICK_ATTACK);
    assert(g.caught[0].pp[2]==29);
    puts("R09_QUICK_ATTACK_OVERRIDES_SPEED_PASS");
}
static void non_critical_defensive_test(void) {
    YellowGame g;prep(&g);
    yellow_begin_battle(&g,YK_PIDGEY,7);
    uint8_t initial=g.foe_hp;
    bool seen=false;
    for(uint32_t seed=1;seed<5000&&!seen;seed++) {
        g.rng=seed;g.screen=YScreenRivalBattle;
        g.foe_hp=initial;g.hp=yellow_player_max_hp(&g);
        g.pika_pp[0]=30;
        yellow_input(&g,YOk);yellow_input(&g,YOk);
        if(g.last_critical && g.last_player_first &&
           g.foe_hp<initial)seen=true;
    }
    assert(seen);
    puts("R09_CRITICAL_INTEGRATION_PASS");
}
int main(void) {
    priority_speed_crit_math();
    player_first_ko_prevents_counterattack();
    quick_enemy_interrupts_pending_attack();
    player_quick_outspeeds_higher_speed();
    non_critical_defensive_test();
    puts("R09_ALL_TESTS_PASS");
    return 0;
}
