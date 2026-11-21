#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"

static uint32_t fuzz_rng=0x13FEEDB1u;
static uint32_t r(void) {
    fuzz_rng^=fuzz_rng<<13;
    fuzz_rng^=fuzz_rng>>17;
    fuzz_rng^=fuzz_rng<<5;
    return fuzz_rng;
}
static void check_party(const YellowGame* g) {
    assert(g->map<YELLOW_MAP_COUNT);
    assert(g->screen<=YScreenMoveForget);
    assert(yellow_party_size(g)<=YELLOW_PARTY_MAX);
    assert(g->caught_count<=YELLOW_CAPTURE_SLOTS);
    assert(g->pika_level>=5 && g->pika_level<=100);
    assert(g->hp<=yellow_player_max_hp(g));
    assert(yellow_valid_moves(YK_PIKACHU,g->pika_level,g->pika_moves,g->pika_pp));
    for(uint8_t i=0;i<g->caught_count;i++) {
        const YCapturedMon* m=&g->caught[i];
        assert(m->species==YK_PIDGEY||m->species==YK_RATTATA);
        assert(m->level>=2 && m->level<=100);
        assert(m->hp<=y_calc_stats(m->species,m->level).hp);
        assert(yellow_valid_moves(m->species,m->level,m->moves,m->pp));
    }
    assert(g->active_party<yellow_party_size(g));
    assert(g->foe_hp<=g->foe_max_hp || g->screen==YScreenWorld || g->screen==YScreenDialog || g->screen==YScreenMenu);
}
int main(void) {
    YellowGame g;yellow_init(&g);
    /* Keep the original startup and map, then use deterministic advanced
       scenario to stress just the native C state transitions. */
    while(g.screen==YScreenDialog)yellow_input(&g,YOk);
    g.story=YStoryPikachuFollows;g.has_pikachu=1;
    g.map=YMapRoute1;g.x=10;g.y=28;g.follower_valid=0;
    g.balls=70;
    for(unsigned n=0;n<16000;n++) {
        if(g.screen==YScreenWorld && (r()&127u)==0) {
            yellow_begin_battle(&g,(r()&1u)?YK_PIDGEY:YK_RATTATA,
                                 (uint8_t)(2u+(r()%7u)));
        }
        YellowKey key=(YellowKey)(r()%6u);
        if(g.screen==YScreenDialog || g.screen==YScreenOakEncounter)
            key=YOk;
        if(g.screen==YScreenMoveForget)key=YBack;
        if(g.screen==YScreenRivalBattle)key=YOk;
        if(g.screen==YScreenParty && g.force_switch) {
            if(yellow_can_switch(&g,g.selected))key=YOk;
            else key=(r()&1u)?YDown:YUp;
        }
        if(g.screen==YScreenMenu && g.selected==3)key=YUp;
        yellow_input(&g,key);
        assert(g.running);
        check_party(&g);
        if(g.screen==YScreenWorld && (n%257u)==0) {
            YellowSave save;yellow_save_create(&g,&save);
            YellowGame copy;
            bool ok=yellow_save_load(&copy,&save);
            if(!ok)fprintf(stderr,"SAVE-FAIL n=%u screen=%d map=%u loc=%u,%u pikaHP=%u active=%u party=%u story=%u exp=%lu lev=%u
",
                 n,g.screen,g.map,(unsigned)g.x,(unsigned)g.y,g.hp,g.active_party,g.caught_count,g.story,(unsigned long)g.experience,g.pika_level);
            assert(ok);
            assert(copy.pika_level==g.pika_level);
            assert(copy.caught_count==g.caught_count);
            assert(copy.experience==g.experience);
        }
    }
    puts("R10_16000_DETERMINISTIC_STATE_TRANSITIONS_PASS");
    return 0;
}
