/* v064 integration guard: new dialogue must not bypass the existing
 * two-Pokemon trainer battle, defeated flag, or v13 save compatibility.
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
int main(void) {
    YellowGame g;yellow_init(&g);
    g.story=YStoryPikachuFollows;
    g.map=YMapViridianForest;
    g.screen=YScreenWorld;
    g.has_pikachu=1;
    g.follower_valid=0;
    g.x=29;g.y=33;g.facing=YRight;
    yellow_input(&g,YOk);
    assert(g.screen==YScreenDialog && g.dialog_count==2);
    assert(g.dialog_lines==forest_youngster2_challenge);
    assert(g.forest_seen_mask&1);
    assert(g.forest_defeated_mask==0 && g.foe_hp==0);
    yellow_input(&g,YOk);
    assert(g.screen==YScreenDialog&&g.dialog_page==1);
    yellow_input(&g,YOk);
    assert(g.screen==YScreenWorld);
    yellow_input(&g,YOk);
    assert(g.screen==YScreenRivalBattle);
    assert(g.forest_active_trainer==0);
    assert(g.foe_kind==YK_CATERPIE&&g.foe_level==7);
    finalize_battle(&g,true);
    assert(g.forest_active_trainer==0&&g.forest_party_index==1);
    finalize_battle(&g,true);
    assert(g.forest_active_trainer==255&&(g.forest_defeated_mask&1));
    yellow_input(&g,YOk);
    assert(g.screen==YScreenWorld);
    YellowSave parent;yellow_save_create(&g,&parent);
    YellowTrainerSave side;yellow_trainer_save_create(&g,&parent,&side);
    assert(parent.version==13&&side.version==1&&(side.defeated_mask&1));
    yellow_input(&g,YOk);
    assert(g.screen==YScreenDialog&&g.foe_hp==0);
    puts("V064_FOREST_MANUAL_DIALOGUE_BATTLE_DEFEAT_SAVE_PASS");
    return 0;
}
