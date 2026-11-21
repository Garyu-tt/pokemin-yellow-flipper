#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"

static void setup(YellowGame* g) {
    yellow_init(g);
    g->screen=YScreenWorld;
    g->map=YMapRoute1; g->x=10;g->y=28;
    g->story=YStoryPikachuFollows;
    g->has_pikachu=1;g->balls=5;g->follower_valid=0;
}
static void catch_mon(YellowGame* g,uint8_t species,uint8_t level) {
    yellow_begin_battle(g,species,level);
    assert(g->screen==YScreenRivalBattle);
    g->foe_hp=1; /* weakened target guarantees this Gen1 capture roll */
    yellow_input(g,YOk);
    assert(g->screen==YScreenFightMenu);
    uint8_t moves=y_player_move_count(g->pika_level);
    for(uint8_t i=0;i<moves;i++)yellow_input(g,YDown);
    assert(g->selected==moves);
    uint8_t before=g->balls;
    yellow_input(g,YOk);
    assert(g->balls==before-1u);
    assert(g->screen==YScreenDialog);
    assert(g->caught[g->caught_count-1u].species==species);
    assert(g->caught[g->caught_count-1u].level==level);
    while(g->screen==YScreenDialog)yellow_input(g,YOk);
    assert(g->screen==YScreenWorld);
}
static void check_capture_math(void) {
    assert(y_species_catch_rate(YK_PIDGEY)==255);
    assert(y_species_catch_rate(YK_RATTATA)==255);
    assert(y_species_catch_rate(YK_EEVEE)==45);
    assert(!y_gen1_pokeball_capture(45,150,150,46,0));
    assert(!y_gen1_pokeball_capture(45,150,150,45,255));
    assert(y_gen1_pokeball_capture(45,150,150,45,0));
    assert(y_gen1_pokeball_capture(255,15,5,255,255));
    assert(!y_gen1_pokeball_capture(0,15,5,0,0));
    assert(!y_gen1_pokeball_capture(255,15,0,0,0));
    puts("R06_GEN1_CAPTURE_BOUNDS_PASS");
}
static void check_ball_consumption_and_party(void) {
    YellowGame g;setup(&g);
    assert(yellow_party_size(&g)==1);
    catch_mon(&g,YK_PIDGEY,3);
    assert(yellow_party_size(&g)==2 && g.balls==4);
    assert(g.caught[0].pp[0]==35);
    catch_mon(&g,YK_RATTATA,4);
    assert(g.caught[1].pp[0]==35&&g.caught[1].pp[1]==30);
    catch_mon(&g,YK_PIDGEY,5);
    assert(g.caught[2].pp[1]==15); /* Sand Attack at level 5 */
    catch_mon(&g,YK_RATTATA,7);
    assert(g.caught[3].pp[2]==30); /* Quick Attack at level 7 */
    catch_mon(&g,YK_PIDGEY,7);
    assert(yellow_party_size(&g)==6);
    assert(g.caught_count==YELLOW_CAPTURE_SLOTS);
    assert(g.balls==0);
    YellowSave save;
    yellow_save_create(&g,&save);
    assert(save.version==13);
    YellowGame other;
    assert(yellow_save_load(&other,&save));
    assert(yellow_party_size(&other)==6 && other.balls==0);
    assert(other.caught[4].level==7 && other.caught[4].species==YK_PIDGEY);
    assert(other.caught[1].pp[1]==30);
    assert(other.caught[2].pp[1]==15);
    assert(other.caught[3].pp[2]==30);
    save.caught[0].species=YK_EEVEE;
    save.checksum=checksum(&save);
    assert(!yellow_save_load(&other,&save));
    puts("R06_SIX_PARTY_CATCH_SAVE_VALIDATION_PASS");
    yellow_begin_battle(&g,YK_PIDGEY,3);
    yellow_input(&g,YOk);
    for(uint8_t i=0;i<y_player_move_count(g.pika_level);i++)yellow_input(&g,YDown);
    assert(g.selected==2);
    yellow_input(&g,YOk);
    assert(g.screen==YScreenFightMenu && g.caught_count==5 && g.balls==0);
    g.balls=2; /* full party catches into PC, not beyond party array */
    g.foe_hp=1;
    yellow_input(&g,YOk);
    assert(g.screen==YScreenDialog && g.caught_count==5 && g.balls==1);
    assert(g.pc.count==1 && g.pc.slots[0].species==YK_PIDGEY);
    puts("R20_PARTY_FULL_AUTO_PC_PASS");
    g.screen=YScreenWorld;
    yellow_input(&g,YBack);
    assert(g.screen==YScreenMenu);
    yellow_input(&g,YOk); /* party */
    assert(g.screen==YScreenParty);
    for(uint8_t i=0;i<5;i++)yellow_input(&g,YDown);
    assert(g.selected==5);
    yellow_input(&g,YDown);
    assert(g.selected==5);
    yellow_input(&g,YBack);
    assert(g.screen==YScreenMenu);
    yellow_input(&g,YDown);yellow_input(&g,YOk);
    assert(g.screen==YScreenBag && g.balls==1);
    puts("R06_PARTY_SCROLL_AND_BAG_NAVIGATION_PASS");
}
static void check_trainer_ball_block_and_heal(void) {
    YellowGame g;setup(&g);
    yellow_begin_battle(&g,YK_EEVEE,5);
    yellow_input(&g,YOk);
    for(uint8_t i=0;i<y_player_move_count(g.pika_level);i++)yellow_input(&g,YDown);
    uint8_t previous=g.balls;
    yellow_input(&g,YOk);
    assert(g.screen==YScreenFightMenu && g.balls==previous);
    puts("R06_TRAINER_CAPTURE_BLOCK_PASS");
    g.screen=YScreenWorld;
    catch_mon(&g,YK_RATTATA,3);
    g.caught[0].hp=1;
    g.caught[0].pp[0]=0;
    yellow_heal_pikachu(&g);
    assert(g.caught[0].hp==y_calc_stats(YK_RATTATA,3).hp);
    assert(g.caught[0].pp[0]==35);
    puts("R06_CAUGHT_MON_HEAL_PASS");
}
int main(void) {
    check_capture_math();
    check_ball_consumption_and_party();
    check_trainer_ball_block_and_heal();
    puts("R06_ALL_TESTS_PASS");
    return 0;
}
