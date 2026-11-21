#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "yellow_pewter_progress.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
#include "../yellow_pewter_progress.c"
static void gym_after_brock_roundtrip(void) {
 YellowGame g;
 yellow_init(&g);
 assert(g.pewter_flags==0);
 g.screen=YScreenWorld;
 g.story=YStoryPikachuFollows;
 g.has_pikachu=1;
 g.map=YMapRoute1;
 g.x=10;g.y=20;
 assert(y_pewter_record_gym_trainer(&g.pewter_flags));
 assert(y_pewter_record_brock_victory(&g.pewter_flags));
 assert(!y_pewter_claim_tm34(&g.pewter_flags,false));
 assert(g.pewter_flags==(YPewterFlagGymTrainer0|YPewterFlagBrock|YPewterFlagBoulderBadge));
 YellowSave original;
 yellow_save_create(&g,&original);
 assert(original.version==13 && sizeof(original)==132);
 YPewterProgressSave extra;
 assert(y_pewter_progress_save(g.pewter_flags,original.checksum,&extra));
 YellowGame restored;
 assert(yellow_save_load(&restored,&original));
 assert(restored.pewter_flags==0);
 assert(y_pewter_progress_restore(&restored.pewter_flags,original.checksum,&extra));
 assert(restored.pewter_flags==g.pewter_flags);
 assert(restored.map==YMapRoute1 && restored.x==10 && restored.y==20);
 assert(y_pewter_claim_tm34(&restored.pewter_flags,true));
 assert(restored.pewter_flags==YPewterAllFlags);
 YPewterProgressSave updated;
 YellowSave unchanged;
 yellow_save_create(&restored,&unchanged);
 assert(unchanged.version==13 && unchanged.map==original.map);
 assert(unchanged.x==original.x && unchanged.y==original.y);
 assert(unchanged.hp==original.hp && unchanged.pika_level==original.pika_level);
 assert(y_pewter_progress_save(restored.pewter_flags,unchanged.checksum,&updated));
 YellowGame second;
 assert(yellow_save_load(&second,&unchanged));
 assert(y_pewter_progress_restore(&second.pewter_flags,unchanged.checksum,&updated));
 assert(second.pewter_flags==YPewterAllFlags);
 assert(!y_pewter_claim_tm34(&second.pewter_flags,true));
 /* Parent snapshot replacement must make its predecessor-sidecar unusable. */
 YellowSave new_parent=unchanged;
 new_parent.checksum^=0x10u;
 uint8_t old_flags=second.pewter_flags;
 assert(!y_pewter_progress_restore(&second.pewter_flags,new_parent.checksum,&updated));
 assert(second.pewter_flags==old_flags);
}
int main(void) {
 gym_after_brock_roundtrip();
 puts("PEWTER_V13_SAVE_AND_NEW_GYM_SIDECAR_REAL_GAME_STATE_COMPAT_PASS");
 return 0;
}
