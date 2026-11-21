/* R11 trusted original-source B1F warp acceptance oracle. NOT model-generated.
   Maps from pinned pret/pokeyellow object events, warp indices 1-based.
   Read-only candidate test; does not modify source or accepted tests. */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_core.h"
#include "yellow_assets.c"
#include "yellow_battle.c"
#include "yellow_core.c"

static void expect_warp(uint8_t source,int x,int y,uint8_t dest,int dx,int dy){
 uint8_t map=255,tx=255,ty=255;
 assert(yellow_original_warp_at(source,x,y,&map,&tx,&ty));
 assert(map==dest);
 assert(tx==dx);
 assert(ty==dy);
}
int main(void){
 uint8_t map=255,tx=255,ty=255;
 assert(YMapMtMoon1F==15);
 assert(YMapMtMoonB1F==16);
 assert(yellow_original_warp_count()==28);
 /* Original MtMoonB1F.asm -> Original MtMoon1F.asm warp IDs 3/4/5. */
 expect_warp(YMapMtMoonB1F,5,5,YMapMtMoon1F,5,5);
 expect_warp(YMapMtMoonB1F,25,9,YMapMtMoon1F,17,11);
 expect_warp(YMapMtMoonB1F,25,15,YMapMtMoon1F,25,15);
 /* Exact reciprocal object events, 1F warp IDs 3/4/5. */
 expect_warp(YMapMtMoon1F,5,5,YMapMtMoonB1F,5,5);
 expect_warp(YMapMtMoon1F,17,11,YMapMtMoonB1F,25,9);
 expect_warp(YMapMtMoon1F,25,15,YMapMtMoonB1F,25,15);
 /* B2F (4 events) and LAST_MAP (1 event) unported; never fabricate. */
 assert(!yellow_original_warp_at(YMapMtMoonB1F,17,11,&map,&tx,&ty));
 assert(!yellow_original_warp_at(YMapMtMoonB1F,21,17,&map,&tx,&ty));
 assert(!yellow_original_warp_at(YMapMtMoonB1F,13,27,&map,&tx,&ty));
 assert(!yellow_original_warp_at(YMapMtMoonB1F,23,3,&map,&tx,&ty));
 assert(!yellow_original_warp_at(YMapMtMoonB1F,27,3,&map,&tx,&ty));
 assert(!yellow_original_warp_at(YMapMtMoonB2F,1,1,&map,&tx,&ty));
 puts("R11_PINNED_B1F_RECIPROCAL_WARPS_PASS");
 return 0;
}
