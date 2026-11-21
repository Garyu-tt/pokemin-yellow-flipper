#ifndef YELLOW_PEWTER_PROGRESS_H
#define YELLOW_PEWTER_PROGRESS_H
#include <stdbool.h>
#include <stdint.h>
/* Reflects independent original Gen1 event flags, not a claimed full script VM. */
enum {
 YPewterFlagGymTrainer0=1u<<0,  /* EVENT_BEAT_PEWTER_GYM_TRAINER_0 */
 YPewterFlagBrock=1u<<1,        /* EVENT_BEAT_BROCK */
 YPewterFlagBoulderBadge=1u<<2, /* BIT_BOULDERBADGE */
 YPewterFlagTM34=1u<<3,         /* EVENT_GOT_TM34 */
 YPewterAllFlags=0x0fu
};
#define YELLOW_PEWTER_SIDECAR_MAGIC 0x59504731u /* YPG1 */
#define YELLOW_PEWTER_SIDECAR_VERSION 1u
typedef struct {
 uint32_t magic;
 uint16_t version;
 uint8_t flags,reserved;
 uint32_t parent_checksum;
 uint32_t checksum;
} YPewterProgressSave;
bool y_pewter_flags_valid(uint8_t flags);
bool y_pewter_record_gym_trainer(uint8_t* flags);
bool y_pewter_record_brock_victory(uint8_t* flags);
bool y_pewter_claim_tm34(uint8_t* flags,bool has_bag_space);
bool y_pewter_progress_save(uint8_t flags,uint32_t parent_checksum,YPewterProgressSave* out);
bool y_pewter_progress_restore(uint8_t* destination,uint32_t parent_checksum,const YPewterProgressSave* saved);
#endif
