/* Independent live-runtime exact-position sidecar regression (v064). */
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
#include "../yellow_exact_position_v1.c"
static void ready(YellowGame* g){
 yellow_init(g);g->running=true;g->screen=YScreenWorld;
 g->story=YStoryPikachuFollows;g->has_pikachu=1;g->pika_level=8;
 g->hp=y_calc_stats(YK_PIKACHU,8).hp;
 g->experience=y_exp_threshold(YK_PIKACHU,8);
 g->map=YMapRoute3;g->x=16;g->y=9;g->facing=YLeft;
}
int main(void){
 yellow_b2f_set_enabled(true);
 assert(sizeof(YellowSave)==132 && sizeof(YellowExactPositionV1)==28);
 unsigned good=0, rejected=0;
 for(unsigned map=YMapPewterCity;map<=YMapCeruleanMart;map++){
  const YellowLevelAsset* a=yellow_gameplay_scene_asset((uint8_t)map);
  if(!a)continue;
  int maxx=a->w*2-1,maxy=a->h*2-1;
  if(maxx>127||maxy>127)continue; /* YellowGame coordinates are int8_t */
  for(unsigned n=0;n<4;n++){
   YellowGame g,loaded;ready(&g);g.map=(uint8_t)map;
   g.x=(int8_t)(n==1||n==3?maxx:0);
   g.y=(int8_t)(n==2||n==3?maxy:0);
   YellowAtomicSnapshot p={0};YellowExactPositionV1 pos={0};
   assert(yellow_atomic_make(&g,100+map*4+n,&p));
   assert(yellow_atomic_unpack(&p,&loaded));
   assert(loaded.map==YMapRoute2);
   assert(yellow_exact_position_make(&g,&p,&pos));
   assert(yellow_exact_position_restore(&loaded,&p,&pos));
   assert(loaded.map==g.map&&loaded.x==g.x&&loaded.y==g.y&&loaded.facing==g.facing);
   good++;
   YellowGame snapshot=loaded;
   YellowExactPositionV1 damaged=pos;damaged.checksum^=1;
   assert(!yellow_exact_position_restore(&loaded,&p,&damaged));
   assert(memcmp(&snapshot,&loaded,sizeof(loaded))==0);rejected++;
   YellowAtomicSnapshot newer=p;newer.generation++;
   newer.checksum=yellow_atomic_checksum(&newer);
   assert(yellow_atomic_unpack(&newer,NULL));
   assert(!yellow_exact_position_restore(&loaded,&newer,&pos));
   assert(memcmp(&snapshot,&loaded,sizeof(loaded))==0);rejected++;
  }
 }
 /* A producer must never seal a sidecar that its consumer rejects. */
 YellowGame g,loaded;ready(&g);
 YellowAtomicSnapshot parent={0};YellowExactPositionV1 pos={0};
 assert(yellow_atomic_make(&g,200,&parent));
 assert(yellow_atomic_unpack(&parent,&loaded));
 g.facing=255;
 assert(!yellow_exact_position_make(&g,&parent,&pos));
 /* Legacy v13/atomic state remains intact if extended position cannot be saved. */
 assert(loaded.map==YMapRoute2 && loaded.x==9 && loaded.y==0);
 printf("EXACT_POSITION_TRANSACTION_HOURLY1_PASS good=%u rejected=%u invalid_facing_rejected=1\n",good,rejected);
 return 0;
}
