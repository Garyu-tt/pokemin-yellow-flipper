/* Bounded native guard regression. The delivered-parcel flags below are
 * explicitly a fixture; full natural quest is tested by ARM input trace. */
#include <assert.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_antidote_cure.c"
#include "../yellow_core.c"
#include "../yellow_antidote_game.c"
#include "../yellow_pewter_progress.c"
#include "../yellow_viridian_mart_purchase.c"
#include "../yellow_mart_runtime.c"
int main(void) {
    YellowGame g;
    yellow_init(&g);
    g.native_mart_intro_extension=true;
    assert(g.story==YStoryBeforeOak && g.oak_parcel_flags==0u);
    assert(!yellow_game_buy_antidote(&g,1u));
    assert(g.antidotes==0u && g.money_bcd[1]==0x30u);
    /* The fixture isolates a guard against wasting medicine on healthy party. */
    g.has_pikachu=1u;
    g.story=YStoryPikachuFollows;
    g.oak_parcel_flags=YOakParcelAll;
    assert(yellow_game_buy_antidote(&g,1u));
    assert(g.antidotes==1u && g.money_bcd[1]==0x29u);
    assert(g.pika_status==YStatusNone);
    assert(!yellow_game_apply_antidote(&g,0u,&g.antidotes));
    assert(g.antidotes==1u && g.pika_status==YStatusNone);
    g.pika_status=YStatusPoisoned; /* fixture, NOT a natural poison event */
    assert(yellow_game_apply_antidote(&g,0u,&g.antidotes));
    assert(g.antidotes==0u && g.pika_status==YStatusNone);
    puts("UNSEEDED_GUARD_HEALTHY_ITEM_NOT_CONSUMED_PASS");
    return 0;
}
