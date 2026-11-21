/* Independent R16 runtime smoke: 8 B1F/B2F directional input crossings.
 * Uses exact source-linked map coordinates; no save mutation or mock warp data.
 */
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "yellow_core.h"
#include "yellow_assets.c"
#include "yellow_battle.c"
#include "yellow_core.c"
static int pass,fail;
static void cross(uint8_t map,uint8_t x,uint8_t y,
                   YellowKey dir,uint8_t dst,uint8_t dx,uint8_t dy) {
  YellowGame g;
  yellow_init(&g);
  g.running=true;g.screen=YScreenWorld;
  g.story=YStoryPikachuFollows;g.has_pikachu=0;g.follower_valid=0;
  g.map=map;g.x=x;g.y=y;g.facing=dir;
  yellow_input(&g,dir);
  if(g.map!=dst || g.x!=dx || g.y!=dy) {
    fprintf(stderr,"FAIL %u (%u,%u) -> expected %u (%u,%u), got %u (%d,%d) screen %u\n",
            map,x,y,dst,dx,dy,g.map,g.x,g.y,g.screen);fail++;
  } else pass++;
}
int main(void) {
  assert(!yellow_gameplay_scene_asset(YMapMtMoonB2F));
  yellow_b2f_set_enabled(true);
  cross(YMapMtMoonB1F,17,10,YDown,YMapMtMoonB2F,25,9);
  cross(YMapMtMoonB1F,21,16,YDown,YMapMtMoonB2F,21,17);
  cross(YMapMtMoonB1F,13,26,YDown,YMapMtMoonB2F,15,27);
  cross(YMapMtMoonB1F,23,2,YDown,YMapMtMoonB2F,5,7);
  cross(YMapMtMoonB2F,25,8,YDown,YMapMtMoonB1F,17,11);
  cross(YMapMtMoonB2F,21,16,YDown,YMapMtMoonB1F,21,17);
  cross(YMapMtMoonB2F,15,26,YDown,YMapMtMoonB1F,13,27);
  cross(YMapMtMoonB2F,5,6,YDown,YMapMtMoonB1F,23,3);
  if(!fail)printf("B2F_EIGHT_LIVE_INPUT_WARPS_PASS %d\n",pass);
  return fail?1:0;
}
