/* Owner-independent Toxic acceptance oracle; NOT model-authored test.
 * Compile with TCC -I <candidate/source> from this support directory.
 * Original pret/pokeyellow data/moves/moves.asm: TOXIC POISON_EFFECT 0 POISON 85 10.
 * Original engine/battle/effects.asm: Toxic flags badly poisoned, poison types immune.
 * No full R0.12-C3 claim; no acquisition of TM06 or natural gameplay proof.
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"

static void ready(YellowGame* g,uint8_t target) {
 yellow_init(g);
 g->story=YStoryPikachuFollows;
 g->has_pikachu=1;
 g->screen=YScreenWorld;
 g->map=YMapRoute1;
 g->x=10;g->y=28;
 g->hp=yellow_player_max_hp(g);
 g->follower_valid=0;
 yellow_begin_battle(g,target,5);
 g->pika_moves[0]=YM_TOXIC;
 g->pika_pp[0]=10;
 g->selected=0;
 g->active_party=0;
 g->rng=1;
}
int main(void) {
 assert(YM_TOXIC==YM_POUND+1);
 assert(YM_NONE==255);
 const YMoveInfo* info=y_move_info(YM_TOXIC);
 assert(info&&info->power==0&&info->accuracy==85&&info->pp==10);
 assert(info->type==YT_POISON);
 assert(y_max_pp(YM_TOXIC)==10);
 YellowGame g,loaded;
 ready(&g,YK_RATTATA);
 battle_action_pipeline(&g,true,YM_TOXIC,false);
 assert(g.foe_status==YStatusToxic1);
 assert(g.pika_pp[0]==9);
 uint8_t hp=g.foe_hp;
 assert(apply_residual_after_action(&g,false));
 assert(g.foe_status==YStatusToxic2);
 assert(g.foe_hp<hp);
 ready(&g,YK_RATTATA);
 g.foe_status=YStatusParalyzed;
 battle_action_pipeline(&g,true,YM_TOXIC,false);
 assert(g.foe_status==YStatusParalyzed);
 ready(&g,YK_ZUBAT);
 battle_action_pipeline(&g,true,YM_TOXIC,false);
 assert(g.foe_status==YStatusNone); /* original Poison-type immunity */
 ready(&g,YK_RATTATA);
 battle_action_pipeline(&g,false,YM_TOXIC,false);
 assert(g.pika_status==YStatusToxic1);
 /* Toxic is a staged move effect, not yet teachable via a real TM; restore a
  * normally learned move before testing the existing v13 status persistence. */
 g.pika_moves[0]=YM_THUNDERSHOCK;g.pika_pp[0]=y_max_pp(YM_THUNDERSHOCK);
 YellowSave save;
 yellow_save_create(&g,&save);
 assert(save.version==13 && sizeof(save)==132);
 assert(yellow_save_load(&loaded,&save));
 assert(loaded.pika_status==YStatusToxic1);
 int found_miss=0;
 for(uint32_t seed=1;seed<=512;seed++) {
  ready(&g,YK_RATTATA);
  g.rng=seed;
  battle_action_pipeline(&g,true,YM_TOXIC,false);
  if(g.foe_status==YStatusNone) {
   assert(g.pika_pp[0]==9);
   found_miss=1;break;
  }
 }
 assert(found_miss);
 puts("OWNER_TOXIC_MOVE_SOURCE_EFFECT_IMMUNITY_PP_RNG_SAVE_PASS");
 return 0;
}
