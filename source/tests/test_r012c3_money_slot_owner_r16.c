#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "../yellow_whiteout_money_slot.c"
#include "../yellow_whiteout_preflight.c"

static void encode(unsigned v,uint8_t b[3]) {
    for(int i=2;i>=0;i--) {
        unsigned pair=v%100u;
        b[i]=(uint8_t)(((pair/10u)<<4u)|(pair%10u));
        v/=100u;
    }
}
static void bad_inputs(void) {
    uint8_t good[3]={0x12,0x34,0x56}, bad[3]={0x00,0x1a,0x00};
    YellowWhiteoutMoneySlot s,untouched;
    memset(&untouched,0xA5,sizeof untouched);
    s=untouched;
    assert(!yellow_whiteout_money_slot_create(bad,1,1,&s));
    assert(!memcmp(&s,&untouched,sizeof s));
    assert(!yellow_whiteout_money_slot_create(good,0,1,&s));
    assert(!yellow_whiteout_money_slot_create(good,1,0,&s));
    assert(!yellow_whiteout_money_slot_create(0,1,1,&s));
    assert(!yellow_whiteout_money_slot_create(good,1,1,0));
    assert(yellow_whiteout_money_slot_create(good,0x12345678,42,&s));
    uint8_t out[3]={0xAA,0xBB,0xCC};
    uint32_t generation=0xABCDEF01u;
    assert(!yellow_whiteout_money_slot_restore(&s,2,out,&generation));
    assert(!yellow_whiteout_money_slot_half(&s,2,42,out));
    assert(!yellow_whiteout_money_slot_half(&s,0x12345678,41,out));
    assert(!yellow_whiteout_money_slot_restore(0,0x12345678,out,&generation));
    assert(!yellow_whiteout_money_slot_restore(&s,0x12345678,0,&generation));
    assert(!yellow_whiteout_money_slot_restore(&s,0x12345678,out,0));
    assert(!yellow_whiteout_money_slot_half(0,0x12345678,42,out));
    assert(!yellow_whiteout_money_slot_half(&s,0x12345678,42,0));
    assert(!memcmp(out,(uint8_t[]){0xAA,0xBB,0xCC},3));
    assert(generation==0xABCDEF01u);
    YellowWhiteoutMoneySlot broken=s;
    broken.money[1]=0x1Au; broken.checksum=slot_hash(&broken);
    assert(!yellow_whiteout_money_slot_restore(&broken,0x12345678,out,&generation));
    broken=s;broken.money[0]^=1u;
    assert(!yellow_whiteout_money_slot_half(&broken,0x12345678,42,out));
    broken=s;broken.checksum^=1u;
    assert(!yellow_whiteout_money_slot_restore(&broken,0x12345678,out,&generation));
    broken=s;broken.seal^=1u;
    assert(!yellow_whiteout_money_slot_half(&broken,0x12345678,42,out));
    broken=s;broken.version++;
    assert(!yellow_whiteout_money_slot_restore(&broken,0x12345678,out,&generation));
    broken=s;broken.generation=0u;broken.checksum=slot_hash(&broken);
    assert(!yellow_whiteout_money_slot_restore(&broken,0x12345678,out,&generation));
    assert(!memcmp(out,(uint8_t[]){0xAA,0xBB,0xCC},3));
    assert(generation==0xABCDEF01u);
}
int main(void) {
    bad_inputs();
    for(unsigned v=0;v<1000000u;v++) {
        uint8_t money[3],expected[3],half[3]={0},restored[3]={0};
        uint32_t generation=0;
        encode(v,money);encode(v/2u,expected);
        YellowWhiteoutMoneySlot slot;
        assert(yellow_whiteout_money_slot_create(money,0x10203040u,42u,&slot));
        assert(yellow_whiteout_money_slot_restore(&slot,0x10203040u,restored,&generation));
        assert(generation==42u && !memcmp(money,restored,3));
        assert(yellow_whiteout_money_slot_half(&slot,0x10203040u,42u,half));
        assert(!memcmp(expected,half,3));
        uint8_t reference[3]={0};
        assert(y_whiteout_half_money_bcd(money,reference));
        assert(!memcmp(reference,half,3));
        assert(yellow_whiteout_money_slot_half(&slot,0x10203040u,42u,half));
        assert(!memcmp(expected,half,3));
    }
    puts("R012C3_MONEY_SLOT_1000000_BCD_VALUES_AND_FAILURE_ATOMICITY_PASS_PREWORK");
    return 0;
}
