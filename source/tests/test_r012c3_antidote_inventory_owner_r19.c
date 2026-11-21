#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../yellow_antidote_inventory_slot.c"
int main(void) {
    assert(sizeof(YellowAntidoteInventorySlot)==24u);
    YellowAntidoteInventorySlot s;memset(&s,0xA5,sizeof s);
    YellowAntidoteInventorySlot old=s;
    assert(!yellow_antidote_inventory_create(100,1,1,&s));
    assert(!yellow_antidote_inventory_create(1,0,1,&s));
    assert(!yellow_antidote_inventory_create(1,1,0,&s));
    assert(!yellow_antidote_inventory_create(1,1,1,NULL));
    assert(!memcmp(&s,&old,sizeof s));
    for(unsigned i=0;i<256;i++) {
        if(i>99){assert(!yellow_antidote_inventory_create((uint8_t)i,17,3,&s));continue;}
        assert(yellow_antidote_inventory_create((uint8_t)i,17,3,&s));
        uint8_t stock=0xEE;
        assert(yellow_antidote_inventory_restore(&s,17,3,&stock));
        assert(stock==i);
        stock=0xEE;assert(!yellow_antidote_inventory_restore(&s,18,3,&stock));
        assert(!yellow_antidote_inventory_restore(&s,17,4,&stock));
        assert(!yellow_antidote_inventory_restore(&s,17,0,&stock));
        assert(!yellow_antidote_inventory_restore(&s,0,3,&stock));
        assert(!yellow_antidote_inventory_restore(NULL,17,3,&stock));
        assert(!yellow_antidote_inventory_restore(&s,17,3,NULL));
        assert(stock==0xEE);
        for(unsigned j=0;j<24;j++) {
            YellowAntidoteInventorySlot corrupt=s;corrupt.wire[j]^=1u;
            assert(!yellow_antidote_inventory_restore(&corrupt,17,3,&stock));
            assert(stock==0xEE);
        }
    }
    puts("R012C3_ANTIDOTE_INVENTORY_100_STOCKS_2400_BYTE_MUTATIONS_PASS_PREWORK");
    return 0;
}
