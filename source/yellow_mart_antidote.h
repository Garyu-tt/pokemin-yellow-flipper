#ifndef YELLOW_MART_ANTIDOTE_H
#define YELLOW_MART_ANTIDOTE_H
#include <stdint.h>
#include <stdbool.h>
/* Verified upstream pret/pokeyellow data/items/marts.asm + prices.asm.
 * Strictly an isolated domain helper, not yet attached to the game UI.
 * The caller must supply a PERSISTENT wallet before enabling in-game buying.
 */
#define YELLOW_ANTIDOTE_PRICE 100u
typedef enum {
    YMartAntidotePurchased=0,
    YMartAntidoteUnavailable=1,
    YMartAntidoteInsufficientFunds=2,
    YMartAntidoteBagFull=3,
    YMartAntidoteInvalid=4
} YMartAntidoteResult;
bool yellow_mart_sells_antidote(uint8_t map, bool oak_parcel_delivered);
YMartAntidoteResult yellow_mart_buy_antidote(
    uint8_t map,bool oak_parcel_delivered,
    uint32_t* wallet,uint8_t* antidotes);
#endif
