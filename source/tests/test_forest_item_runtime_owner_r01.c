#include <assert.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
static void ready(YellowGame* g,int x,int y){
 yellow_init(g);g->map=YMapViridianForest;g->story=YStoryPikachuFollows;
 g->screen=YScreenWorld;g->has_pikachu=1;g->follower_valid=0;
 g->x=x;g->y=y;g->facing=YUp;g->hp=yellow_player_max_hp(g);
}
static void take(YellowGame* g,int x,int y,uint8_t bit,int ball){
 ready(g,x,y+1);
 uint8_t old=ball?g->balls:g->potions;
 assert(!yellow_walkable(g,x,y));
 yellow_input(g,YOk);
 assert(g->screen==YScreenDialog);
 assert((g->forest_pickups&bit)!=0);
 assert((ball?g->balls:g->potions)==old+1u);
 yellow_input(g,YOk);
 assert(g->screen==YScreenWorld);
 assert(yellow_walkable(g,x,y));
 uint8_t count=ball?g->balls:g->potions;
 yellow_input(g,YOk);
 assert((ball?g->balls:g->potions)==count);
}
int main(void){
 YellowGame g;
 take(&g,25,11,1,0);
 take(&g,12,29,2,0);
 take(&g,1,31,4,1);
 ready(&g,25,12);g.potions=99;
 yellow_input(&g,YOk);
 assert(g.screen==YScreenDialog && g.potions==99 && g.forest_pickups==0);
 yellow_input(&g,YOk);
 assert(!yellow_walkable(&g,25,11));
 ready(&g,1,32);g.balls=99;
 yellow_input(&g,YOk);
 assert(g.screen==YScreenDialog && g.balls==99 && g.forest_pickups==0);
 ready(&g,25,12);g.potions=7;g.forest_pickups=1;
 g.forest_defeated_mask=3;
 YellowSave parent;yellow_save_create(&g,&parent);
 assert(parent.version==13);
 YellowItemSave side;yellow_item_save_create(&g,&parent,&side);
 YellowTrainerSave trainer;yellow_trainer_save_create(&g,&parent,&trainer);
 YellowGame restored;
 assert(yellow_save_load_with_items(&restored,&parent,&side));
 assert(restored.forest_pickups==1 && restored.potions==7);
 assert(yellow_walkable(&restored,25,11));
 assert(yellow_trainer_save_load(&restored,&parent,&trainer));
 assert(restored.forest_defeated_mask==3);
 YellowItemSave bad=side;bad.collected_mask=7;
 assert(!yellow_save_load_with_items(&restored,&parent,&bad));
 bad=side;bad.parent_checksum++;
 assert(!yellow_save_load_with_items(&restored,&parent,&bad));
 bad=side;bad.version=7;
 assert(!yellow_save_load_with_items(&restored,&parent,&bad));
 YellowSave wrong=parent;wrong.checksum++;
 assert(!yellow_save_load_with_items(&restored,&wrong,&side));
 assert(yellow_save_load_with_items(&restored,&parent,NULL));
 assert(restored.forest_pickups==0);
 puts("FOREST_ITEM_REAL_PICKUP_CAP_99_SIDECAR_AND_TRAINER_PARITY_PASS");
 return 0;
}
