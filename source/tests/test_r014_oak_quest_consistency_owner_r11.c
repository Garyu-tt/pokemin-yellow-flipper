#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../yellow_oak_parcel_sidecar.c"
#include "../yellow_oak_postparcel.c"
#include "../yellow_oak_quest_consistency.c"
int main(void) {
 const uint32_t parent=0xBADBEEFu;
 YOakQuestSnapshot initial={YOakParcelAll,{YOakPostWait,0,0}};
 YOakParcelSidecar parcel;YOakPostRecord post;
 assert(y_oak_quest_pack(&initial,parent,&parcel,&post));
 YOakQuestSnapshot restored={0};
 assert(y_oak_quest_unpack(&restored,parent,&parcel,&post));
 assert(restored.parcel_flags==YOakParcelAll && restored.post.phase==YOakPostWait);
 assert(y_oak_post_step(&initial.post,YOakPostOakDialogueFinished,YOakParcelAll,false,true)==YOakPostSpawnRival);
 assert(y_oak_post_step(&initial.post,YOakPostRivalArrived,YOakParcelAll,false,true)==YOakPostBeginRequest);
 assert(y_oak_post_step(&initial.post,YOakPostRequestFinished,YOakParcelAll,false,true)==YOakPostGrantPokedex);
 assert(y_oak_post_step(&initial.post,YOakPostRivalExited,YOakParcelAll,false,true)==YOakPostEnableRoute22Rival);
 assert(y_oak_quest_pack(&initial,parent,&parcel,&post));
 assert(y_oak_quest_unpack(&restored,parent,&parcel,&post));
 assert(restored.post.pokedex_owned==1 && restored.post.route22_rival_enabled==1);
 YOakQuestSnapshot old=restored;
 assert(!y_oak_quest_unpack(&restored,parent+1,&parcel,&post));
 assert(memcmp(&old,&restored,sizeof(old))==0);
 post.checksum^=1u;
 assert(!y_oak_quest_unpack(&restored,parent,&parcel,&post));
 assert(memcmp(&old,&restored,sizeof(old))==0);
 post.checksum^=1u;
 YOakParcelSidecar premature;
 assert(y_oak_parcel_save(YOakParcelGot,parent,&premature));
 assert(!y_oak_quest_unpack(&restored,parent,&premature,&post));
 assert(memcmp(&old,&restored,sizeof(old))==0);
 YOakQuestSnapshot invalid={YOakParcelGot,{YOakPostDone,1,1}};
 assert(!y_oak_quest_pack(&invalid,parent,&parcel,&post));
 assert(!y_oak_quest_unpack(&restored,parent,&parcel,NULL));
 assert(!y_oak_quest_unpack(NULL,parent,&parcel,&post));
 puts("R014_OAK_PARCEL_POSTPARCEL_CROSS_GENERATION_GUARD_PASS");
 puts("NOT_NATIVE_SD_OR_GAMEPLAY_INTEGRATION");
 return 0;
}
