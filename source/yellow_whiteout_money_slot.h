#ifndef YELLOW_WHITEOUT_MONEY_SLOT_H
#define YELLOW_WHITEOUT_MONEY_SLOT_H
#include <stdbool.h>
#include <stdint.h>

/* R0.12-C3 isolated PREWORK: money is NOT yet connected to YellowGame.
 * Source: pret/pokeyellow engine/events/black_out.asm.
 * No YellowSave v13 ABI changes, no automatic whiteout claim. */
#define YELLOW_WHITEOUT_MONEY_MAGIC 0x59574D31u
#define YELLOW_WHITEOUT_MONEY_SEAL  0x59574D4Du
#define YELLOW_WHITEOUT_MONEY_VERSION 1u

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint8_t money[3]; /* six packed decimal digits, most significant first */
    uint32_t parent_checksum;
    uint32_t generation;
    uint32_t checksum;
    uint32_t seal;
} YellowWhiteoutMoneySlot;

bool yellow_whiteout_money_slot_create(const uint8_t money_bcd[3],
    uint32_t parent_checksum, uint32_t generation, YellowWhiteoutMoneySlot* out);
bool yellow_whiteout_money_slot_restore(const YellowWhiteoutMoneySlot* slot,
    uint32_t parent_checksum, uint8_t money_out[3], uint32_t* out_generation);
bool yellow_whiteout_money_slot_half(const YellowWhiteoutMoneySlot* slot,
    uint32_t parent_checksum, uint32_t generation, uint8_t money_out[3]);
#endif
