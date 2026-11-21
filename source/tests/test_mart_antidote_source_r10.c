#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_mart_antidote.h"
#include "yellow_core.h"
#include "../yellow_mart_antidote.c"
/* Pin: pret/pokeyellow/data/items/marts.asm and prices.asm. */
int main(void) {
    assert(YELLOW_ANTIDOTE_PRICE==100u);
    for(unsigned map=0;map<256;map++)
        for(unsigned parcel=0;parcel<2;parcel++) {
            bool expected=(map==YMapViridianMart && parcel) ||
                           map==YMapCeruleanMart;
            assert(yellow_mart_sells_antidote((uint8_t)map,parcel)==expected);
            uint32_t wallet=1000;uint8_t stock=2;
            YMartAntidoteResult rc=yellow_mart_buy_antidote(
               (uint8_t)map,parcel,&wallet,&stock);
            assert(rc==(expected?YMartAntidotePurchased:YMartAntidoteUnavailable));
            assert(wallet==(expected?900u:1000u));
            assert(stock==(expected?3u:2u));
        }
    uint32_t balance=99;uint8_t stock=2;
    assert(yellow_mart_buy_antidote(YMapCeruleanMart,true,&balance,&stock)
           ==YMartAntidoteInsufficientFunds);
    assert(balance==99 && stock==2);
    balance=100;stock=99;
    assert(yellow_mart_buy_antidote(YMapCeruleanMart,true,&balance,&stock)
           ==YMartAntidoteBagFull);
    assert(balance==100 && stock==99);
    balance=0;stock=100;
    assert(yellow_mart_buy_antidote(YMapCeruleanMart,true,&balance,&stock)
           ==YMartAntidoteInvalid);
    assert(balance==0 && stock==100);
    assert(yellow_mart_buy_antidote(YMapCeruleanMart,true,0,&stock)
           ==YMartAntidoteInvalid);
    assert(yellow_mart_buy_antidote(YMapCeruleanMart,true,&balance,0)
           ==YMartAntidoteInvalid);
    balance=10000;stock=0;
    unsigned purchased=0;
    while(yellow_mart_buy_antidote(YMapCeruleanMart,true,&balance,&stock)
          ==YMartAntidotePurchased)purchased++;
    assert(purchased==99 && stock==99 && balance==100);
    puts("MART_ANTIDOTE_SOURCE_CATALOG_512_AND_ATOMIC_WALLET_PASS");
    return 0;
}
