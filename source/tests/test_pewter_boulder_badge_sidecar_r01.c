#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "yellow_pewter_progress.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
#include "../yellow_pewter_progress.c"
static void expected_original_flags(void) {
 uint8_t state=0;
 assert(y_pewter_flags_valid(0));
 assert(!y_pewter_claim_tm34(&state,true));
 assert(state==0);
 assert(y_pewter_record_gym_trainer(&state));
 assert(state==YPewterFlagGymTrainer0);
 assert(y_pewter_record_brock_victory(&state));
 assert((state&(YPewterFlagBrock|YPewterFlagBoulderBadge))==
        (YPewterFlagBrock|YPewterFlagBoulderBadge));
 assert(!(state&YPewterFlagTM34));
 assert(!y_pewter_claim_tm34(&state,false)); /* bag-full original route */
 assert(!(state&YPewterFlagTM34));
 assert(y_pewter_claim_tm34(&state,true));
 assert((state&YPewterAllFlags)==YPewterAllFlags);
 assert(!y_pewter_claim_tm34(&state,true)); /* no duplicating TM34 */
 assert(state==YPewterAllFlags);
 assert(!y_pewter_flags_valid(YPewterFlagBrock));
 assert(!y_pewter_flags_valid(YPewterFlagBoulderBadge));
 assert(!y_pewter_flags_valid(YPewterFlagTM34));
 assert(!y_pewter_flags_valid(0x80));
}
static void save_compat_and_tamper_detection(void) {
 YellowGame g;yellow_init(&g);
 g.story=YStoryPikachuFollows;g.has_pikachu=1;g.map=YMapRoute1;
 g.x=10;g.y=20;g.screen=YScreenWorld;
 YellowSave parent;yellow_save_create(&g,&parent);
 assert(parent.version==13 && sizeof(parent)==132);
 YPewterProgressSave p={0};
 uint8_t flags=0;
 assert(y_pewter_record_brock_victory(&flags));
 assert(y_pewter_progress_save(flags,parent.checksum,&p));
 assert(sizeof(p)==16);
 assert(p.version==1 && p.magic==YELLOW_PEWTER_SIDECAR_MAGIC);
 YellowGame loaded;
 assert(yellow_save_load(&loaded,&parent));
 assert(loaded.map==YMapRoute1);
 uint8_t saved_state=0;
 assert(y_pewter_progress_restore(&saved_state,parent.checksum,&p));
 assert(saved_state==flags);
 assert(!y_pewter_progress_restore(&saved_state,parent.checksum^1u,&p));
 assert(saved_state==flags);
 YPewterProgressSave broken=p;
 broken.flags^=YPewterFlagTM34;
 assert(!y_pewter_progress_restore(&saved_state,parent.checksum,&broken));
 broken=p;broken.reserved=1;
 assert(!y_pewter_progress_restore(&saved_state,parent.checksum,&broken));
 broken=p;broken.version=2;
 assert(!y_pewter_progress_restore(&saved_state,parent.checksum,&broken));
 broken=p;broken.magic=0;
 assert(!y_pewter_progress_restore(&saved_state,parent.checksum,&broken));
 broken=p;broken.parent_checksum++;
 assert(!y_pewter_progress_restore(&saved_state,parent.checksum,&broken));
 broken=p;broken.checksum^=1;
 assert(!y_pewter_progress_restore(&saved_state,parent.checksum,&broken));
 assert(!y_pewter_progress_save(YPewterFlagBrock,parent.checksum,&broken));
 assert(!y_pewter_progress_save(flags,parent.checksum,NULL));
 assert(!y_pewter_progress_restore(NULL,parent.checksum,&p));
 assert(!y_pewter_progress_restore(&saved_state,parent.checksum,NULL));
 assert(saved_state==flags);
 /* Existing save image remains unaffected by new optional state. */
 YellowSave second;yellow_save_create(&g,&second);
 assert(memcmp(&parent,&second,sizeof(parent))==0);
}
int main(void) {
 expected_original_flags();
 save_compat_and_tamper_detection();
 puts("PINNED_BROCK_BOULDER_BADGE_TM34_VERSIONED_SIDECAR_V13_ABI_PASS");
 return 0;
}
