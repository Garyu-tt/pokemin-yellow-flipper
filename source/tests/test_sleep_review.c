/* Independent behavioural review for MK-7dfee0b5c0114c3c.
 * Target: SleepEffect / MoveHitTest in pinned pret/pokeyellow.
 * Include the local isolated source; never link accepted v064 in place.
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "yellow_assets.c"
#include "yellow_battle.c"
#include "yellow_core.c"

static void ready(YellowGame* g) {
    yellow_init(g);
    g->story=YStoryPikachuFollows;g->has_pikachu=1;
    g->map=YMapRoute1;g->x=10;g->y=28;
    g->screen=YScreenWorld;g->follower_valid=0;
    g->hp=yellow_player_max_hp(g);
    yellow_begin_battle(g,YK_RATTATA,3);
    g->selected=0;
}
static uint8_t predict(YellowGame* oracle, int player) {
    uint8_t roll=(uint8_t)next_rng(oracle);
    if(!y_move_hits(YM_SLEEP_POWDER,
            player?oracle->player_accuracy_stage:oracle->foe_accuracy_stage,
            player?oracle->foe_evasion_stage:oracle->player_evasion_stage,
            roll)) return YStatusNone;
    uint8_t count;
    unsigned limit=0;
    do { count=(uint8_t)next_rng(oracle)&7u; assert(++limit<100); } while(!count);
    return (uint8_t)(YStatusSleep1+count-1u);
}
int main(void) {
    unsigned hits=0,misses=0,counts[8]={0};
    for(uint32_t seed=1;seed<=8192;seed++) for(int player=0;player<=1;player++) {
        YellowGame g;ready(&g);g.rng=seed;
        YellowGame oracle=g;
        uint8_t expected=predict(&oracle,player);
        uint8_t old_hp=g.hp,old_foe_hp=g.foe_hp,old_pp=g.pika_pp[0];
        battle_action_pipeline(&g,player!=0,YM_SLEEP_POWDER,false);
        uint8_t actual=player?g.foe_status:g.pika_status;
        assert(actual==expected);
        assert(g.rng==oracle.rng);
        assert(g.hp==old_hp && g.foe_hp==old_foe_hp);
        assert(g.pika_pp[0]==(uint8_t)(old_pp-(player?1:0)));
        if(actual==YStatusNone)misses++;
        else { hits++;counts[actual-YStatusSleep1+1]++; }
    }
    assert(hits>0 && misses>0);
    for(unsigned i=1;i<=7;i++) assert(counts[i]>0);
    puts("SLEEP_DURATION_16384_BIDIRECTIONAL_PASS");

    const uint8_t occupied[]={YStatusSleep5,YStatusPoisoned,YStatusParalyzed,YStatusFrozen};
    for(unsigned i=0;i<sizeof(occupied);i++) for(int player=0;player<=1;player++) {
        YellowGame g;ready(&g);g.rng=123456;
        if(player)g.foe_status=occupied[i];else g.pika_status=occupied[i];
        uint32_t old_rng=g.rng;
        uint8_t pp=g.pika_pp[0],hp=g.hp,foe_hp=g.foe_hp;
        battle_action_pipeline(&g,player!=0,YM_SLEEP_POWDER,false);
        assert((player?g.foe_status:g.pika_status)==occupied[i]);
        assert(g.rng==old_rng);
        assert(g.hp==hp && g.foe_hp==foe_hp);
        assert(g.pika_pp[0]==(uint8_t)(pp-(player?1:0)));
    }
    puts("SLEEP_OCCUPIED_TARGET_RNG_GUARD_PASS");

    for(int player=0;player<=1;player++) {
        YellowGame g;ready(&g);g.rng=424242;
        if(player)g.pika_status=YStatusSleep3;
        else g.foe_status=YStatusSleep3;
        uint32_t old_rng=g.rng;
        uint8_t pp=g.pika_pp[0],hp=g.hp,foe_hp=g.foe_hp;
        battle_action_pipeline(&g,player!=0,YM_TACKLE,false);
        assert((player?g.pika_status:g.foe_status)==YStatusSleep2);
        assert(g.rng==old_rng && g.pika_pp[0]==pp);
        assert(g.hp==hp && g.foe_hp==foe_hp);
    }
    puts("SLEEP_ACTION_SUPPRESSION_PASS");
    printf("ALL_REVIEW_TESTS_PASS hits=%u misses=%u\\n",hits,misses);
    return 0;
}
