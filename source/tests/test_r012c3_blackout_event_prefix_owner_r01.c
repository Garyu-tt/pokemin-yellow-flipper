/* Independent owner test for pret/pokeyellow engine/events/poison.asm
 * BLACKED_OUT event prefix. NOT full Gen1 whiteout/recovery.
 */
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
#include "../yellow_pewter_progress.c"
#include "../yellow_atomic_snapshot.c"
#include "../yellow_exact_position_v1.c"

static void ready(YellowGame* g) {
 yellow_init(g);g->running=true;g->screen=YScreenWorld;
 g->story=YStoryPikachuFollows;g->has_pikachu=1;
 g->map=YMapRoute9;g->follower_valid=0;g->steps=3;
 g->hp=1;g->pika_status=YStatusPoisoned;g->caught_count=0;
 for(int y=2;y<yellow_map_h(YMapRoute9)-2;y++)
  for(int x=3;x<yellow_map_w(YMapRoute9)-2;x++)
   if(yellow_walkable(g,x,y)&&yellow_walkable(g,x+1,y)){
    g->x=x;g->y=y;g->facing=YRight;return;
   }
 assert(!"no original source-backed passable Route9 pair");
}
static void one_mon_blackout(void) {
 YellowGame g,loaded;YellowSave save;ready(&g);
 uint8_t map=g.map;
 yellow_input(&g,YRight);
 assert(g.hp==0 && g.steps==4);
 assert(g.screen==YScreenDialog && g.overworld_blackout_pending==1);
 assert(g.dialog_count==1 && g.dialog_page==0);
 assert(g.dialog_lines && strcmp(g.dialog_lines[0],"BLACKED OUT!")==0);
 assert(g.map==map && g.hp==0); /* no invented teleport/heal */
 yellow_input(&g,YOk);
 assert(g.screen==YScreenWorld && g.overworld_blackout_pending==1);
 assert(g.map==map && g.hp==0);
 yellow_input(&g,YBack); /* menu must not re-open the dialog */
 assert(g.screen==YScreenMenu);
 yellow_input(&g,YBack);
 assert(g.screen==YScreenWorld);
 yellow_save_create(&g,&save);
 assert(sizeof(YellowSave)==132 && save.version==13);
 /* v13 deliberately rejects all-fainted parties until a real source-backed
  * blackout recovery path exists. Never silently manufacture a heal or spawn.
  */
 assert(!yellow_save_load(&loaded,&save));
 assert(save.version==13 && save.hp==0 && save.steps==4);
 /* An existing nurse/heal event must allow a later distinct blackout. */
 yellow_heal_pikachu(&g);
 assert(g.hp==yellow_player_max_hp(&g));
 assert(g.overworld_blackout_pending==0);
 g.hp=1;g.pika_status=YStatusPoisoned;g.steps=7;g.facing=YLeft;
 yellow_input(&g,YLeft);
 assert(g.steps==8 && g.hp==0 && g.screen==YScreenDialog);
 assert(g.overworld_blackout_pending==1);
}
static void healthy_party_no_blackout(void) {
 YellowGame g;ready(&g);
 g.caught_count=1;
 g.caught[0].species=YK_PIDGEY;
 g.caught[0].level=5;
 g.caught[0].hp=3;
 g.caught[0].status=YStatusNone;
 g.caught[0].experience=y_exp_threshold(YK_PIDGEY,5);
 y_build_moveset(YK_PIDGEY,5,g.caught[0].moves);
 for(unsigned i=0;i<YELLOW_MOVE_SLOTS;i++)
  g.caught[0].pp[i]=y_max_pp(g.caught[0].moves[i]);
 yellow_input(&g,YRight);
 assert(g.hp==0 && g.caught[0].hp==3);
 assert(g.overworld_blackout_pending==0 && g.screen==YScreenWorld);
 YellowSave saved;YellowGame loaded;
 yellow_save_create(&g,&saved);
 assert(yellow_save_load(&loaded,&saved));
 assert(loaded.hp==0 && loaded.caught_count==1 && loaded.caught[0].hp==3);
 assert(loaded.overworld_blackout_pending==0);
}
static void blocked_step_no_blackout(void) {
 YellowGame g;ready(&g);
 int found=0;
 for(int y=2;y<yellow_map_h(YMapRoute9)-2&&!found;y++)
  for(int x=2;x<yellow_map_w(YMapRoute9)-2&&!found;x++)
   if(yellow_walkable(&g,x,y)&&!yellow_walkable(&g,x-1,y)){
    g.x=x;g.y=y;g.facing=YRight;
    uint16_t before=g.steps;
    yellow_input(&g,YLeft);
    assert(g.steps==before && g.hp==1);
    assert(g.overworld_blackout_pending==0 && g.screen==YScreenWorld);
    found=1;
   }
 assert(found);
}
int main(void){
 one_mon_blackout();
 healthy_party_no_blackout();
 blocked_step_no_blackout();
 puts("SOURCE_POISON_BLACKOUT_DIALOG_NO_FAKE_RECOVERY_V13_PASS");
 return 0;
}
