#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
int main(void) {
 YellowGame g;yellow_init(&g);
 g.story=YStoryPikachuFollows;g.map=YMapViridianForest;
 g.screen=YScreenWorld;g.has_pikachu=1;g.follower_valid=0;
 g.x=28;g.y=33;g.facing=YDown;
 g.pika_level=30;g.experience=y_exp_threshold(YK_PIKACHU,30);
 g.hp=yellow_player_max_hp(&g);
 y_build_moveset(YK_PIKACHU,30,g.pika_moves);
 for(int i=0;i<YELLOW_MOVE_SLOTS;i++)g.pika_pp[i]=y_max_pp(g.pika_moves[i]);
 yellow_input(&g,YRight); assert(g.screen==YScreenDialog);
 yellow_input(&g,YOk);assert(g.screen==YScreenWorld);
 yellow_input(&g,YOk);assert(g.screen==YScreenRivalBattle);
 unsigned attack_rounds=0;
 while(g.screen==YScreenRivalBattle || g.screen==YScreenFightMenu) {
  if(++attack_rounds>50) return 40;
  if(g.screen==YScreenRivalBattle)yellow_input(&g,YOk);
  if(g.screen==YScreenFightMenu) {
   g.selected=0;yellow_input(&g,YOk);
  }
 }
 assert(g.screen==YScreenDialog);
 assert(g.forest_defeated_mask&1u);
 yellow_input(&g,YOk);
 assert(g.screen==YScreenWorld);
 printf("FOREST_ACTUAL_BUTTON_COMBAT_PASS rounds=%u\n",attack_rounds);
 return 0;
}
