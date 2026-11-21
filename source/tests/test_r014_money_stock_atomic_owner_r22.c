#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../yellow_money_stock_slot.c"
static void bcd(unsigned n,uint8_t b[3]) {
    for(int i=2;i>=0;i--) {
        unsigned x=n%100u;
        b[i]=(uint8_t)(((x/10u)<<4u)|(x%10u));
        n/=100u;
    }
}
int main(void) {
    const uint32_t parent=0x53ac1e8bu,generation=0xfffffffeu;
    YellowMoneyStockSlot s={{0}},sentinel;
    uint8_t m[3],got[3],stock=0;
    for(unsigned n=0;n<1000000u;n+=97u) {
        bcd(n,m);
        for(unsigned count=0;count<100u;count++) {
            assert(yellow_money_stock_create(m,(uint8_t)count,parent,generation,&s));
            memset(got,0xa5,3);stock=0xa5;
            assert(yellow_money_stock_restore(&s,parent,generation,got,&stock));
            assert(memcmp(m,got,3)==0 && stock==count);
        }
    }
    bcd(3000u,m);
    assert(yellow_money_stock_create(m,2u,parent,generation,&s));
    for(unsigned byte=0;byte<28u;byte++) {
        YellowMoneyStockSlot bad=s;
        bad.wire[byte]^=0x01u;
        memset(got,0xa5,3);stock=0xa5;
        assert(!yellow_money_stock_restore(&bad,parent,generation,got,&stock));
        assert(memcmp(got,(uint8_t[]){0xa5,0xa5,0xa5},3)==0 && stock==0xa5);
    }
    memset(got,0xa5,3);stock=0xa5;
    assert(!yellow_money_stock_restore(&s,parent,generation+1u,got,&stock));
    assert(!yellow_money_stock_restore(&s,parent+1u,generation,got,&stock));
    assert(!yellow_money_stock_restore(&s,0u,generation,got,&stock));
    assert(!yellow_money_stock_restore(&s,parent,0u,got,&stock));
    assert(!yellow_money_stock_restore(&s,parent,generation,NULL,&stock));
    assert(!yellow_money_stock_restore(&s,parent,generation,got,NULL));
    assert(!yellow_money_stock_restore(NULL,parent,generation,got,&stock));
    assert(memcmp(got,(uint8_t[]){0xa5,0xa5,0xa5},3)==0 && stock==0xa5);
    sentinel=s;
    uint8_t invalid[3]={0x00,0xfa,0x00};
    assert(!yellow_money_stock_create(invalid,2,parent,generation,&s));
    assert(!yellow_money_stock_create(m,100,parent,generation,&s));
    assert(!yellow_money_stock_create(m,2,0,generation,&s));
    assert(!yellow_money_stock_create(m,2,parent,0,&s));
    assert(!yellow_money_stock_create(NULL,2,parent,generation,&s));
    assert(!yellow_money_stock_create(m,2,parent,generation,NULL));
    assert(memcmp(&sentinel,&s,sizeof(s))==0);
    puts("R22_MONEY_STOCK_1M_ROUNDTRIPS_AND_MUTATIONS_PASS");
    return 0;
}
