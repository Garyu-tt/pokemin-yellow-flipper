#include "yellow_core.h"
#include "yellow_mart_antidote.h"
bool yellow_mart_sells_antidote(uint8_t map,bool oak_parcel_delivered) {
    /* Viridian clerk sells items only after Oak's parcel is returned;
     * Cerulean mart has Antidote in its original sourced inventory. */
    return (map==YMapViridianMart && oak_parcel_delivered) ||
           map==YMapCeruleanMart;
}
YMartAntidoteResult yellow_mart_buy_antidote(
    uint8_t map,bool oak_parcel_delivered,
    uint32_t* wallet,uint8_t* antidotes) {
    if(!wallet || !antidotes || *antidotes>99u)
        return YMartAntidoteInvalid;
    if(!yellow_mart_sells_antidote(map,oak_parcel_delivered))
        return YMartAntidoteUnavailable;
    if(*antidotes==99u)return YMartAntidoteBagFull;
    if(*wallet<YELLOW_ANTIDOTE_PRICE)return YMartAntidoteInsufficientFunds;
    /* No intermediate mutation: either both values change, or neither. */
    *wallet-=YELLOW_ANTIDOTE_PRICE;
    ++*antidotes;
    return YMartAntidotePurchased;
}
