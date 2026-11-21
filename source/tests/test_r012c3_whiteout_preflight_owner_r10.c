#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../yellow_whiteout_preflight.c"

static void source_gate(void) {
    YWhiteoutPreflight p={0xCCu,false,true};
    assert(!y_whiteout_preflight(0x29u,1u,true,false,false,&p));
    assert(p.original_last_blackout_map==0xCCu && p.executable);
    assert(!y_whiteout_preflight(1u,0u,true,false,false,&p));
    assert(!y_whiteout_preflight(1u,1u,false,false,false,&p));
    assert(!y_whiteout_preflight(1u,1u,true,true,false,&p));
    assert(!y_whiteout_preflight(1u,1u,true,false,true,&p));
    assert(y_whiteout_preflight(1u,1u,true,false,false,&p));
    assert(p.original_last_blackout_map==1u);
    assert(p.requires_verified_warp_anchor && !p.executable);
    puts("SOURCE_BLACKOUT_MAP_AND_FAIL_CLOSED_GATES_PASS");
}
static void money(void) {
    static const struct {uint8_t in[3],out[3];} vectors[]={
        {{0,0,0},{0,0,0}},
        {{0,0,1},{0,0,0}},
        {{0,0,5},{0,0,2}},
        {{0,0,0x99},{0,0,0x49}},
        {{0x99,0x99,0x99},{0x49,0x99,0x99}},
        {{0x12,0x34,0x56},{0x06,0x17,0x28}},
    };
    for(unsigned i=0;i<sizeof(vectors)/sizeof(vectors[0]);i++) {
        uint8_t out[3]={0xCC,0xCC,0xCC};
        assert(y_whiteout_half_money_bcd(vectors[i].in,out));
        assert(memcmp(out,vectors[i].out,3)==0);
    }
    uint8_t bad[3]={0,0xFA,0},out[3]={1,2,3};
    assert(!y_whiteout_half_money_bcd(bad,out));
    assert(out[0]==1&&out[1]==2&&out[2]==3);
    assert(!y_whiteout_half_money_bcd(0,out));
    assert(!y_whiteout_half_money_bcd(bad,0));
    puts("GEN1_THREE_BYTE_BCD_HALVING_POSITIVE_NEGATIVE_PASS");
}
int main(void){source_gate();money();puts("R012C3_WHITEOUT_SOURCE_PREFLIGHT_ONLY_NOT_RECOVERY");return 0;}
