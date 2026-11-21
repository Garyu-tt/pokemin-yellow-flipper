#ifndef YELLOW_OAK_POSTPARCEL_H
#define YELLOW_OAK_POSTPARCEL_H
#include <stdbool.h>
#include <stdint.h>
/* Bounded, NOT yet wired to native NPC/dialog/SD hooks. Source:
 * upstream/pokeyellow/scripts/OaksLab.asm:
 * OaksLabOak1Text -> OaksLabRivalArrivesAtOaksRequestScript ->
 * OaksLabOakGivesPokedexScript -> OaksLabRivalLeavesWithPokedexScript.
 * EVENT_GOT_POKEDEX occurs only during OakGivesPokedexScript, not
 * immediately when EVENT_OAK_GOT_PARCEL is recorded.
 */
enum {
 YOakPostWait=0, YOakPostRivalArriving=1, YOakPostRequest=2,
 YOakPostRivalLeaving=3, YOakPostDone=4
};
enum {
 YOakPostNoEvent=0, YOakPostSpawnRival=1, YOakPostBeginRequest=2,
 YOakPostGrantPokedex=3, YOakPostEnableRoute22Rival=4
};
enum {
 YOakPostOakDialogueFinished=1, YOakPostRivalArrived=2,
 YOakPostRequestFinished=3, YOakPostRivalExited=4
};
#define Y_OAK_POST_MAGIC 0x31504f50u /* POP1 */
#define Y_OAK_POST_VERSION 1u
typedef struct {
 uint8_t phase;
 uint8_t pokedex_owned;
 uint8_t route22_rival_enabled;
} YOakPostState;
typedef struct {
 uint32_t magic;
 uint16_t version;
 uint8_t phase;
 uint8_t flags;
 uint32_t parent_checksum;
 uint32_t checksum;
} YOakPostRecord;
void y_oak_post_init(YOakPostState* state);
bool y_oak_post_valid(const YOakPostState* state);
uint8_t y_oak_post_step(YOakPostState* state, uint8_t input,
                        uint8_t parcel_flags, bool parcel_integrity_error,
                        bool lab_starter_battle_done);
bool y_oak_post_pack(const YOakPostState* state, uint32_t parent_checksum,
                     YOakPostRecord* record);
bool y_oak_post_unpack(YOakPostState* state, uint32_t parent_checksum,
                       const YOakPostRecord* record);
#endif
