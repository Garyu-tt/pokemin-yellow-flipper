#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "yellow_battle.h"
#include "../yellow_battle.c"
int main(void) {
 const YMoveInfo* m0=y_move_info(YM_SLEEP_POWDER);
 assert(m0 && strcmp(m0->name, "SLEEP POWDER")==0);
 assert(m0->power==0 && m0->accuracy==75 && m0->pp==15);
 assert(m0->type==YT_GRASS && m0->special==1);
 const YMoveInfo* m1=y_move_info(YM_POISONPOWDER);
 assert(m1 && strcmp(m1->name, "POISONPOWDER")==0);
 assert(m1->power==0 && m1->accuracy==75 && m1->pp==35);
 assert(m1->type==YT_POISON && m1->special==0);
 const YMoveInfo* m2=y_move_info(YM_STUN_SPORE);
 assert(m2 && strcmp(m2->name, "STUN SPORE")==0);
 assert(m2->power==0 && m2->accuracy==75 && m2->pp==30);
 assert(m2->type==YT_GRASS && m2->special==1);
 puts("MIKOSHI_METADATA_INTEGRATION_PASS");
 return 0;
}
