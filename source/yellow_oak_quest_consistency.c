#include "yellow_oak_quest_consistency.h"
/* Pinned pret/pokeyellow/scripts/OaksLab.asm: rival/Pokedex sequence
 * follows EVENT_OAK_GOT_PARCEL; never grant it before parcel delivery.
 * This helper only cross-validates state. It is not a game-event engine. */
bool y_oak_quest_consistent(const YOakQuestSnapshot* s) {
 if(!s||!y_oak_parcel_valid(s->parcel_flags)||!y_oak_post_valid(&s->post))
  return false;
 if(s->post.phase!=YOakPostWait && s->parcel_flags!=YOakParcelAll)
  return false;
 return true;
}
bool y_oak_quest_pack(const YOakQuestSnapshot* s,uint32_t parent,
                      YOakParcelSidecar* parcel,YOakPostRecord* post) {
 if(!parcel||!post||!y_oak_quest_consistent(s))return false;
 YOakParcelSidecar p={0};YOakPostRecord q={0};
 if(!y_oak_parcel_save(s->parcel_flags,parent,&p)||
    !y_oak_post_pack(&s->post,parent,&q))return false;
 *parcel=p;*post=q;return true;
}
bool y_oak_quest_unpack(YOakQuestSnapshot* s,uint32_t parent,
                        const YOakParcelSidecar* parcel,const YOakPostRecord* post) {
 if(!s||!parcel||!post)return false;
 YOakQuestSnapshot candidate={0};
 if(!y_oak_parcel_restore(&candidate.parcel_flags,parent,parcel)||
    !y_oak_post_unpack(&candidate.post,parent,post)||
    !y_oak_quest_consistent(&candidate))return false;
 *s=candidate;return true;
}
