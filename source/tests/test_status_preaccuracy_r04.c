/* Independent source-pinned Gen1 non-damaging status ordering regression. */
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"

static void ready(YellowGame* g,uint8_t move) {
    yellow_init(g);
    g->story=YStoryPikachuFollows;
    g->has_pikachu=1;
    g->screen=YScreenWorld;
    g->map=YMapRoute1;g->x=10;g->y=28;
    g->hp=yellow_player_max_hp(g);
    yellow_begin_battle(g,YK_RATTATA,3);
    g->pika_moves[0]=move;g->pika_pp[0]=35;g->selected=0;
}
static uint8_t expected_status(uint8_t move) {
    if(move==YM_POISONPOWDER)return YStatusPoisoned;
    return YStatusParalyzed;
}
int main(void) {
    unsigned occupied=0, normal=0, hits=0, misses=0;
    const uint8_t moves[]={YM_POISONPOWDER,YM_THUNDER_WAVE,YM_STUN_SPORE};
    const uint8_t existing[]={YStatusPoisoned,YStatusParalyzed,
                              YStatusFrozen,YStatusSleep3};
    for(unsigned m=0;m<3;m++)for(unsigned player=0;player<2;player++) {
        for(unsigned q=0;q<4;q++) {
            YellowGame g;ready(&g,moves[m]);g.rng=0x1234u;
            if(player)g.foe_status=existing[q];
            else g.pika_status=existing[q];
            uint8_t hp=g.hp,foe_hp=g.foe_hp,pp=g.pika_pp[0];
            battle_action_pipeline(&g,player!=0,moves[m],false);
            assert((player?g.foe_status:g.pika_status)==existing[q]);
            assert(g.rng==0x1234u); /* Gen1: no MoveHitTest on occupied status */
            assert(g.hp==hp && g.foe_hp==foe_hp);
            assert(g.pika_pp[0]==pp-(player?1u:0u));
            occupied++;
        }
        for(uint32_t seed=1;seed<=4096;seed++) {
            YellowGame g;ready(&g,moves[m]);g.rng=seed;
            YellowGame oracle=g;
            uint8_t roll=(uint8_t)next_rng(&oracle);
            int hit=y_move_hits(moves[m],0,0,roll);
            uint8_t hp=g.hp,foe_hp=g.foe_hp;
            battle_action_pipeline(&g,player!=0,moves[m],false);
            assert(g.rng==oracle.rng); /* ONLY one move accuracy RNG */
            assert((player?g.foe_status:g.pika_status)==
                (hit?expected_status(moves[m]):YStatusNone));
            assert(g.hp==hp && g.foe_hp==foe_hp); /* no phantom damage */
            assert(g.pika_pp[0]==35-(player?1u:0u));
            if(hit)hits++;else misses++;
            normal++;
        }
    }
    assert(occupied==24 && normal==24576 && hits>0 && misses>0);
    printf("STATUS_PREACCURACY_24600_PASS occupied=%u fresh=%u hit=%u miss=%u\n",
        occupied,normal,hits,misses);
    return 0;
}
