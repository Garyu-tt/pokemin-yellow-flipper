#include <assert.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"

static void matchups(void) {
    assert(YELLOW_TYPE_MATCHUP_COUNT==82);
    assert(YT_GHOST==8 && YT_ELECTRIC==23 && YT_PSYCHIC==24);
    assert(y_type_effectiveness(YT_NORMAL,YT_GHOST,YT_GHOST)==0);
    assert(y_type_effectiveness(YT_GHOST,YT_PSYCHIC,YT_PSYCHIC)==0);
    assert(y_type_effectiveness(YT_BUG,YT_POISON,YT_POISON)==200);
    assert(y_type_effectiveness(YT_POISON,YT_BUG,YT_BUG)==200);
    assert(y_type_effectiveness(YT_GROUND,YT_FLYING,YT_FLYING)==0);
    assert(y_type_effectiveness(YT_GROUND,YT_ELECTRIC,YT_ELECTRIC)==200);
    assert(y_type_effectiveness(YT_ELECTRIC,YT_WATER,YT_FLYING)==400);
    assert(y_type_effectiveness(YT_ELECTRIC,YT_ELECTRIC,YT_FLYING)==100);
    assert(y_type_effectiveness(YT_FIRE,YT_GRASS,YT_BUG)==400);
    assert(y_type_effectiveness(YT_NORMAL,YT_ROCK,YT_GHOST)==0);
    assert(y_type_effectiveness(YT_ICE,YT_DRAGON,YT_DRAGON)==200);
    assert(y_type_effectiveness(YT_DRAGON,YT_DRAGON,YT_DRAGON)==200);
    assert(y_type_effectiveness(YT_NORMAL,YT_NORMAL,YT_NORMAL)==100);
    puts("R12_ALL_ORIGINAL_GEN1_TYPE_MATCHUP_CASES_PASS");
}
static void damage(void) {
    YDamage regular=y_calc_damage_crit(YK_PIKACHU,10,YK_RATTATA,10,
        YM_THUNDERSHOCK,0,0,0,38,false);
    YDamage super=y_calc_damage_crit(YK_PIKACHU,10,YK_PIDGEY,10,
        YM_THUNDERSHOCK,0,0,0,38,false);
    assert(regular.hit&&super.hit);
    assert(super.damage>regular.damage);
    assert(y_type_effectiveness(YT_ELECTRIC,YT_FLYING,YT_FLYING)==200);
    YDamage gust_vs_pika=y_calc_damage_crit(YK_PIDGEY,10,YK_PIKACHU,10,
        YM_GUST,0,0,0,38,false);
    YDamage gust_vs_rattata=y_calc_damage_crit(YK_PIDGEY,10,YK_RATTATA,10,
        YM_GUST,0,0,0,38,false);
    assert(gust_vs_pika.hit&&gust_vs_rattata.hit);
    assert(gust_vs_pika.damage>0&&gust_vs_rattata.damage>0);
    puts("R12_INTEGRATED_TYPE_DAMAGE_PASS");
}
int main(void) {
    matchups();
    damage();
    puts("R12_TYPE_CHART_ALL_PASS");
    return 0;
}
