#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_core.h"
#include "ux_visual_math.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
int main(void) {
    assert(ux_sprite_frame(YDown,0)==0);
    assert(ux_sprite_frame(YUp,1)==4);
    assert(ux_sprite_frame(YLeft,1)==5);
    assert(ux_linear_q8(140,280)==128);
    assert(ux_linear_q8(280,280)==256);
    assert(ux_walk_pose(64) && !ux_walk_pose(128));
    YellowGame g;yellow_init(&g);
    g.screen=YScreenBag;g.selected=0;
    g.has_pikachu=1;
    g.hp=2;g.potions=2;
    yellow_input(&g,YOk);
    assert(g.screen==YScreenBag);
    assert(g.potions==1 && g.hp>=2);
    uint8_t hp=g.hp;
    yellow_input(&g,YBack);
    assert(g.screen==YScreenMenu && g.selected==1);
    assert(g.hp==hp);
    puts("OWNER_UX_R05_WALK_AND_POTION_CORE_PASS");
    return 0;
}
