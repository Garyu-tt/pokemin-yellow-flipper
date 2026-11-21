#include "yellow_viridian_mart_purchase.h"
#include <string.h>

static bool money_decode(const uint8_t bcd[3], uint32_t* value) {
    if(!bcd || !value) return false;
    uint32_t amount=0u;
    for(unsigned i=0;i<3u;i++) {
        uint8_t hi=(uint8_t)(bcd[i]>>4u),lo=(uint8_t)(bcd[i]&15u);
        if(hi>9u || lo>9u) return false;
        amount=amount*100u+(uint32_t)hi*10u+(uint32_t)lo;
    }
    *value=amount;
    return true;
}
static void money_encode(uint32_t value,uint8_t bcd[3]) {
    for(int i=2;i>=0;i--) {
        uint32_t pair=value%100u;
        bcd[i]=(uint8_t)(((pair/10u)<<4u)|(pair%10u));
        value/=100u;
    }
}
bool yellow_viridian_mart_buy_antidote(const uint8_t money_bcd[3],
    uint8_t stock, bool oak_got_parcel, uint8_t quantity,
    uint8_t next_money_bcd[3], uint8_t* next_stock) {
    if(!next_money_bcd || !next_stock || !oak_got_parcel ||
       !quantity || quantity>99u || stock>99u ||
       (unsigned)quantity>99u-(unsigned)stock) return false;
    uint32_t funds;
    if(!money_decode(money_bcd,&funds)) return false;
    const uint32_t price=(uint32_t)quantity*YELLOW_VIRIDIAN_ANTIDOTE_PRICE;
    if(funds<price) return false;
    uint8_t encoded[3];
    money_encode(funds-price,encoded);
    const uint8_t updated_stock=(uint8_t)(stock+quantity);
    memcpy(next_money_bcd,encoded,sizeof encoded);
    *next_stock=updated_stock;
    return true;
}

/* Gen1 shop sell price is half the listed purchase price. Transaction only. */
bool yellow_viridian_mart_sell_antidote(const uint8_t money_bcd[3],
    uint8_t stock, uint8_t quantity, uint8_t next_money_bcd[3],
    uint8_t* next_stock) {
    if(!next_money_bcd || !next_stock || !quantity || stock>99u || quantity>stock)
        return false;
    uint32_t funds;
    if(!money_decode(money_bcd,&funds))return false;
    const uint32_t proceeds=(uint32_t)quantity*(YELLOW_VIRIDIAN_ANTIDOTE_PRICE/2u);
    if(funds>999999u-proceeds)return false;
    uint8_t encoded[3];
    money_encode(funds+proceeds,encoded);
    memcpy(next_money_bcd,encoded,sizeof encoded);
    *next_stock=(uint8_t)(stock-quantity);
    return true;
}
