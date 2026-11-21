#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
static void ready(YellowGame *g) {
 yellow_init(g);g->story=YStoryPikachuFollows;g->has_pikachu=1;
 g->screen=YScreenMenu;g->selected=1;g->map=YMapRoute1;
 g->x=10;g->y=28;g->hp=5;g->potions=2;
 yellow_input(g,YOk);
 assert(g->screen==YScreenBag && g->selected==0);
 assert(g->bag_notice==YBagNoNotice && g->bag_last_heal==0);
}
int main(void) {
 YellowGame g;ready(&g);
 uint8_t max=yellow_player_max_hp(&g);
 yellow_input(&g,YOk);
 assert(g.bag_notice==YBagHealed);
 assert(g.bag_last_heal==(uint8_t)(max-5>20?20:max-5));
 assert(g.hp==(uint8_t)(5+g.bag_last_heal));
 assert(g.potions==1);
 yellow_input(&g,YOk);
 assert(g.bag_notice==YBagFullHP && g.bag_last_heal==0);
 assert(g.potions==1 && g.hp==max);
 yellow_input(&g,YDown);
 assert(g.selected==1 && g.bag_notice==YBagNoNotice);
 yellow_input(&g,YUp);
 assert(g.selected==0 && g.bag_notice==YBagNoNotice);
 g.potions=0;yellow_input(&g,YOk);
 assert(g.bag_notice==YBagNoPotions && !g.bag_last_heal && g.hp==max);
 g.potions=1;g.hp=0;yellow_input(&g,YOk);
 assert(g.bag_notice==YBagFainted && g.hp==0 && g.potions==1);
 g.has_pikachu=0;yellow_input(&g,YOk);
 assert(g.bag_notice==YBagNoParty && g.potions==1);
 yellow_input(&g,YBack);
 assert(g.screen==YScreenMenu && g.selected==1);
 assert(g.bag_notice==YBagNoNotice && g.bag_last_heal==0);
 puts("OWNER_BAG_RUNTIME_FEEDBACK_PASS");return 0;
}
