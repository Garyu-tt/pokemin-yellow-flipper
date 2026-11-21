/* Independent RED source-backed all-party poison blackout prerequisite.
 * pret/pokeyellow/engine/events/poison.asm: AnyPartyAlive == false after
 * fourth-step poison => DisplayTextID(TEXT_BLACKED_OUT) and set
 * wOutOfBattleBlackout. The current port has no accepted blackout UX/state.
 * Do not interpret this failing test as a full-game acceptance result.
 */
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
#include "../yellow_pewter_progress.c"
#include "../yellow_atomic_snapshot.c"
#include "../yellow_exact_position_v1.c"

static void ready(YellowGame* g) {
    yellow_init(g);
    g->story=YStoryPikachuFollows;
    g->has_pikachu=1;
    g->screen=YScreenWorld;
    g->map=YMapRoute9;
    g->follower_valid=0;
    g->caught_count=0;
    g->hp=1;
    g->pika_status=YStatusPoisoned;
    g->steps=3;
    for(int y=2;y<yellow_map_h(YMapRoute9)-2;y++) {
      for(int x=3;x<yellow_map_w(YMapRoute9)-2;x++) {
        if(yellow_walkable(g,x,y) && yellow_walkable(g,x+1,y)) {
          g->x=x;g->y=y;g->facing=YRight;
          return;
        }
      }
    }
    assert(!"original Route9 passable pair missing");
}

int main(void) {
    YellowGame g;
    ready(&g);
    yellow_input(&g,YRight);
    if(g.hp!=0 || g.steps!=4) {
        puts("BLOCKED_ORIGINAL_POISON_STEP_DAMAGE");
        return 2;
    }
    if(g.screen==YScreenWorld) {
        puts("BLOCKED_ORIGINAL_ALL_PARTY_POISON_BLACKOUT_DIALOG");
        return 3;
    }
    puts("BLACKOUT_DIALOG_PREREQUISITE_ONLY_PASS_NOT_FULL_WHITEOUT");
    return 0;
}
