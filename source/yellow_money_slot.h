#ifndef YELLOW_MONEY_SLOT_H
#define YELLOW_MONEY_SLOT_H
#include <stdbool.h>
#include <stdint.h>
#define YELLOW_START_MONEY 3000u
#define YELLOW_MAX_MONEY 999999u
#define YELLOW_ANTIDOTE_COST 100u
#define YELLOW_MONEY_SLOT_BYTES 28u
typedef struct {uint8_t wire[YELLOW_MONEY_SLOT_BYTES];} YellowMoneySlot;
typedef enum {YMoneyBuySuccess=0,YMoneyBuyNoFunds=1,YMoneyBuyBagFull=2,YMoneyBuyInvalid=3} YMoneyBuyResult;
bool yellow_money_slot_make(uint32_t money,uint32_t parent_checksum,uint32_t generation,YellowMoneySlot* out);
bool yellow_money_slot_restore(const YellowMoneySlot* in,uint32_t parent_checksum,uint32_t generation,uint32_t* money_out);
YMoneyBuyResult yellow_money_buy_antidote(uint32_t* wallet,uint8_t* stock);
#endif
