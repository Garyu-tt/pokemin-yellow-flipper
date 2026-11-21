/* New exact-position v1 extension. Protects unchanged v13 and v1 atomics.
 * Verifies binding to parent, generation, negative and torn sidecar cases.
 */
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
 yellow_init(g);
 g->running=true;g->screen=YScreenWorld;g->story=YStoryPikachuFollows;
 g->has_pikachu=1;g->pika_level=8;
 g->hp=y_calc_stats(YK_PIKACHU,8).hp;
 g->experience=y_exp_threshold(YK_PIKACHU,8);
 g->balls=5;g->map=YMapCeruleanCity;g->x=24;g->y=18;
 g->facing=YLeft;
}
int main(void){
 yellow_b2f_set_enabled(true);
 assert(sizeof(YellowSave)==132);
 assert(sizeof(YellowExactPositionV1)==28);
 const uint8_t maps[]={YMapPewterCity,YMapPewterGym,YMapRoute3,
                        YMapRoute4,YMapMtMoon1F,YMapMtMoonB1F,
                        YMapMtMoonB2F,YMapCeruleanCity};
 unsigned accepted=0;
 for(unsigned i=0;i<sizeof(maps);i++){
  YellowGame game,restored;ready(&game);
  game.map=maps[i];
  game.x=3;game.y=3;
  assert(yellow_exact_position_bounds(game.map,game.x,game.y));
  YellowAtomicSnapshot parent={0};
  assert(yellow_atomic_make(&game,20+i,&parent));
  assert(parent.parent.map==YMapRoute2 && parent.parent.x==9 &&
         parent.parent.y==0);
  assert(parent.parent.version==13);
  assert(yellow_atomic_unpack(&parent,&restored));
  assert(restored.map==YMapRoute2);
  YellowExactPositionV1 pos={0};
  assert(yellow_exact_position_make(&game,&parent,&pos));
  assert(pos.parent_checksum==parent.parent.checksum);
  assert(pos.generation==parent.generation);
  assert(yellow_exact_position_restore(&restored,&parent,&pos));
  assert(restored.map==maps[i]&&restored.x==3&&restored.y==3&&
         restored.facing==YLeft);
  accepted++;
  YellowExactPositionV1 fault=pos;
  fault.parent_checksum^=1;
  assert(!yellow_exact_position_restore(&restored,&parent,&fault));
  fault=pos;fault.generation++;
  assert(!yellow_exact_position_restore(&restored,&parent,&fault));
  fault=pos;fault.x=255;fault.checksum=yellow_exact_position_hash(&fault);
  assert(!yellow_exact_position_restore(&restored,&parent,&fault));
  fault=pos;fault.map=YMapPallet;fault.checksum=yellow_exact_position_hash(&fault);
  assert(!yellow_exact_position_restore(&restored,&parent,&fault));
  fault=pos;fault.facing=255;fault.checksum=yellow_exact_position_hash(&fault);
  assert(!yellow_exact_position_restore(&restored,&parent,&fault));
  fault=pos;fault.version++;
  assert(!yellow_exact_position_restore(&restored,&parent,&fault));
  fault=pos;fault.seal=0;
  assert(!yellow_exact_position_restore(&restored,&parent,&fault));
  fault=pos;fault.checksum^=1;
  assert(!yellow_exact_position_restore(&restored,&parent,&fault));
  for(unsigned n=0;n<sizeof(pos);n++){
   YellowExactPositionV1 truncated={0};
   memcpy(&truncated,&pos,n);
   assert(!yellow_exact_position_restore(&restored,&parent,&truncated));
  }
  YellowAtomicSnapshot other=parent;other.generation++;
  other.checksum=yellow_atomic_checksum(&other);
  assert(yellow_atomic_unpack(&other,NULL));
  assert(!yellow_exact_position_restore(&restored,&other,&pos));
 }
 assert(accepted==8);
 /* Legacy saves carry no sidecar, still round-trip their exact old location. */
 YellowGame old,loaded;ready(&old);
 old.map=YMapRoute2;old.x=7;old.y=11;
 YellowAtomicSnapshot legacy={0};
 assert(yellow_atomic_make(&old,1,&legacy));
 assert(yellow_atomic_unpack(&legacy,&loaded));
 assert(loaded.map==YMapRoute2&&loaded.x==7&&loaded.y==11);
 YellowExactPositionV1 out={0};
 assert(!yellow_exact_position_make(&old,&legacy,&out));
 assert(!yellow_exact_position_restore(&loaded,&legacy,&out));
 assert(loaded.map==YMapRoute2&&loaded.x==7&&loaded.y==11);
 /* Corrupt sidecar fails without mutation of safely restored atomic parent. */
 YellowGame g;ready(&g);
 YellowAtomicSnapshot p={0};
 assert(yellow_atomic_make(&g,44,&p));
 assert(yellow_atomic_unpack(&p,&loaded));
 assert(loaded.map==YMapRoute2);
 assert(yellow_exact_position_make(&g,&p,&out));
 out.checksum^=1;
 assert(!yellow_exact_position_restore(&loaded,&p,&out));
 assert(loaded.map==YMapRoute2&&loaded.x==9&&loaded.y==0);
 printf("EXACT_POSITION_SIDECAR_V1_PASS %u maps, atomic+legacy intact, reject corruption\n",accepted);
 return 0;
}
