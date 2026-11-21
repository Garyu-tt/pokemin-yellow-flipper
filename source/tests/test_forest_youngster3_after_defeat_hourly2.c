/* Independent runtime regression: Viridian Forest Bug Catcher #2 (30,19),
 * source: pinned pret/pokeyellow/text/ViridianForest.asm
 * _ViridianForestYoungster3AfterBattleText. No mocked input or save calls. */
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
#define CHECK(cond, msg) do { if(!(cond)) { fprintf(stderr, "FAIL:%s line=%d\n", msg, __LINE__); return 41; } } while(0)
int main(void) {
    YellowGame g;
    yellow_init(&g);
    g.story=YStoryPikachuFollows;
    g.screen=YScreenWorld;
    g.map=YMapViridianForest;
    g.has_pikachu=1;
    g.hp=yellow_player_max_hp(&g);
    g.follower_valid=0;
    g.x=29;g.y=19;g.facing=YRight;
    yellow_input(&g,YOk);
    CHECK(g.screen==YScreenDialog && (g.forest_seen_mask&2u), "manual first interaction");
    yellow_input(&g,YOk);
    CHECK(g.screen==YScreenWorld, "challenge closes");
    yellow_input(&g,YOk);
    CHECK(g.screen==YScreenRivalBattle && g.forest_active_trainer==1u, "actual trainer battle");
    CHECK(g.foe_kind==YK_METAPOD && g.foe_level==6, "original first party slot");
    for(unsigned i=0;i<3;i++) {
        CHECK(g.screen==YScreenRivalBattle && g.forest_active_trainer==1u, "trainer multi-mon chain");
        finalize_battle(&g,true);
    }
    CHECK(g.screen==YScreenDialog && (g.forest_defeated_mask&2u), "defeat flag after final slot");
    yellow_input(&g,YOk);
    CHECK(g.screen==YScreenWorld, "victory dialog closes");
    YellowSave parent;
    yellow_save_create(&g,&parent);
    YellowTrainerSave side;
    yellow_trainer_save_create(&g,&parent,&side);
    CHECK(parent.version==13 && side.version==1 && side.defeated_mask==2u, "backwards compatible sidecar");
    YellowGame loaded;
    CHECK(yellow_save_load(&loaded,&parent), "load original parent");
    CHECK(loaded.forest_defeated_mask==0, "legacy save does not fabricate flags");
    CHECK(yellow_trainer_save_load(&loaded,&parent,&side), "load sidecar");
    CHECK(loaded.forest_defeated_mask==2u, "persistent defeat flag");
    loaded.map=YMapViridianForest;
    loaded.screen=YScreenWorld;
    loaded.x=29;loaded.y=19;loaded.facing=YRight;
    loaded.follower_valid=0;
    yellow_input(&loaded,YOk);
    CHECK(loaded.screen==YScreenDialog && loaded.pending==YPendingNone, "defeated trainer speaks without rematch");
    CHECK(loaded.dialog_count==2, "original after-battle dialogue uses two pages");
    CHECK(strcmp(loaded.dialog_lines[0],"DARN! I'M GOING")==0, "upstream line 1");
    CHECK(strcmp(loaded.dialog_lines[1],"TO CATCH SOME")==0, "upstream line 2");
    CHECK(strcmp(loaded.dialog_lines[2],"STRONGER ONES!")==0, "upstream line 3");
    yellow_input(&loaded,YOk);
    CHECK(loaded.screen==YScreenDialog && loaded.dialog_page==1, "page transition");
    yellow_input(&loaded,YOk);
    CHECK(loaded.screen==YScreenWorld && loaded.forest_active_trainer==255 && loaded.foe_hp==0, "no rematch");
    puts("FOREST_YOUNGSTER3_AFTER_DEFEAT_SAVE_INPUT_PASS");
    return 0;
}
