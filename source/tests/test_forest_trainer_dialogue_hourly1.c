/* Independent runtime regression: one original Viridian Forest trainer dialogue.
 * Based on pinned pret/pokeyellow/text/ViridianForest.asm Youngster2BattleText.
 * All inputs use actual yellow_input(), collisions and dialogue paging.
 */
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
#define CHECK(c,msg) do {if(!(c)){fprintf(stderr,"FAIL:%s at line %d\n",msg,__LINE__);return 40;}}while(0)
static void world(YellowGame *g,int x,int y) {
    yellow_init(g);
    g->running=true;g->screen=YScreenWorld;
    g->story=YStoryPikachuFollows;g->map=YMapViridianForest;
    g->has_pikachu=1;g->hp=0;g->follower_valid=0;
    g->x=(int8_t)x;g->y=(int8_t)y;g->facing=YDown;
}
int main(void) {
    YellowGame g;
    world(&g,29,33);
    g.facing=YRight;
    unsigned steps=g.steps;
    yellow_input(&g,YOk);
    CHECK(g.x==29&&g.y==33&&g.steps==steps,"manual dialogue does not move");
    CHECK(g.screen==YScreenDialog&&g.pending==YPendingNone,"trainer interaction enters dialogue");
    CHECK(g.foe_hp==0,"no phantom trainer battle");
    CHECK(g.dialog_count==2,"two-page original challenge");
    CHECK(strcmp(g.dialog_lines[0],"HEY! YOU HAVE")==0,"upstream line 1");
    CHECK(strcmp(g.dialog_lines[1],"POKEMON! COME ON!")==0,"upstream line 2");
    CHECK(strcmp(g.dialog_lines[2],"LET'S BATTLE 'EM!")==0,"upstream line 3");
    yellow_input(&g,YLeft);
    CHECK(g.screen==YScreenDialog&&g.x==29,"no movement while talking");
    yellow_input(&g,YOk);
    CHECK(g.screen==YScreenDialog&&g.dialog_page==1,"page two");
    yellow_input(&g,YOk);
    CHECK(g.screen==YScreenWorld&&g.foe_hp==0,"dialogue ends without combat");

    world(&g,28,33);
    CHECK(yellow_walkable(&g,29,33),"LOS approach is walkable");
    yellow_input(&g,YRight);
    CHECK(g.x==29&&g.screen==YScreenDialog&&g.dialog_lines==forest_trainer,
          "legacy LOS remains unchanged until acceptance test migration");

    world(&g,16,44);
    g.facing=YUp;
    yellow_input(&g,YOk);
    CHECK(g.screen==YScreenDialog&&g.dialog_lines==forest_trainer,
          "non-sighting youngster remains unchanged");

    world(&g,29,19);
    g.facing=YRight;
    yellow_input(&g,YOk);
    CHECK(g.screen==YScreenDialog&&g.dialog_lines==forest_trainer,
          "different trainer retains existing dialogue");
    puts("FOREST_YOUNGSTER2_MANUAL_CHALLENGE_RUNTIME_PASS");
    return 0;
}
