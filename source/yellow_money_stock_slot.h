#ifndef YELLOW_MONEY_STOCK_SLOT_H
#define YELLOW_MONEY_STOCK_SLOT_H
#include <stdbool.h>
#include <stdint.h>
/* R22: fixed 28-byte LE sidecar for money + Antidote stock in ONE record.
 * Bound to the immutable YellowAtomicSnapshot parent checksum + generation.
 * The parent is sealed only AFTER the sidecar has been written/read back.
 * This is not the legacy YellowSave v13 wire format. */
typedef struct { uint8_t wire[28]; } YellowMoneyStockSlot;
bool yellow_money_stock_create(const uint8_t money_bcd[3],uint8_t stock,
    uint32_t parent,uint32_t generation,YellowMoneyStockSlot* out);
bool yellow_money_stock_restore(const YellowMoneyStockSlot* slot,
    uint32_t parent,uint32_t generation,uint8_t money_bcd[3],uint8_t* stock);
#endif
