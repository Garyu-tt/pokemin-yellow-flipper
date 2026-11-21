#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_owner_pewter_source.c"
#include "../yellow_pewter_progress.c"
#include "../yellow_core.c"

static void gym_ready(YellowGame* g,int x,int y) {
    yellow_init(g);
    g->screen=YScreenWorld;g->running=true;
    g->story=YStoryPikachuFollows;g->has_pikachu=1;
    g->follower_valid=0;g->map=YMapPewterGym;
    g->x=(int8_t)x;g->y=(int8_t)y;g->facing=YUp;
    g->pika_level=60;g->hp=yellow_player_max_hp(g);
    g->pika_moves[0]=YM_QUICK_ATTACK;
    g->pika_pp[0]=30;
}
static void hit(YellowGame* g) {
    assert(g->screen==YScreenRivalBattle);
    /* Force a low foe HP only to shorten this deterministic runtime test.
       Actual victory still requires an input attack and finalization. */
    g->foe_hp=1;
    yellow_input(g,YOk);
    assert(g->screen==YScreenFightMenu);
    g->selected=0;
    yellow_input(g,YOk);
}
static void original_teams() {
    YPewterTrainerParty t={0};
    uint8_t kind=0,level=0;
    assert(y_pewter_trainer(0,&t));
    assert(t.count==2 && t.mon[0].dex==50 && t.mon[0].level==9);
    assert(t.mon[1].dex==27 && t.mon[1].level==9);
    pewter_party_mon(0,0,&kind,&level);assert(kind==YK_DIGLETT && level==9);
    pewter_party_mon(0,1,&kind,&level);assert(kind==YK_SANDSHREW && level==9);
    assert(y_pewter_trainer(1,&t));
    assert(t.count==2 && t.mon[0].dex==74 && t.mon[0].level==10);
    assert(t.mon[1].dex==95 && t.mon[1].level==12);
    pewter_party_mon(1,0,&kind,&level);assert(kind==YK_GEODUDE && level==10);
    pewter_party_mon(1,1,&kind,&level);assert(kind==YK_ONIX && level==12);
    assert(!y_pewter_trainer(2,&t));
}
static void junior_defeat_and_repeat() {
    YellowGame g;gym_ready(&g,3,7);
    yellow_input(&g,YOk);
    assert(g.screen==YScreenDialog && g.pewter_active_trainer==0);
    yellow_input(&g,YOk);
    assert(g.screen==YScreenRivalBattle && g.pewter_active_trainer==1 && g.foe_kind==YK_DIGLETT);
    hit(&g);
    assert(g.screen==YScreenRivalBattle && g.pewter_active_trainer==1 && g.pewter_party_index==1 && g.foe_kind==YK_SANDSHREW);
    assert(!(g.pewter_flags&YPewterFlagGymTrainer0));
    hit(&g);
    assert(g.screen==YScreenDialog && g.pewter_active_trainer==0);
    assert(g.pewter_flags&YPewterFlagGymTrainer0);
    yellow_input(&g,YOk);
    assert(g.screen==YScreenWorld);
    yellow_input(&g,YOk);
    assert(g.screen==YScreenDialog && g.pending==YPendingNone && g.pewter_active_trainer==0);
}
static void brock_win_and_save() {
    YellowGame g;gym_ready(&g,4,2);
    yellow_input(&g,YOk);
    assert(g.screen==YScreenDialog && !(g.pewter_flags&YPewterFlagBrock));
    yellow_input(&g,YOk);
    assert(g.screen==YScreenRivalBattle && g.pewter_active_trainer==2 && g.foe_kind==YK_GEODUDE);
    hit(&g);
    assert(g.screen==YScreenRivalBattle && g.foe_kind==YK_ONIX);
    assert(!(g.pewter_flags&(YPewterFlagBrock|YPewterFlagBoulderBadge)));
    hit(&g);
    assert(g.screen==YScreenDialog && g.pewter_active_trainer==0);
    assert((g.pewter_flags&(YPewterFlagBrock|YPewterFlagBoulderBadge))==(YPewterFlagBrock|YPewterFlagBoulderBadge));
    assert(!(g.pewter_flags&YPewterFlagTM34));
    yellow_input(&g,YOk);
    assert(g.screen==YScreenWorld);
    yellow_input(&g,YOk);
    assert(g.screen==YScreenDialog && g.pending==YPendingNone);
    YellowSave parent;
    YPewterProgressSave sidecar;
    yellow_save_create(&g,&parent);
    assert(parent.version==13 && parent.map==YMapRoute2);
    assert(y_pewter_progress_save(g.pewter_flags,parent.checksum,&sidecar));
    uint8_t restored=0;
    assert(y_pewter_progress_restore(&restored,parent.checksum,&sidecar));
    assert(restored==g.pewter_flags);
    assert(!y_pewter_progress_restore(&restored,parent.checksum+1,&sidecar));
}
static void loss_and_no_capture() {
    YellowGame g;gym_ready(&g,4,2);
    yellow_input(&g,YOk);
    yellow_input(&g,YOk);
    assert(g.pewter_active_trainer==2);
    g.balls=9;
    assert(g.screen==YScreenRivalBattle);
    yellow_input(&g,YOk);
    assert(g.screen==YScreenFightMenu);
    g.selected=(uint8_t)(yellow_active_move_count(&g)+0u);
    yellow_input(&g,YOk); /* bag capture blocked by trainer guard */
    assert(g.balls==9 && g.pewter_active_trainer==2);
    g.screen=YScreenRivalBattle;
    finalize_battle(&g,false);
    assert(g.pewter_active_trainer==0 && g.map==YMapRedsHouse);
    assert(!(g.pewter_flags&(YPewterFlagBrock|YPewterFlagBoulderBadge)));
}
int main(void) {
    original_teams();
    junior_defeat_and_repeat();
    brock_win_and_save();
    loss_and_no_capture();
    puts("PEWTER_R04_TRAINERS_SOURCE_TEAMS_INPUT_TWO_FOES_NO_DUPLICATE_SAVE_PASS");
    return 0;
}
