#include "yellow_money_stock_slot.h"
#include <string.h>
#define MONEY_STOCK_MAGIC 0x31534d59u /* YMS1 */
#define MONEY_STOCK_SEAL  0x53534d59u /* YMSS */
static void put32(uint8_t* p,uint32_t n) {
    for(unsigned i=0;i<4u;i++)p[i]=(uint8_t)(n>>(i*8u));
}
static uint32_t get32(const uint8_t* p) {
    uint32_t n=0u;
    for(unsigned i=0;i<4u;i++)n|=(uint32_t)p[i]<<(i*8u);
    return n;
}
static uint32_t hash20(const uint8_t* p) {
    uint32_t h=2166136261u;
    for(unsigned i=0;i<20u;i++)h=(h^p[i])*16777619u;
    return h;
}
static bool valid_bcd(const uint8_t* money) {
    if(!money)return false;
    for(unsigned i=0;i<3u;i++)
        if((money[i]>>4u)>9u || (money[i]&15u)>9u)return false;
    return true;
}
bool yellow_money_stock_create(const uint8_t money_bcd[3],uint8_t stock,
    uint32_t parent,uint32_t generation,YellowMoneyStockSlot* out) {
    if(!out || !parent || !generation || stock>99u || !valid_bcd(money_bcd))
        return false;
    YellowMoneyStockSlot tmp={{0}};
    put32(tmp.wire,MONEY_STOCK_MAGIC);
    tmp.wire[4]=1u;
    memcpy(tmp.wire+6,money_bcd,3u);
    tmp.wire[9]=stock;
    put32(tmp.wire+12,parent);
    put32(tmp.wire+16,generation);
    put32(tmp.wire+20,hash20(tmp.wire));
    put32(tmp.wire+24,MONEY_STOCK_SEAL);
    *out=tmp;
    return true;
}
bool yellow_money_stock_restore(const YellowMoneyStockSlot* slot,
    uint32_t parent,uint32_t generation,uint8_t money_bcd[3],uint8_t* stock) {
    if(!slot || !parent || !generation || !money_bcd || !stock)return false;
    const uint8_t* p=slot->wire;
    if(get32(p)!=MONEY_STOCK_MAGIC || p[4]!=1u || p[5]!=0u ||
       p[10]!=0u || p[11]!=0u || p[9]>99u ||
       get32(p+12)!=parent || get32(p+16)!=generation ||
       !valid_bcd(p+6) || get32(p+20)!=hash20(p) ||
       get32(p+24)!=MONEY_STOCK_SEAL)return false;
    uint8_t tmp[3];
    memcpy(tmp,p+6,3u);
    const uint8_t count=p[9];
    memcpy(money_bcd,tmp,3u);
    *stock=count;
    return true;
}
