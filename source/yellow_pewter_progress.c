#include "yellow_pewter_progress.h"
/* Independently keyed optional sidecar. Existing 132-byte YellowSave v13
   remains byte-for-byte unchanged. No badge without a won Brock fight.
   Original PewterGym.asm allows a full bag to defer TM34 while retaining
   EVENT_BEAT_BROCK and BIT_BOULDERBADGE. */
bool y_pewter_flags_valid(uint8_t flags) {
 if((flags&~YPewterAllFlags)!=0)return false;
 if((flags&YPewterFlagBoulderBadge) && !(flags&YPewterFlagBrock))return false;
 if((flags&YPewterFlagBrock) && !(flags&YPewterFlagBoulderBadge))return false;
 if((flags&YPewterFlagTM34) && !(flags&YPewterFlagBrock))return false;
 return true;
}
bool y_pewter_record_gym_trainer(uint8_t* flags) {
 if(!flags || !y_pewter_flags_valid(*flags))return false;
 *flags|=YPewterFlagGymTrainer0;return true;
}
bool y_pewter_record_brock_victory(uint8_t* flags) {
 if(!flags || !y_pewter_flags_valid(*flags))return false;
 *flags|=YPewterFlagBrock|YPewterFlagBoulderBadge;
 return true;
}
bool y_pewter_claim_tm34(uint8_t* flags,bool has_bag_space) {
 if(!flags || !y_pewter_flags_valid(*flags) ||
    !(*flags&YPewterFlagBrock) || !has_bag_space ||
    (*flags&YPewterFlagTM34))return false;
 *flags|=YPewterFlagTM34;
 return true;
}
static uint32_t pewter_checksum(const YPewterProgressSave* p) {
 uint32_t h=2166136261u;
 const uint32_t values[4]={p->magic,(uint32_t)p->version,
                           (uint32_t)p->flags|((uint32_t)p->reserved<<8u),
                           p->parent_checksum};
 for(unsigned i=0;i<4;i++) {
  for(unsigned j=0;j<4;j++) {
   h^=(uint8_t)(values[i]>>(8u*j));
   h*=16777619u;
  }
 }
 return h;
}
bool y_pewter_progress_save(uint8_t flags,uint32_t parent_checksum,YPewterProgressSave* out) {
 if(!out || !y_pewter_flags_valid(flags))return false;
 YPewterProgressSave s={0};
 s.magic=YELLOW_PEWTER_SIDECAR_MAGIC;
 s.version=YELLOW_PEWTER_SIDECAR_VERSION;
 s.flags=flags;
 s.parent_checksum=parent_checksum;
 s.checksum=pewter_checksum(&s);
 *out=s;return true;
}
bool y_pewter_progress_restore(uint8_t* target,uint32_t parent_checksum,
                                const YPewterProgressSave* saved) {
 if(!target || !saved || saved->magic!=YELLOW_PEWTER_SIDECAR_MAGIC ||
    saved->version!=YELLOW_PEWTER_SIDECAR_VERSION ||
    saved->reserved!=0 || saved->parent_checksum!=parent_checksum ||
    !y_pewter_flags_valid(saved->flags) ||
    saved->checksum!=pewter_checksum(saved))return false;
 *target=saved->flags;return true;
}
