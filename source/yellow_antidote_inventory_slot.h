#ifndef YELLOW_ANTIDOTE_INVENTORY_SLOT_H
#define YELLOW_ANTIDOTE_INVENTORY_SLOT_H
#include <stdint.h>
#include <stdbool.h>
/* R19 independent PREWORK, not yet connected to runtime save/load.
 * Fixed 24-byte LE wire format, no C struct padding or YellowSave ABI change. */
typedef struct { uint8_t wire[24]; } YellowAntidoteInventorySlot;
bool yellow_antidote_inventory_create(uint8_t stock,uint32_t parent,
    uint32_t generation,YellowAntidoteInventorySlot* out);
bool yellow_antidote_inventory_restore(const YellowAntidoteInventorySlot* slot,
    uint32_t parent,uint32_t generation,uint8_t* stock_out);
#endif
