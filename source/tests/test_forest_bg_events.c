#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
int main(void) {
 const uint8_t sx[]={24,16,26,4,18,2};
 const uint8_t sy[]={40,32,17,24,45,1};
 const char* const* dialogs[]={forest_tips1,forest_antidote_sign,forest_tips2,forest_tips3,forest_tips4,forest_leave_sign};
 const uint8_t pages[]={2,2,2,2,2,1};
 for(unsigned i=0;i<6;i++) {
  YellowGame g; yellow_init(&g);
  g.story=YStoryPikachuFollows;g.map=YMapViridianForest;
  g.screen=YScreenWorld;g.has_pikachu=0;g.follower_valid=0;
  g.x=sx[i];g.y=sy[i]+1;g.facing=YUp;
  const uint8_t potions=g.potions,balls=g.balls;
  const uint32_t rng=g.rng;
  const uint16_t steps=g.steps;
  yellow_input(&g,YOk);
  assert(g.screen==YScreenDialog);
  assert(g.dialog_lines==dialogs[i]);
  assert(g.dialog_count==pages[i] && g.pending==YPendingNone);
  for(unsigned page=1;page<pages[i];page++) {
   yellow_input(&g,YOk);
   assert(g.screen==YScreenDialog && g.dialog_page==page);
  }
  yellow_input(&g,YOk);
  assert(g.screen==YScreenWorld && g.map==YMapViridianForest);
  assert(g.rng==rng && g.steps==steps && g.balls==balls && g.potions==potions);
 }
 YellowGame g;yellow_init(&g);g.map=YMapViridianForest;
 g.story=YStoryPikachuFollows;g.screen=YScreenWorld;
 g.x=9;g.y=9;g.facing=YUp;g.has_pikachu=0;
 yellow_input(&g,YOk);
 assert(g.screen==YScreenWorld);
 puts("R13_FOREST_SIX_PINNED_BG_EVENTS_PASS");
 return 0;
}
