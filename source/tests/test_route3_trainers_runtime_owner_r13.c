#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"

static void ready(YellowGame* g,int x,int y) {
    yellow_init(g);
    g->running=true;g->screen=YScreenWorld;
    g->story=YStoryPikachuFollows;g->has_pikachu=1;
    g->map=YMapRoute3;g->x=x;g->y=y;
    g->facing=YRight;g->follower_valid=0;
    g->pika_level=60;
    g->experience=y_exp_threshold(YK_PIKACHU,60);
    g->hp=y_calc_stats(YK_PIKACHU,60).hp;
    g->pika_moves[0]=YM_QUICK_ATTACK;
    g->pika_pp[0]=30;
    g->balls=7;
}
static void hit(YellowGame* g) {
    assert(g->screen==YScreenRivalBattle);
    uint8_t old_slot=g->route3_party_index;
    for(unsigned attempt=0;attempt<12;attempt++) {
        g->foe_hp=1;
        yellow_input(g,YOk);
        assert(g->screen==YScreenFightMenu);
        g->selected=0;
        yellow_input(g,YOk);
        if(g->screen!=YScreenRivalBattle ||
           g->route3_party_index!=old_slot || g->foe_hp==0)return;
    }
    assert(0);
}
static void canonical_one(unsigned index,int playerx,int playery) {
    YellowGame g;ready(&g,playerx,playery);
    const YRoute3Trainer* info=route3_trainer(index);
    assert(info && info->count==2 && info->original_index==2u+2u*index);
    assert(route3_trainer_at(info->x,info->y)==(int)index);
    assert(yellow_world_scene_asset(g.map));
    g.facing=index==2?YRight:YRight;
    yellow_input(&g,YOk);
    assert(g.screen==YScreenDialog && g.pending==YPendingBeginRoute3TrainerBattle);
    assert(g.route3_pending_trainer==index+1);
    yellow_input(&g,YOk);
    assert(g.route3_pending_trainer==0 && g.route3_active_trainer==index+1);
    assert(g.screen==YScreenRivalBattle &&
           g.foe_kind==info->species[0] && g.foe_level==info->levels[0]);
    /* These are source trainer parties. They cannot be caught or escaped. */
    uint8_t oldballs=g.balls;
    yellow_input(&g,YOk);
    assert(g.screen==YScreenFightMenu);
    g.selected=yellow_active_move_count(&g);
    yellow_input(&g,YOk);
    assert(g.balls==oldballs && g.screen==YScreenFightMenu);
    g.selected=(uint8_t)(yellow_active_move_count(&g)+1);
    yellow_input(&g,YOk);
    assert(g.screen==YScreenRivalBattle);
    hit(&g);
    assert(g.route3_active_trainer==index+1);
    assert(g.route3_party_index==1);
    assert(g.screen==YScreenRivalBattle &&
           g.foe_kind==info->species[1] && g.foe_level==info->levels[1]);
    assert(!(g.route3_defeated_mask & (1u<<info->original_index)));
    hit(&g);
    assert(g.screen==YScreenDialog && g.pending==YPendingEndWildBattle);
    assert(g.route3_active_trainer==0 && g.route3_party_index==0);
    assert(g.route3_defeated_mask==(1u<<info->original_index));
    yellow_input(&g,YOk);
    assert(g.screen==YScreenWorld && g.map==YMapRoute3);
    yellow_input(&g,YOk);
    assert(g.screen==YScreenDialog && g.pending==YPendingNone);
    yellow_input(&g,YOk);
    assert(g.screen==YScreenWorld && g.route3_defeated_mask==(1u<<info->original_index));
    YellowSave parent;
    yellow_save_create(&g,&parent);
    assert(parent.version==13 && sizeof(parent)==132);
    assert(parent.map==YMapRoute2);
    YellowRoute3TrainerSave t;
    assert(yellow_route3_trainer_save(g.route3_defeated_mask,parent.checksum,&t));
    assert(sizeof(t)==16);
    assert(t.magic==YELLOW_ROUTE3_TRAINER_MAGIC);
    uint8_t loaded=0;
    assert(yellow_route3_trainer_restore(&loaded,parent.checksum,&t));
    assert(loaded==g.route3_defeated_mask);
    t.checksum^=1u;
    assert(!yellow_route3_trainer_restore(&loaded,parent.checksum,&t));
    assert(loaded==g.route3_defeated_mask);
}
static void sightline_and_negative(void) {
    YellowGame g;ready(&g,13,9);
    assert(route3_trainer_sight(&g)==-1);
    yellow_input(&g,YRight);
    assert(g.x==14 && g.screen==YScreenDialog);
    assert(g.route3_pending_trainer==1);
    yellow_input(&g,YOk);
    assert(g.screen==YScreenRivalBattle &&
           g.foe_kind==YK_PIDGEY && g.foe_level==9);
    ready(&g,13,9);
    g.route3_defeated_mask=1u<<2;
    yellow_input(&g,YRight);
    assert(g.screen==YScreenWorld && g.route3_pending_trainer==0);
    ready(&g,13,9);
    g.story=YStoryBeforeOak;
    yellow_input(&g,YRight);
    assert(g.screen==YScreenWorld && !g.route3_pending_trainer);
    ready(&g,15,9);
    g.facing=YRight;
    g.route3_defeated_mask=1u<<2;
    yellow_input(&g,YOk);
    assert(g.screen==YScreenDialog && g.pending==YPendingNone);
    ready(&g,15,9);
    g.hp=0;
    yellow_input(&g,YOk);
    assert(g.screen==YScreenWorld && g.route3_pending_trainer==0);
    uint8_t mask=0;
    assert(!yellow_route3_trainer_save(1u,42,&(YellowRoute3TrainerSave){0}));
    assert(yellow_route3_trainer_save(0,42,&(YellowRoute3TrainerSave){0}));
    YellowRoute3TrainerSave saved;
    assert(yellow_route3_trainer_save(YELLOW_ROUTE3_DEFEATED_BITS,42,&saved));
    assert(!yellow_route3_trainer_restore(&mask,43,&saved));
    assert(mask==0);
    saved.defeated_mask=1;
    saved.checksum=route3_trainer_checksum(&saved);
    assert(!yellow_route3_trainer_restore(&mask,42,&saved));
    assert(mask==0);
}
int main(void) {
    canonical_one(0,15,9);
    canonical_one(1,22,4);
    canonical_one(2,23,6);
    sightline_and_negative();
    puts("ROUTE3_ORIGINAL_THREE_TRAINERS_SIGHT_TWO_MON_BATTLE_INPUT_SIDECAR_V13_PASS");
    return 0;
}
