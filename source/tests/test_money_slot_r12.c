#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_money_slot.h"
#include "../yellow_money_slot.c"

int main(void) {
 assert(YELLOW_START_MONEY==3000u && YELLOW_ANTIDOTE_COST==100u);
 assert(sizeof(YellowMoneySlot)==28u);
 const uint32_t parent=0x7f125439u,generation=17u;
 YellowMoneySlot s,mut;
 uint32_t value=123456u;
 assert(!yellow_money_slot_make(1000000u,parent,generation,&s));
 assert(!yellow_money_slot_make(0u,0u,generation,&s));
 assert(!yellow_money_slot_make(0u,parent,0u,&s));
 assert(!yellow_money_slot_make(0u,parent,generation,0));
 assert(!yellow_money_slot_restore(0,parent,generation,&value));
 assert(!yellow_money_slot_restore(&s,0,generation,&value));
 assert(!yellow_money_slot_restore(&s,parent,0,&value));
 assert(!yellow_money_slot_restore(&s,parent,generation,0));
 unsigned valid=0,corruption=0;
 for(uint32_t money=0;money<=YELLOW_MAX_MONEY;money+=997u) {
  assert(yellow_money_slot_make(money,parent,generation,&s));
  value=0xffffffffu;
  assert(yellow_money_slot_restore(&s,parent,generation,&value));
  assert(value==money);
  valid++;
 }
 assert(yellow_money_slot_make(3000,parent,generation,&s));
 assert(!yellow_money_slot_restore(&s,parent+1u,generation,&value));
 assert(!yellow_money_slot_restore(&s,parent,generation+1u,&value));
 for(unsigned byte=0;byte<YELLOW_MONEY_SLOT_BYTES;byte++) {
  for(unsigned bit=0;bit<8;bit++) {
   mut=s;mut.wire[byte]^=(uint8_t)(1u<<bit);
   value=7777;
   assert(!yellow_money_slot_restore(&mut,parent,generation,&value));
   assert(value==7777u);
   corruption++;
  }
 }
 unsigned purchases=0,rejections=0;
 for(unsigned cash=0;cash<=2000;cash+=37)
 for(unsigned stock=0;stock<=100;stock++) {
  uint32_t wallet=cash;uint8_t n=(uint8_t)stock;
  YMoneyBuyResult result=yellow_money_buy_antidote(&wallet,&n);
  if(stock==100u)assert(result==YMoneyBuyInvalid);
  else if(stock==99u)assert(result==YMoneyBuyBagFull);
  else if(cash<100u)assert(result==YMoneyBuyNoFunds);
  else assert(result==YMoneyBuySuccess);
  if(result==YMoneyBuySuccess){
   assert(wallet==cash-100u && n==stock+1u);purchases++;
  }else{
   assert(wallet==cash && n==stock);rejections++;
  }
 }
 uint32_t wallet=YELLOW_START_MONEY;uint8_t stock=0;
 for(unsigned i=0;i<30;i++)
  assert(yellow_money_buy_antidote(&wallet,&stock)==YMoneyBuySuccess);
 assert(wallet==0u && stock==30u);
 assert(yellow_money_buy_antidote(&wallet,&stock)==YMoneyBuyNoFunds);
 assert(wallet==0u && stock==30u);
 assert(yellow_money_buy_antidote(0,&stock)==YMoneyBuyInvalid);
 assert(yellow_money_buy_antidote(&wallet,0)==YMoneyBuyInvalid);
 printf("R12_MONEY_CODEC_PASS valid=%u bitflip_reject=%u purchase=%u rejection=%u\n",valid,corruption,purchases,rejections);
 return 0;
}
