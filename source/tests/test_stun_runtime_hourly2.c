/* R15 independent Stun Spore runtime regression. Host-only: includes engine internals. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
static void ready(YellowGame *g,uint8_t foe) {
    yellow_init(g);g->story=YStoryPikachuFollows;g->has_pikachu=1;
    g->screen=YScreenWorld;g->map=YMapRoute1;g->x=10;g->y=28;
    yellow_begin_battle(g,foe,5);g->selected=0;g->pika_pp[0]=30;
}
static uint32_t seed_for(bool hit) {
    for(uint32_t seed=1;seed<100000;seed++) {
        YellowGame g;ready(&g,YK_RATTATA);g.rng=seed;
        uint8_t roll=(uint8_t)next_rng(&g);
        if(y_move_hits(YM_STUN_SPORE,0,0,roll)==hit)return seed;
    }
    assert(!"no deterministic seed found");return 0;
}
int main(void) {
    uint32_t hit=seed_for(true),miss=seed_for(false);
    YellowGame g;ready(&g,YK_GEODUDE);g.rng=hit;
    uint32_t expected=next_rng(&g);g.rng=hit;
    uint8_t pp=g.pika_pp[0];
    battle_action_pipeline(&g,true,YM_STUN_SPORE,false);
    assert(g.foe_status==YStatusParalyzed && g.rng==expected);
    assert(g.pika_pp[0]==pp-1); /* one accuracy draw, no damage RNG */
    ready(&g,YK_RATTATA);g.rng=miss;expected=next_rng(&g);g.rng=miss;
    battle_action_pipeline(&g,true,YM_STUN_SPORE,false);
    assert(g.foe_status==YStatusNone && g.rng==expected);
    ready(&g,YK_RATTATA);g.foe_status=YStatusPoisoned;g.rng=hit;
    battle_action_pipeline(&g,true,YM_STUN_SPORE,false);
    assert(g.foe_status==YStatusPoisoned && g.rng==hit);
    ready(&g,YK_RATTATA);g.rng=hit;expected=next_rng(&g);g.rng=hit;
    battle_action_pipeline(&g,false,YM_STUN_SPORE,false);
    assert(yellow_party_status(&g,g.active_party)==YStatusParalyzed && g.rng==expected);
    assert(y_effective_speed(YK_PIKACHU,5,YStatusParalyzed)==y_calc_stats(YK_PIKACHU,5).speed/4u);
    assert(y_full_paralysis(62) && !y_full_paralysis(63));
    g.screen=YScreenWorld;g.foe_hp=0;
    YellowSave save;yellow_save_create(&g,&save);
    YellowGame restored;assert(yellow_save_load(&restored,&save));
    assert(yellow_party_status(&restored,restored.active_party)==YStatusParalyzed);
    puts("STUN_RUNTIME_R15_HIT_MISS_IMMUNITY_RNG_PP_SAVE_PASS");
    return 0;
}
