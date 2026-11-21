#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"

/* Isolated source-backed R0.12-C3 canary; compile against copied v064. */
static void ready(YellowGame* g, uint8_t foe) {
    yellow_init(g);
    g->story = YStoryPikachuFollows;
    g->has_pikachu = 1;
    g->screen = YScreenWorld;
    g->map = YMapRoute1;
    g->x = 10;
    g->y = 28;
    g->hp = yellow_player_max_hp(g);
    yellow_begin_battle(g, foe, 5);
    g->selected = 0;
    g->pika_pp[0] = 5;
    g->rng = 1;
}

int main(void) {
    YellowGame g;
    /* Poison type immunity (primary and secondary typing). */
    ready(&g, YK_NIDORAN_F);
    assert(status_move_type_immune(&g, true, YM_POISONPOWDER));
    uint32_t before = g.rng;
    battle_action_pipeline(&g, true, YM_POISONPOWDER, false);
    assert(g.foe_status == YStatusNone && g.rng == before);
    assert(g.pika_pp[0] == 4); /* action still spends PP */

    ready(&g, YK_ZUBAT); /* Poison/Flying */
    assert(status_move_type_immune(&g, true, YM_POISONPOWDER));
    before = g.rng;
    battle_action_pipeline(&g, true, YM_POISONPOWDER, false);
    assert(g.foe_status == YStatusNone && g.rng == before);

    /* Ground immunity to Electric-type Thunder Wave, either type slot. */
    ready(&g, YK_SANDSHREW); /* Ground/Ground */
    assert(status_move_type_immune(&g, true, YM_THUNDER_WAVE));
    before = g.rng;
    battle_action_pipeline(&g, true, YM_THUNDER_WAVE, false);
    assert(g.foe_status == YStatusNone && g.rng == before);

    ready(&g, YK_GEODUDE); /* Rock/Ground */
    assert(status_move_type_immune(&g, true, YM_THUNDER_WAVE));
    before = g.rng;
    battle_action_pipeline(&g, true, YM_THUNDER_WAVE, false);
    assert(g.foe_status == YStatusNone && g.rng == before);

    /* Ground does NOT block Grass-type Stun Spore. */
    ready(&g, YK_GEODUDE);
    assert(!status_move_type_immune(&g, true, YM_STUN_SPORE));
    battle_action_pipeline(&g, true, YM_STUN_SPORE, false);
    assert(g.foe_status == YStatusParalyzed);

    /* Non-immune controls still inflict their status. */
    ready(&g, YK_RATTATA);
    assert(!status_move_type_immune(&g, true, YM_POISONPOWDER));
    battle_action_pipeline(&g, true, YM_POISONPOWDER, false);
    assert(g.foe_status == YStatusPoisoned);

    ready(&g, YK_RATTATA);
    assert(!status_move_type_immune(&g, true, YM_THUNDER_WAVE));
    battle_action_pipeline(&g, true, YM_THUNDER_WAVE, false);
    assert(g.foe_status == YStatusParalyzed);

    /* Foe attacks a caught dual-type Ground party member. */
    ready(&g, YK_RATTATA);
    g.caught_count = 1;
    g.active_party = 1;
    g.caught[0].species = YK_GEODUDE;
    g.caught[0].level = 5;
    g.caught[0].hp = y_calc_stats(YK_GEODUDE, 5).hp;
    g.caught[0].status = YStatusNone;
    before = g.rng;
    assert(status_move_type_immune(&g, false, YM_THUNDER_WAVE));
    battle_action_pipeline(&g, false, YM_THUNDER_WAVE, false);
    assert(g.caught[0].status == YStatusNone && g.rng == before);

    /* Foe cannot poison caught Nidoran, either status source direction. */
    g.caught[0].species = YK_NIDORAN_M;
    before = g.rng;
    assert(status_move_type_immune(&g, false, YM_POISONPOWDER));
    battle_action_pipeline(&g, false, YM_POISONPOWDER, false);
    assert(g.caught[0].status == YStatusNone && g.rng == before);

    /* No accidental filtering of damage moves, even for invalid target. */
    ready(&g, YK_RATTATA);
    g.foe_kind = 13; /* invalid/reserved species ID */
    assert(!status_move_type_immune(&g, true, YM_TACKLE));
    assert(status_move_type_immune(&g, true, YM_THUNDER_WAVE));

    puts("STATUS_TYPE_IMMUNITY_CANARY_PASS");
    return 0;
}
