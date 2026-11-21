#include <assert.h>
#include <stdio.h>
#include "../yellow_oak_parcel_sidecar.c"
#include "../yellow_oak_postparcel.c"
static void progression(void) {
 YOakPostState s;YOakPostRecord r; y_oak_post_init(&s);
 assert(y_oak_post_valid(&s));
 /* Wrong prerequisites and out-of-order input never mint a Pokedex. */
 assert(!y_oak_post_step(&s,YOakPostOakDialogueFinished,YOakParcelGot,false,true));
 assert(!y_oak_post_step(&s,YOakPostOakDialogueFinished,YOakParcelAll,true,true));
 assert(!y_oak_post_step(&s,YOakPostOakDialogueFinished,YOakParcelAll,false,false));
 assert(!y_oak_post_step(&s,YOakPostRequestFinished,YOakParcelAll,false,true));
 assert(s.phase==YOakPostWait&&!s.pokedex_owned);
 assert(y_oak_post_step(&s,YOakPostOakDialogueFinished,YOakParcelAll,false,true)==YOakPostSpawnRival);
 assert(!s.pokedex_owned&&!s.route22_rival_enabled);
 assert(!y_oak_post_step(&s,YOakPostRequestFinished,YOakParcelAll,false,true));
 assert(y_oak_post_step(&s,YOakPostRivalArrived,YOakParcelAll,false,true)==YOakPostBeginRequest);
 assert(!s.pokedex_owned);
 assert(y_oak_post_pack(&s,0x12345678u,&r));
 YOakPostState recovered; y_oak_post_init(&recovered);
 assert(y_oak_post_unpack(&recovered,0x12345678u,&r));
 assert(recovered.phase==YOakPostRequest&&!recovered.pokedex_owned);
 assert(y_oak_post_step(&recovered,YOakPostRequestFinished,YOakParcelAll,false,true)==YOakPostGrantPokedex);
 assert(recovered.pokedex_owned&&!recovered.route22_rival_enabled);
 assert(!y_oak_post_step(&recovered,YOakPostRequestFinished,YOakParcelAll,false,true));
 assert(y_oak_post_step(&recovered,YOakPostRivalExited,YOakParcelAll,false,true)==YOakPostEnableRoute22Rival);
 assert(recovered.phase==YOakPostDone&&recovered.route22_rival_enabled);
 assert(!y_oak_post_step(&recovered,YOakPostRivalExited,YOakParcelAll,false,true));
 assert(y_oak_post_pack(&recovered,0xABCDEF01u,&r));
 assert(y_oak_post_unpack(&s,0xABCDEF01u,&r));
 assert(s.phase==YOakPostDone&&s.pokedex_owned&&s.route22_rival_enabled);
 puts("R014_POSTPARCEL_ORDERED_RIVAL_POKEDEX_ROUTE22_PASS");
}
static void corruption(void) {
 YOakPostState s={YOakPostWait,0,0};YOakPostRecord r;
 assert(y_oak_post_pack(&s,0x11223344u,&r));
 YOakPostState dst={YOakPostDone,1,1};
 assert(!y_oak_post_unpack(&dst,0x11223345u,&r));
 assert(dst.phase==YOakPostDone);
 r.checksum^=1u;
 assert(!y_oak_post_unpack(&dst,0x11223344u,&r));
 assert(dst.phase==YOakPostDone);
 r.checksum^=1u;r.phase=YOakPostRivalLeaving;
 assert(!y_oak_post_unpack(&dst,0x11223344u,&r));
 YOakPostState impossible={YOakPostWait,1,0};
 assert(!y_oak_post_valid(&impossible));
 assert(!y_oak_post_pack(&impossible,1u,&r));
 assert(!y_oak_post_step(&impossible,YOakPostOakDialogueFinished,YOakParcelAll,false,true));
 puts("R014_POSTPARCEL_CORRUPTION_NEGATIVE_CONTROL_PASS");
}
int main(void){progression();corruption();return 0;}
