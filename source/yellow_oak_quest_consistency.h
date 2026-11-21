#ifndef YELLOW_OAK_QUEST_CONSISTENCY_H
#define YELLOW_OAK_QUEST_CONSISTENCY_H
#include "yellow_oak_parcel_sidecar.h"
#include "yellow_oak_postparcel.h"
/* Pure fail-closed prerequisite for later atomic SD integration.
 * Both records MUST be from the same already-validated YellowSave v13
 * parent generation. No native gameplay or SD hooks are provided here. */
typedef struct { uint8_t parcel_flags; YOakPostState post; } YOakQuestSnapshot;
bool y_oak_quest_consistent(const YOakQuestSnapshot* state);
bool y_oak_quest_pack(const YOakQuestSnapshot* state,uint32_t parent,
                      YOakParcelSidecar* parcel,YOakPostRecord* post);
bool y_oak_quest_unpack(YOakQuestSnapshot* state,uint32_t parent,
                        const YOakParcelSidecar* parcel,const YOakPostRecord* post);
#endif
