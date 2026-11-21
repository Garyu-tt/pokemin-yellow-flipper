#ifndef YELLOW_VIRIDIAN_MART_PURCHASE_H
#define YELLOW_VIRIDIAN_MART_PURCHASE_H
#include <stdbool.h>
#include <stdint.h>

/* R21 prework only: source-pinned Viridian Mart ANTIDOTE transaction.
 * Does not change YellowSave v13, persist funds, or enable shop gameplay.
 * Caller MUST pass EVENT_OAK_GOT_PARCEL, not EVENT_GOT_OAKS_PARCEL.
 * On failure outputs are untouched; aliasing money input/output is allowed. */
#define YELLOW_VIRIDIAN_ANTIDOTE_PRICE 100u
bool yellow_viridian_mart_buy_antidote(const uint8_t money_bcd[3],
    uint8_t stock, bool oak_got_parcel, uint8_t quantity,
    uint8_t next_money_bcd[3], uint8_t* next_stock);
/* Pure sale: no UI or save side effects, output unchanged on failure. */
bool yellow_viridian_mart_sell_antidote(const uint8_t money_bcd[3],
    uint8_t stock, uint8_t quantity, uint8_t next_money_bcd[3],
    uint8_t* next_stock);
#endif
