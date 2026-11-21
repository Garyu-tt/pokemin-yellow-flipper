#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../yellow_antidote_cure.c"
int main(void) {
    for(unsigned st=0;st<256;st++) {
        for(unsigned hp=0;hp<3;hp++) {
            for(unsigned stock=0;stock<3;stock++) {
                uint8_t result=0xA5, left=0x5A;
                bool expected=(st==YStatusPoisoned ||
                    (st>=YStatusToxic1 && st<=YStatusToxic15)) && hp && stock;
                bool ok=yellow_antidote_cure((uint8_t)st,(uint8_t)hp,
                                               (uint8_t)stock,&result,&left);
                assert(ok==expected);
                if(expected){assert(result==YStatusNone && left==stock-1u);}
                else{assert(result==0xA5 && left==0x5A);}
            }
        }
    }
    uint8_t a=0xA5,b=0x5A;
    assert(!yellow_antidote_cure(YStatusPoisoned,1,1,0,&b));
    assert(!yellow_antidote_cure(YStatusPoisoned,1,1,&a,0));
    assert(!yellow_antidote_cure(YStatusPoisoned,1,1,&a,&a));
    assert(a==0xA5 && b==0x5A);
    a=99;b=99;
    assert(yellow_antidote_cure(YStatusToxic15,1,255,&a,&b));
    assert(a==YStatusNone && b==254);
    puts("R012C3_ANTIDOTE_2304_STATES_ATOMIC_NO_EFFECT_AND_TOXIC_PASS_PREWORK");
    return 0;
}
