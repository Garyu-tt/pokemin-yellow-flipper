#include "yellow_money_slot.h"
/* Versioned money sidecar, tied to atomic snapshot generation and parent.
 * 28 bytes, checksum is FNV-1a for error detection, not authentication. */
#define MONEY_MAGIC 0x314f4d59u
#define MONEY_SEAL 0x534f4d59u
static void put32(uint8_t* p,uint32_t v) {
 for(unsigned i=0;i<4;i++)p[i]=(uint8_t)(v>>(8u*i));
}
static uint32_t get32(const uint8_t* p) {
 uint32_t n=0;
 for(unsigned i=0;i<4;i++)n|=(uint32_t)p[i]<<(8u*i);
 return n;
}
static uint32_t sum20(const uint8_t* p) {
 uint32_t h=2166136261u;
 for(unsigned i=0;i<20;i++)h=(h^p[i])*16777619u;
 return h;
}
bool yellow_money_slot_make(uint32_t money,uint32_t parent_checksum,
                            uint32_t generation,YellowMoneySlot* out) {
 if(!out || money>YELLOW_MAX_MONEY || !parent_checksum || !generation)return false;
 YellowMoneySlot r={{0}};
 put32(r.wire,MONEY_MAGIC);r.wire[4]=1u;
 put32(r.wire+8,money);put32(r.wire+12,parent_checksum);
 put32(r.wire+16,generation);put32(r.wire+20,sum20(r.wire));
 put32(r.wire+24,MONEY_SEAL);*out=r;return true;
}
bool yellow_money_slot_restore(const YellowMoneySlot* input,
 uint32_t parent_checksum,uint32_t generation,uint32_t* money_out) {
 if(!input || !money_out || !parent_checksum || !generation)return false;
 const uint8_t* p=input->wire;
 if(get32(p)!=MONEY_MAGIC || p[4]!=1u || p[5] || p[6] || p[7] ||
    get32(p+8)>YELLOW_MAX_MONEY ||
    get32(p+12)!=parent_checksum || get32(p+16)!=generation ||
    get32(p+20)!=sum20(p) || get32(p+24)!=MONEY_SEAL)return false;
 *money_out=get32(p+8);return true;
}
YMoneyBuyResult yellow_money_buy_antidote(uint32_t* wallet,uint8_t* stock) {
 if(!wallet || !stock || *wallet>YELLOW_MAX_MONEY || *stock>99u)
  return YMoneyBuyInvalid;
 if(*stock==99u)return YMoneyBuyBagFull;
 if(*wallet<YELLOW_ANTIDOTE_COST)return YMoneyBuyNoFunds;
 *wallet-=YELLOW_ANTIDOTE_COST; ++*stock;return YMoneyBuySuccess;
}
