#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "../yellow_viridian_mart_purchase.c"

static void encode(unsigned v,uint8_t b[3]) {
    for(int i=2;i>=0;i--) {
        unsigned pair=v%100u;
        b[i]=(uint8_t)(((pair/10u)<<4u)|(pair%10u));
        v/=100u;
    }
}
static void check(unsigned amount,uint8_t stock,uint8_t qty,bool delivered) {
    uint8_t before[3],after[3]={0xa5,0xa5,0xa5},expected[3];
    uint8_t after_stock=0xa5u;
    encode(amount,before);
    bool should=delivered && qty>0 && qty<=99u && stock<=99u &&
        (unsigned)stock+qty<=99u && amount>=100u*(unsigned)qty;
    bool ok=yellow_viridian_mart_buy_antidote(before,stock,delivered,qty,
                                               after,&after_stock);
    assert(ok==should);
    if(should) {
        encode(amount-100u*(unsigned)qty,expected);
        assert(memcmp(after,expected,3)==0);
        assert(after_stock==stock+qty);
    } else {
        assert(memcmp(after,(uint8_t[]){0xa5,0xa5,0xa5},3)==0);
        assert(after_stock==0xa5u);
    }
}
int main(void) {
    for(unsigned money=0;money<1000000u;money++) {
        check(money,0u,1u,true);
        check(money,98u,2u,true);
        check(money,0u,99u,true);
    }
    for(unsigned money=0;money<1000000u;money+=997u) {
        check(money,0u,1u,false);
        check(money,99u,1u,true);
        check(money,0u,0u,true);
        check(money,0u,100u,true);
    }
    uint8_t bad[3]={0x12,0xfa,0x56},out[3]={0xa5,0xa5,0xa5};
    uint8_t stock=0xa5;
    assert(!yellow_viridian_mart_buy_antidote(bad,0,true,1,out,&stock));
    assert(!memcmp(out,(uint8_t[]){0xa5,0xa5,0xa5},3) && stock==0xa5);
    assert(!yellow_viridian_mart_buy_antidote(0,0,true,1,out,&stock));
    assert(!yellow_viridian_mart_buy_antidote(bad,0,true,1,0,&stock));
    assert(!yellow_viridian_mart_buy_antidote(bad,0,true,1,out,0));
    uint8_t alias[3]={0x00,0x01,0x00};
    assert(yellow_viridian_mart_buy_antidote(alias,0,true,1,alias,&stock));
    assert(!memcmp(alias,(uint8_t[]){0,0,0},3) && stock==1u);
    puts("R21_PREWORK_3M_EXHAUSTIVE_BCD_PURCHASE_CASES_PASS");
    return 0;
}
