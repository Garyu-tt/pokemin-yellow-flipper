#include <assert.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
static void ready(YellowGame* g) {
 yellow_init(g);
 g->story=YStoryPikachuFollows;g->screen=YScreenWorld;
 g->map=YMapViridianForest;g->has_pikachu=1;
 g->follower_valid=0;g->hp=yellow_player_max_hp(g);
}
int main(void) {
 YellowGame g;ready(&g);
 /* Connected first action: genuine one-time Potion pickup. */
 g.x=25;g.y=12;g.facing=YUp;
 uint8_t old_potions=g.potions;
 yellow_input(&g,YOk);
 assert(g.screen==YScreenDialog && g.forest_pickups==1);
 assert(g.potions==old_potions+1);
 yellow_input(&g,YOk);assert(g.screen==YScreenWorld);
 /* Original Bug Catcher #15 at 13,17 faces RIGHT and has sight range 4. */
 g.x=15;g.y=17;g.facing=YDown;
 assert(yellow_walkable(&g,14,17));
 yellow_input(&g,YLeft);
 assert(g.x==14 && g.y==17 && g.screen==YScreenDialog);
 assert((g.forest_seen_mask&8)==8 && (g.forest_defeated_mask&8)==0);
 yellow_input(&g,YOk);assert(g.screen==YScreenWorld);
 yellow_input(&g,YOk);assert(g.screen==YScreenRivalBattle);
 assert(g.forest_active_trainer==3 && g.foe_kind==YK_CATERPIE && g.foe_level==8);
 finalize_battle(&g,true);
 assert(g.screen==YScreenRivalBattle && g.foe_kind==YK_METAPOD && g.foe_level==8);
 assert((g.forest_defeated_mask&8)==0);
 finalize_battle(&g,true);
 assert(g.screen==YScreenDialog && (g.forest_defeated_mask&8)==8);
 yellow_input(&g,YOk);assert(g.screen==YScreenWorld);
 /* Three sidecars remain compatible; previous beaten trainers preserved. */
 YellowSave parent;yellow_save_create(&g,&parent);
 YellowItemSave items;yellow_item_save_create(&g,&parent,&items);
 YellowTrainerSave trainers;yellow_trainer_save_create(&g,&parent,&trainers);
 assert(trainers.defeated_mask==8 && items.collected_mask==1);
 YellowGame restored;
 assert(yellow_save_load_with_items(&restored,&parent,&items));
 assert(yellow_trainer_save_load(&restored,&parent,&trainers));
 assert(restored.forest_pickups==1 && (restored.forest_defeated_mask&8)==8);
 /* Unsupported Lass #19 remains separate; a defeated catcher cannot retrigger. */
 restored.x=15;restored.y=17;restored.facing=YDown;
 restored.hp=0;
 yellow_input(&restored,YLeft);
 assert(restored.x==14 && restored.y==17 && restored.screen==YScreenWorld);
 YellowTrainerSave invalid=trainers;invalid.defeated_mask=0x10;
 assert(!yellow_trainer_save_load(&restored,&parent,&invalid));
 YellowGame backward;ready(&backward);backward.forest_defeated_mask=7;
 YellowSave old_parent;yellow_save_create(&backward,&old_parent);
 YellowTrainerSave old_sidecar;yellow_trainer_save_create(&backward,&old_parent,&old_sidecar);
 assert(old_sidecar.defeated_mask==7);
 puts("FOREST_CONNECTED_ITEM_FOURTH_BUGCATCHER_DEFEAT_SAVE_PASS");
 return 0;
}
