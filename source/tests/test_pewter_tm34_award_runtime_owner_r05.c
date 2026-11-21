#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_pewter_progress.c"
#include "../yellow_owner_pewter_source.c"
#include "../yellow_core.c"

static void ready(YellowGame* g) {
 yellow_init(g);
 g->running=true;g->screen=YScreenWorld;g->story=YStoryPikachuFollows;
 g->has_pikachu=1;g->map=YMapPewterGym;g->x=4;g->y=2;
 g->facing=YUp;g->follower_valid=0;g->pika_level=60;g->experience=y_exp_threshold(YK_PIKACHU,60);
 g->hp=yellow_player_max_hp(g);
 g->pika_moves[0]=YM_QUICK_ATTACK;g->pika_pp[0]=30;
}
static void hit(YellowGame* g) {
 assert(g->screen==YScreenRivalBattle);
 g->foe_hp=1;
 yellow_input(g,YOk);
 assert(g->screen==YScreenFightMenu);
 g->selected=0;yellow_input(g,YOk);
}
static void earn_tm34_from_real_input(void) {
 YellowGame g;ready(&g);
 yellow_input(&g,YOk);
 assert(g.screen==YScreenDialog && g.pending==YPendingBeginPewterBrockBattle);
 assert(!(g.pewter_flags&YPewterAllFlags));
 yellow_input(&g,YOk);
 assert(g.screen==YScreenRivalBattle && g.foe_kind==YK_GEODUDE);
 hit(&g);
 assert(g.screen==YScreenRivalBattle && g.foe_kind==YK_ONIX);
 assert(!(g.pewter_flags&YPewterFlagTM34));
 hit(&g);
 assert(g.screen==YScreenDialog && g.pending==YPendingGrantPewterTM34);
 assert((g.pewter_flags&(YPewterFlagBrock|YPewterFlagBoulderBadge))==
        (YPewterFlagBrock|YPewterFlagBoulderBadge));
 assert(!(g.pewter_flags&YPewterFlagTM34));
 yellow_input(&g,YOk);
 assert(g.screen==YScreenWorld && g.pending==YPendingNone);
 assert(g.pewter_flags&YPewterFlagTM34);
 uint8_t flags=g.pewter_flags;
 yellow_input(&g,YOk);
 assert(g.screen==YScreenDialog && g.pending==YPendingNone);
 yellow_input(&g,YOk);
 assert(g.pewter_flags==flags && g.screen==YScreenWorld);
 assert(!y_pewter_claim_tm34(&g.pewter_flags,true));
 assert(g.pewter_flags==flags);
 YellowSave parent;YPewterProgressSave saved;YellowGame loaded;
 yellow_save_create(&g,&parent);
 assert(parent.version==13 && sizeof(parent)==132);
 assert(y_pewter_progress_save(g.pewter_flags,parent.checksum,&saved));
 assert(yellow_save_load(&loaded,&parent));
 assert(loaded.map==YMapRoute2);
 assert(y_pewter_progress_restore(&loaded.pewter_flags,parent.checksum,&saved));
 assert(loaded.pewter_flags==flags);
 YPewterProgressSave invalid=saved;invalid.checksum^=1;
 uint8_t preserved=loaded.pewter_flags;
 assert(!y_pewter_progress_restore(&loaded.pewter_flags,parent.checksum,&invalid));
 assert(loaded.pewter_flags==preserved);
 assert(!y_pewter_progress_restore(&loaded.pewter_flags,parent.checksum+1,&saved));
}
static void no_reward_without_brock(void) {
 YellowGame g;ready(&g);
 g.pending=YPendingGrantPewterTM34;
 g.screen=YScreenDialog;g.dialog_count=1;g.dialog_page=0;
 yellow_input(&g,YOk);
 assert(g.screen==YScreenWorld && g.pewter_flags==0);
 g.pewter_flags=YPewterFlagGymTrainer0;
 g.pending=YPendingGrantPewterTM34;
 g.screen=YScreenDialog;g.dialog_count=1;g.dialog_page=0;
 yellow_input(&g,YOk);
 assert(g.pewter_flags==YPewterFlagGymTrainer0 && g.screen==YScreenWorld);
 assert(!y_pewter_claim_tm34(&g.pewter_flags,false));
}
static void deferred_item_capacity_contract(void) {
 uint8_t flags=0;
 assert(y_pewter_record_brock_victory(&flags));
 assert(!y_pewter_claim_tm34(&flags,false));
 assert(!(flags&YPewterFlagTM34));
 assert(y_pewter_claim_tm34(&flags,true));
 assert(!y_pewter_claim_tm34(&flags,true));
 assert(y_pewter_flags_valid(flags));
}
int main(void) {
 earn_tm34_from_real_input();
 no_reward_without_brock();
 deferred_item_capacity_contract();
 puts("PEWTER_TM34_INPUT_AWARD_ONCE_BOULDER_BADGE_SIDECAR_V13_PASS");
 return 0;
}
