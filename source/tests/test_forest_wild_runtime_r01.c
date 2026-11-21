#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
static const uint8_t expected_kinds[10]={
 YK_CATERPIE,YK_METAPOD,YK_CATERPIE,YK_CATERPIE,YK_PIDGEY,
 YK_PIDGEY,YK_CATERPIE,YK_METAPOD,YK_PIDGEY,YK_PIDGEOTTO};
static const uint8_t expected_levels[10]={3,4,4,5,4,6,6,6,8,9};
static void ready(YellowGame *g){
 yellow_init(g);g->story=YStoryPikachuFollows;g->has_pikachu=1;
 g->screen=YScreenWorld;g->map=YMapViridianForest;
 g->x=15;g->y=16;g->follower_valid=0;g->hp=yellow_player_max_hp(g);
}
int main(void){
 YellowGame g;
 ready(&g);
 assert(yellow_walkable(&g,15,16)&&yellow_walkable(&g,16,16));
 assert(!yellow_is_grass(YMapViridianForest,15,16));
 assert(yellow_is_grass(YMapViridianForest,16,16));
 assert(yellow_is_grass(YMapViridianForest,16,17));
 for(unsigned i=0;i<10;i++){
  assert(forest_kinds[i]==expected_kinds[i]);
  assert(forest_levels[i]==expected_levels[i]);
  assert(y_species_info(forest_kinds[i])!=NULL);
  assert(y_species_catch_rate(forest_kinds[i])>0);
  unsigned found=0;
  for(uint32_t seed=1;seed<200000&&!found;seed++){
   ready(&g);g.rng=seed;
   YellowGame probe=g;
   if((next_rng(&probe)&255u)>=25u)continue;
   uint8_t roll=(uint8_t)(next_rng(&probe)&255u);
   unsigned slot=0,sum=0;
   while(slot<9){sum+=encounter_weights[slot];if(roll<sum)break;++slot;}
   if(slot!=i)continue;
   yellow_input(&g,YRight);
   assert(g.x==16&&g.y==16);
   assert(g.foe_kind==expected_kinds[i]);
   assert(g.foe_level==expected_levels[i]);
   assert(g.foe_hp>0&&g.screen==YScreenRivalBattle);
   found=1;
  }
  assert(found);
 }
 ready(&g);g.x=16;g.y=16;g.rng=1;
 yellow_input(&g,YLeft);
 assert(g.screen==YScreenWorld&&g.x==15);
 ready(&g);g.hp=0;g.rng=1;
 yellow_input(&g,YRight);
 assert(g.screen==YScreenWorld&&g.x==16);
 puts("FOREST_WILD_PINNED_ALL_10_SLOTS_AND_RUNTIME_PASS");
 return 0;
}
