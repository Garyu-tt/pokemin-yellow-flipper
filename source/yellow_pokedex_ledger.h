#ifndef YELLOW_POKEDEX_LEDGER_H
#define YELLOW_POKEDEX_LEDGER_H
#include <stdint.h>
#include <stdbool.h>
#define YELLOW_DEX_SPECIES 151u
#define YELLOW_DEX_BYTES 19u
#define YELLOW_DEX_SAVE_MAGIC 0x58444559u /* YEDX */
#define YELLOW_DEX_SAVE_VERSION 1u
typedef struct {
    uint8_t seen[YELLOW_DEX_BYTES],caught[YELLOW_DEX_BYTES];
} YellowDex;
typedef struct {
    uint32_t magic;
    uint16_t version;
    uint8_t seen[YELLOW_DEX_BYTES],caught[YELLOW_DEX_BYTES];
    uint32_t parent_checksum;
    uint32_t checksum;
} YellowDexSave;
uint8_t yellow_dex_number(uint8_t native_species);
const char* yellow_dex_name(uint8_t national_id);
bool yellow_dex_mark_seen(YellowDex* dex,uint8_t number);
bool yellow_dex_mark_caught(YellowDex* dex,uint8_t number);
bool yellow_dex_seen(const YellowDex* dex,uint8_t number);
bool yellow_dex_caught(const YellowDex* dex,uint8_t number);
uint16_t yellow_dex_seen_count(const YellowDex* dex);
uint16_t yellow_dex_caught_count(const YellowDex* dex);
bool yellow_dex_valid(const YellowDex* dex);
bool yellow_dex_save(const YellowDex* dex,uint32_t parent_checksum,YellowDexSave* out);
bool yellow_dex_restore(YellowDex* dex,uint32_t parent_checksum,const YellowDexSave* source);
#endif
