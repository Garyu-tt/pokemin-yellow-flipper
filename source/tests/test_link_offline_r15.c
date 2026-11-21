#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"

static void setup(YellowGame* g) {
    yellow_init(g);
    g->story=YStoryPikachuFollows;
    g->has_pikachu=1;
    g->screen=YScreenWorld;
    g->map=YMapRoute1;
    g->x=10;g->y=28;
    g->hp=yellow_player_max_hp(g);
    g->follower_valid=0;
}
static void actual_input_loopback(void) {
    YellowGame g;setup(&g);
    uint32_t rng=g.rng;
    uint8_t hp=g.hp,pp=g.pika_pp[0],potions=g.potions,balls=g.balls;
    YellowSave before,after;
    yellow_save_create(&g,&before);
    yellow_input(&g,YBack);
    assert(g.screen==YScreenMenu && g.selected==0);
    for(int i=0;i<4;i++)yellow_input(&g,YDown);
    assert(g.screen==YScreenMenu && g.selected==4);
    yellow_input(&g,YDown);assert(g.selected==4);
    yellow_input(&g,YOk);
    assert(g.screen==YScreenLinkOffline && g.link_result==0);
    yellow_input(&g,YOk);
    assert(g.link_result==1 && g.link_sequence==1);
    assert(g.hp==hp && g.pika_pp[0]==pp && g.potions==potions);
    assert(g.balls==balls && g.rng==rng);
    yellow_input(&g,YOk);
    assert(g.link_result==1 && g.link_sequence==2);
    yellow_save_create(&g,&after);
    assert(memcmp(&before,&after,sizeof(before))==0);
    YellowGame loaded;
    assert(yellow_save_load(&loaded,&after));
    assert(loaded.link_sequence==0 && loaded.link_result==0);
    yellow_input(&g,YBack);
    assert(g.screen==YScreenMenu && g.selected==4);
    yellow_input(&g,YBack);
    assert(g.screen==YScreenWorld);
    puts("R15_LINK_REAL_YELLOW_INPUT_LOOPBACK_PASS");
}
static void packet_codec_and_reject(void) {
    YellowLinkSnapshot a={.sequence=0xabcd,.kind=1,.species=YK_PIKACHU,
        .level=5,.hp=17,.status=YStatusParalyzed};
    uint8_t p[YELLOW_LINK_PACKET_SIZE],copy[YELLOW_LINK_PACKET_SIZE];
    assert(yellow_link_encode(&a,p));
    assert(p[0]=='Y' && p[1]=='L' && p[2]==1 && p[3]==1);
    assert(p[4]==0xcd && p[5]==0xab);
    YellowLinkSnapshot b={0};
    assert(yellow_link_decode(p,sizeof(p),&b));
    assert(b.sequence==a.sequence && b.kind==1 && b.species==a.species);
    assert(b.level==a.level && b.hp==a.hp && b.status==a.status);
    memcpy(copy,p,sizeof(p));
    b.sequence=0x9999;
    p[8]^=1;
    assert(!yellow_link_decode(p,sizeof(p),&b) && b.sequence==0x9999);
    memcpy(p,copy,sizeof(p));
    p[2]=2;
    assert(!yellow_link_decode(p,sizeof(p),&b));
    memcpy(p,copy,sizeof(p));
    p[3]=2;
    assert(!yellow_link_decode(p,sizeof(p),&b));
    assert(!yellow_link_decode(copy,11,&b));
    assert(!yellow_link_decode(NULL,12,&b));
    assert(!yellow_link_decode(copy,12,NULL));
    memcpy(p,copy,sizeof(p));
    p[6]=0xff;
    uint16_t crc=yellow_link_crc16(p,10);
    p[10]=(uint8_t)crc;p[11]=(uint8_t)(crc>>8);
    assert(!yellow_link_decode(p,sizeof(p),&b));
    memcpy(p,copy,sizeof(p));
    p[9]=27;
    crc=yellow_link_crc16(p,10);
    p[10]=(uint8_t)crc;p[11]=(uint8_t)(crc>>8);
    assert(!yellow_link_decode(p,sizeof(p),&b));
    a.level=0;
    memset(p,0xa5,sizeof(p));
    assert(!yellow_link_encode(&a,p));
    for(size_t i=0;i<sizeof(p);i++)assert(p[i]==0xa5);
    a.level=5;a.hp=255;
    assert(!yellow_link_encode(&a,p));
    assert(!yellow_link_encode(NULL,p));
    assert(!yellow_link_encode(&a,NULL));
    YellowLinkSnapshot hello={.sequence=44,.kind=0};
    assert(yellow_link_encode(&hello,p) && p[3]==0);
    assert(yellow_link_decode(p,sizeof(p),&b));
    assert(b.kind==0 && b.sequence==44 && b.species==0);
    hello.hp=1;
    assert(!yellow_link_encode(&hello,p));
    puts("R15_LINK_CRC_VERSION_LENGTH_PAYLOAD_NEGATIVE_PASS");
}
static void missing_party_and_wrap(void) {
    YellowGame g;setup(&g);
    g.screen=YScreenLinkOffline;
    g.link_sequence=65535;
    yellow_input(&g,YOk);
    assert(g.link_sequence==0 && g.link_result==1);
    g.has_pikachu=0;
    yellow_input(&g,YOk);
    assert(g.link_result==1 && g.link_sequence==1); /* HELLO without Pokemon */
    g.has_pikachu=1;g.active_party=9;
    yellow_input(&g,YOk);
    assert(g.link_result==2 && g.link_sequence==1);
    puts("R15_LINK_VOLATILE_FAILURE_AND_WRAP_PASS");
}
int main(void) {
    actual_input_loopback();
    packet_codec_and_reject();
    missing_party_and_wrap();
    puts("R15_LINK_OFFLINE_ALL_PASS");
    return 0;
}
