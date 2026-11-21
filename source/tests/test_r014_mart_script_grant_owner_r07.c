#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "yellow_oak_parcel_sidecar.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
#include "../yellow_pewter_progress.c"
#include "../yellow_atomic_snapshot.c"
#include "../yellow_exact_position_v1.c"

static void setup(YellowGame* g, uint8_t flags, bool corrupt, bool enabled) {
 yellow_init(g);g->running=true;g->screen=YScreenWorld;
 g->story=YStoryPikachuFollows;g->has_pikachu=1;g->follower_valid=0;
 g->map=YMapViridian;g->x=29;g->y=18;g->facing=YDown;
 g->oak_parcel_flags=flags;g->oak_parcel_integrity_error=corrupt;
 g->native_mart_intro_extension=enabled;
}
static void acknowledge(YellowGame* g, unsigned n) {
 for(unsigned i=0;i<n;i++)yellow_input(g,YOk);
}
static void test_actual_source_sequence_and_deferred_grant(void) {
 YellowGame g;setup(&g,0,false,true);
 assert(sizeof(YellowSave)==132u);
 assert(yellow_viridian_mart_passable(2,6));
 assert(yellow_viridian_mart_passable(2,5));
 assert(yellow_viridian_mart_passable(2,4));
 yellow_input(&g,YDown);
 assert(g.map==YMapViridianMart&&g.x==3&&g.y==6);
 assert(g.screen==YScreenDialog&&g.native_mart_intro_pending==1);
 assert(g.steps==1&&g.oak_parcel_flags==0);
 acknowledge(&g,1);
 assert(g.screen==YScreenDialog&&g.steps==1&&g.oak_parcel_flags==0);
 acknowledge(&g,1);
 assert(g.native_mart_intro_pending==0);
 assert(g.screen==YScreenDialog&&g.pending==YPendingGrantOakParcel);
 assert(g.x==2&&g.y==4&&g.steps==4);
 assert(g.facing==YUp);
 assert(g.oak_parcel_flags==0&&!y_oak_parcel_in_bag(g.oak_parcel_flags,false));
 acknowledge(&g,2);
 assert(g.screen==YScreenDialog&&g.oak_parcel_flags==0);
 acknowledge(&g,1);
 assert(g.screen==YScreenWorld&&g.oak_parcel_flags==YOakParcelGot);
 assert(y_oak_parcel_in_bag(g.oak_parcel_flags,g.oak_parcel_integrity_error));
 YellowSave save;yellow_save_create(&g,&save);
 assert(save.version==13u);
 YOakParcelSidecar parcel;
 assert(y_oak_parcel_save(g.oak_parcel_flags,save.checksum,&parcel));
 YellowGame loaded;assert(yellow_save_load(&loaded,&save));
 uint8_t flags=0;
 assert(y_oak_parcel_restore(&flags,save.checksum,&parcel));
 assert(flags==YOakParcelGot);
 assert(y_oak_parcel_in_bag(flags,false));
 assert(!y_oak_parcel_record_got(&flags)); /* cannot duplicate */
 assert(flags==YOakParcelGot);
 assert(!y_oak_parcel_restore(&flags,save.checksum+1u,&parcel));
 assert(flags==YOakParcelGot);
 puts("R014_SOURCE_SCRIPT_1LEFT_2UP_PARCEL_AFTER_DIALOG_V13_SIDECAR_PASS");
}
static void test_reentry_and_fail_closed(void) {
 YellowGame g;
 for(unsigned i=0;i<4;i++) {
  uint8_t flags=i==0?YOakParcelGot:i==1?YOakParcelAll:i==2?0:0x80u;
  bool corrupt=i==2;
  setup(&g,flags,corrupt,true);
  yellow_input(&g,YDown);
  assert(g.map==YMapViridianMart&&g.x==3&&g.y==6);
  assert(g.screen==YScreenWorld&&g.native_mart_intro_pending==0);
  assert(g.oak_parcel_flags==flags);
 }
 setup(&g,0,false,false);
 yellow_input(&g,YDown);
 assert(g.map==YMapViridianMart&&g.screen==YScreenWorld);
 assert(g.oak_parcel_flags==0);
 puts("R014_NO_DUPLICATE_GRANT_NO_CORRUPT_REPLAY_NO_FAKE_EXTENSION_PASS");
}
int main(void) {
 test_actual_source_sequence_and_deferred_grant();
 test_reentry_and_fail_closed();
 puts("R014_MART_SCRIPT_GRANT_OWNER_R07_ALL_PASS");
 return 0;
}
