/* Independent READ-ONLY MassPort B2F acceptance gate, not an existing test.
 * Compile and run against current isolated persistent candidate.
 * A successful C/ARM build is NOT proof that B2F was hooked into runtime.
 */
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "yellow_core.h"
#include "yellow_assets.c"
#include "yellow_battle.c"
#include "yellow_core.c"
int main(void){
 int fail=0;
 /* Explicit activation in full game + this independent acceptance oracle. */
 yellow_b2f_set_enabled(true);
 const YellowLevelAsset* level=yellow_gameplay_scene_asset(YMapMtMoonB2F);
 if(!level || level->w!=20 || level->h!=18){
   printf("FAIL B2F gameplay dispatch: expected 20x18 actual=%dx%d\n",
      level?level->w:0,level?level->h:0);fail++;
 }
 if(yellow_map_w(YMapMtMoonB2F)!=40 || yellow_map_h(YMapMtMoonB2F)!=36){
    puts("FAIL B2F map dimensions not enabled in gameplay");
    fail++;
 }
 if(yellow_world_tile_id(YMapMtMoonB2F,0,0)==0xff){
    puts("FAIL B2F viewport tile renderer not connected");
    fail++;
 }
 static const struct {uint8_t x,y,dx,dy;} warps[]={
    {17,11,25,9},{21,17,21,17},{13,27,15,27},{23,3,5,7}};
 for(unsigned i=0;i<4;i++){
    uint8_t dst=255,x=255,y=255;
    bool ok=destination(YMapMtMoonB1F,warps[i].x,warps[i].y,&dst,&x,&y);
    if(!ok||dst!=YMapMtMoonB2F||x!=warps[i].dx||y!=warps[i].dy){
        printf("FAIL B1F -> B2F warp %u: ok=%u dest=%u (%u,%u)\n",
               i+1,(unsigned)ok,dst,x,y);fail++;
    }
 }
 static const struct {uint8_t x,y,dx,dy;} back[]={
    {25,9,17,11},{21,17,21,17},{15,27,13,27},{5,7,23,3}};
 for(unsigned i=0;i<4;i++){
    uint8_t dst=255,x=255,y=255;
    bool ok=destination(YMapMtMoonB2F,back[i].x,back[i].y,&dst,&x,&y);
    if(!ok||dst!=YMapMtMoonB1F||x!=back[i].dx||y!=back[i].dy){
        printf("FAIL B2F -> B1F warp %u: ok=%u dest=%u (%u,%u)\n",
               i+1,(unsigned)ok,dst,x,y);fail++;
    }
 }
 if(fail)printf("B2F_RUNTIME_NOT_READY %d independent gates failed\n",fail);
 else puts("B2F_RUNTIME_WARP_DISPATCH_PASS");
 return fail?1:0;
}
