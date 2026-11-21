#ifndef YELLOW_ANTIDOTE_CURE_H
#define YELLOW_ANTIDOTE_CURE_H
#include <stdbool.h>
#include <stdint.h>
/* R0.12-C3 source-backed item effect, PREWORK only. Not yet wired to bag UI
 * or inventory persistence. The input status is the native YStatus encoding,
 * not the original Gen1 bitmask. Source: pret/pokeyellow
 * engine/items/item_effects.asm ItemUseMedicine .checkMonStatus. */
bool yellow_antidote_cure(uint8_t status, uint8_t hp, uint8_t stock,
                         uint8_t* out_status, uint8_t* out_stock);
#endif
