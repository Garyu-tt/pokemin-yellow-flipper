#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"

static void close_dialog(YellowGame* g) {
    assert(g->screen==YScreenDialog);
    while(g->screen==YScreenDialog)yellow_input(g,YOk);
}
static void unit_stats(void) {
    YBattleStats p=y_calc_stats(YK_PIKACHU,5);
    YBattleStats e=y_calc_stats(YK_EEVEE,5);
    assert(p.hp==19 && p.attack==11 && p.defense==8 &&
           p.speed==14 && p.special==10);
    assert(e.hp==21);
    assert(y_species_info(YK_PIKACHU)->exp_yield==82);
    assert(y_species_info(YK_PIDGEY)->growth==1);
    assert(y_exp_threshold(YK_PIKACHU,5)==125);
    assert(y_exp_threshold(YK_PIKACHU,6)==216);
    assert(y_exp_threshold(YK_PIDGEY,5)==135);
    assert(y_max_pp(YM_THUNDERSHOCK)==30);
    assert(y_max_pp(YM_GROWL)==40);
    assert(y_max_pp(YM_TAIL_WHIP)==30);
    assert(y_player_move_count(5)==2 && y_player_move_count(6)==3);
    assert(y_player_move(6,2)==YM_TAIL_WHIP);
    assert(y_move_info(YM_GUST)->type==YT_NORMAL); /* Gen1 */
    assert(y_exp_award(YK_PIDGEY,7,false)==55);
    puts("R05_ORIGINAL_STATS_MOVES_GROWTH_PASS");
}
static void unit_damage(void) {
    YDamage normal=y_calc_damage(YK_PIKACHU,5,YK_RATTATA,5,YM_THUNDERSHOCK,0,0,0,38);
    YDamage flying=y_calc_damage(YK_PIKACHU,5,YK_PIDGEY,5,YM_THUNDERSHOCK,0,0,0,38);
    assert(normal.hit && flying.hit && normal.damage>0 && flying.damage>normal.damage);
    YDamage miss=y_calc_damage(YK_EEVEE,5,YK_PIKACHU,5,YM_TACKLE,0,0,255,15);
    assert(!miss.hit && !miss.damage);
    YDamage normal_hit=y_calc_damage(YK_EEVEE,5,YK_PIKACHU,5,YM_TACKLE,0,0,0,15);
    YDamage weakened=y_calc_damage(YK_EEVEE,5,YK_PIKACHU,5,YM_TACKLE,-6,0,0,15);
    assert(normal_hit.hit && weakened.hit && weakened.damage<=normal_hit.damage);
    puts("R05_STAB_TYPE_ACCURACY_STAGES_PASS");
}
static void setup_player(YellowGame* g) {
    yellow_init(g);
    g->story=YStoryPikachuFollows;
    g->has_pikachu=1;
    g->map=YMapRoute1;g->x=10;g->y=28;
    g->screen=YScreenWorld;
    g->facing=YUp;
    g->follower_valid=0;
}
static void knock_out(YellowGame* g,uint8_t kind,uint8_t level) {
    yellow_begin_battle(g,kind,level);
    assert(g->screen==YScreenRivalBattle);
    for(int n=0;n<30 && g->screen==YScreenRivalBattle;n++) {
        yellow_input(g,YOk);
        assert(g->screen==YScreenFightMenu);
        yellow_input(g,YOk);
    }
    assert(g->screen==YScreenDialog);
    assert(g->last_exp_award==y_exp_award(kind,level,kind==YK_EEVEE));
    close_dialog(g);
    assert(g->story==YStoryPikachuFollows && g->screen==YScreenWorld);
}
static void exp_level_pp_save(void) {
    YellowGame g;setup_player(&g);
    assert(g.experience==125 && g.pika_level==5);
    uint8_t old_pp=g.pika_pp[0];
    knock_out(&g,YK_PIDGEY,7);
    assert(g.experience==180 && g.pika_level==5);
    assert(g.pika_pp[0]<old_pp);
    yellow_heal_pikachu(&g);
    assert(g.pika_pp[0]==30);
    knock_out(&g,YK_PIDGEY,7);
    assert(g.experience==235 && g.pika_level==6);
    assert(g.hp<=yellow_player_max_hp(&g) && yellow_player_max_hp(&g)==21);
    assert(g.pika_pp[2]==30);
    yellow_heal_pikachu(&g);
    yellow_begin_battle(&g,YK_RATTATA,2);
    assert(g.foe_max_hp==y_calc_stats(YK_RATTATA,2).hp);
    yellow_input(&g,YOk);
    yellow_input(&g,YDown);yellow_input(&g,YDown);
    assert(g.selected==2);
    yellow_input(&g,YOk);
    assert(g.pika_pp[2]==29);
    assert(g.foe_defense_stage==-1);
    /* Cannot save midbattle via UI; state snapshot only in overworld. */
    g.screen=YScreenWorld;
    YellowSave save;yellow_save_create(&g,&save);
    YellowGame copy;
    assert(yellow_save_load(&copy,&save));
    assert(copy.pika_level==6 && copy.experience==235 && copy.pika_pp[2]==29);
    save.pika_pp[0]=254;save.checksum=checksum(&save);
    assert(!yellow_save_load(&copy,&save));
    puts("R05_EXP_LEVEL6_TAILWHIP_SAVE_PP_PASS");
}
static void empty_pp_and_center(void) {
    YellowGame g;setup_player(&g);
    yellow_begin_battle(&g,YK_RATTATA,2);
    g.pika_pp[0]=0;
    uint8_t hp=g.foe_hp;
    yellow_input(&g,YOk);
    assert(g.selected==0 && g.screen==YScreenFightMenu);
    yellow_input(&g,YOk);
    assert(g.foe_hp==hp && g.screen==YScreenFightMenu);
    yellow_heal_pikachu(&g);
    assert(g.pika_pp[0]==30 && g.pika_pp[1]==40);
    assert(g.hp==19);
    puts("R05_ZERO_PP_REJECT_HEAL_PASS");
}
int main(void) {
    unit_stats();
    unit_damage();
    exp_level_pp_save();
    empty_pp_and_center();
    puts("R05_ALL_TESTS_PASS");
    return 0;
}
