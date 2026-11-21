#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
#include "../yellow_pewter_progress.c"
#include "../yellow_atomic_snapshot.c"
static void ready(YellowGame* g) {
 yellow_init(g);
 g->running=true;g->screen=YScreenWorld;g->story=YStoryPikachuFollows;
 g->has_pikachu=1;g->map=YMapMtMoon1F;g->pika_level=25;
 g->hp=y_calc_stats(YK_PIKACHU,25).hp;
 g->experience=y_exp_threshold(YK_PIKACHU,25);
 g->balls=5;g->x=14;g->y=34;
 (void)yellow_dex_mark_caught(&g->dex,25);
}
int main(void) {
 YellowGame game,restored;ready(&game);
 YellowAtomicSnapshot a={0},b={0},partial={0};
 assert(yellow_atomic_make(&game,10,&a));
 assert(yellow_atomic_unpack(&a,NULL));
 assert(a.parent.version==13 && sizeof(a.parent)==132);
 assert(yellow_atomic_select(&a,NULL)==0);
 game.balls=9;game.steps=19;
 assert(yellow_atomic_make(&game,11,&b));
 assert(yellow_atomic_select(&a,&b)==1);
 assert(yellow_atomic_unpack(&b,&restored));
 assert(restored.balls==9 && restored.steps==19);
 assert(restored.map==YMapRoute2);
 for(size_t n=0;n<sizeof(b);n++) {
  memset(&partial,0xa5,sizeof(partial));
  memcpy(&partial,&b,n);
  assert(!yellow_atomic_unpack(&partial,NULL));
  assert(yellow_atomic_select(&a,&partial)==0);
 }
 puts("ATOMIC_SAVE_ALL_BYTE_INTERRUPTS_ROLLBACK_PASS");
 return 0;
}
