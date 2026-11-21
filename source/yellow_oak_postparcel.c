#include "yellow_oak_postparcel.h"
#include "yellow_oak_parcel_sidecar.h"
/* Checksums detect accidental corruption and wrong save generation;
 * NOT a cryptographic authentication or an integrated SD transaction. */
static uint32_t checksum(const YOakPostRecord* r) {
 uint32_t words[3]={r->magic,
  (uint32_t)r->version|((uint32_t)r->phase<<16)|((uint32_t)r->flags<<24),
  r->parent_checksum};
 uint32_t h=2166136261u;
 for(unsigned i=0;i<3;i++)for(unsigned j=0;j<4;j++){
  h^=(uint8_t)(words[i]>>(8u*j));h*=16777619u;
 }
 return h;
}
void y_oak_post_init(YOakPostState* s) {
 if(s)*s=(YOakPostState){0,0,0};
}
bool y_oak_post_valid(const YOakPostState* s) {
 if(!s||s->phase>YOakPostDone||
    s->pokedex_owned>1||s->route22_rival_enabled>1)return false;
 if(s->phase<YOakPostRivalLeaving)return
  !s->pokedex_owned&&!s->route22_rival_enabled;
 if(s->phase==YOakPostRivalLeaving)return
  s->pokedex_owned&&!s->route22_rival_enabled;
 return s->pokedex_owned&&s->route22_rival_enabled;
}
uint8_t y_oak_post_step(YOakPostState* s,uint8_t input,
                        uint8_t parcel_flags,bool integrity_error,
                        bool starter_battle_done) {
 if(!y_oak_post_valid(s)||integrity_error||!starter_battle_done||
    !y_oak_parcel_valid(parcel_flags)||parcel_flags!=YOakParcelAll)
  return YOakPostNoEvent;
 switch(s->phase) {
 case YOakPostWait:
  if(input==YOakPostOakDialogueFinished){
   s->phase=YOakPostRivalArriving;return YOakPostSpawnRival;
  }break;
 case YOakPostRivalArriving:
  if(input==YOakPostRivalArrived){
   s->phase=YOakPostRequest;return YOakPostBeginRequest;
  }break;
 case YOakPostRequest:
  if(input==YOakPostRequestFinished){
   s->phase=YOakPostRivalLeaving;s->pokedex_owned=1;
   return YOakPostGrantPokedex;
  }break;
 case YOakPostRivalLeaving:
  if(input==YOakPostRivalExited){
   s->phase=YOakPostDone;s->route22_rival_enabled=1;
   return YOakPostEnableRoute22Rival;
  }break;
 default:break;
 }
 return YOakPostNoEvent;
}
bool y_oak_post_pack(const YOakPostState* s,uint32_t parent,
                     YOakPostRecord* r) {
 if(!r||!y_oak_post_valid(s))return false;
 YOakPostRecord tmp={0};
 tmp.magic=Y_OAK_POST_MAGIC;tmp.version=Y_OAK_POST_VERSION;
 tmp.phase=s->phase;
 tmp.flags=(uint8_t)(s->pokedex_owned|(s->route22_rival_enabled<<1));
 tmp.parent_checksum=parent;tmp.checksum=checksum(&tmp);
 *r=tmp;return true;
}
bool y_oak_post_unpack(YOakPostState* s,uint32_t parent,
                       const YOakPostRecord* r) {
 if(!s||!r||r->magic!=Y_OAK_POST_MAGIC||
    r->version!=Y_OAK_POST_VERSION||r->parent_checksum!=parent||
    (r->flags&~3u)!=0||r->checksum!=checksum(r))return false;
 YOakPostState tmp={r->phase,(uint8_t)(r->flags&1u),
                    (uint8_t)((r->flags>>1)&1u)};
 if(!y_oak_post_valid(&tmp))return false;
 *s=tmp;return true;
}
