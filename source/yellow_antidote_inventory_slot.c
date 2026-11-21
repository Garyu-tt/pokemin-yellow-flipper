#include "yellow_antidote_inventory_slot.h"
#include <string.h>
#define MAGIC 0x59414931u /* YAI1 */
#define SEAL 0x59414953u /* YAIS */
static void put32(uint8_t* p,uint32_t n) {
    for(unsigned i=0;i<4;i++)p[i]=(uint8_t)(n>>(8u*i));
}
static uint32_t get32(const uint8_t* p) {
    uint32_t n=0;for(unsigned i=0;i<4;i++)n|=(uint32_t)p[i]<<(8u*i);return n;
}
static uint32_t hash16(const uint8_t* p) {
    uint32_t h=2166136261u;for(unsigned i=0;i<16;i++)h=(h^p[i])*16777619u;return h;
}
bool yellow_antidote_inventory_create(uint8_t stock,uint32_t parent,
    uint32_t generation,YellowAntidoteInventorySlot* out) {
    if(!out || stock>99u || !parent || !generation)return false;
    YellowAntidoteInventorySlot temp={{0}};
    put32(temp.wire,MAGIC);temp.wire[4]=1u;temp.wire[6]=stock;
    put32(temp.wire+8,parent);put32(temp.wire+12,generation);
    put32(temp.wire+16,hash16(temp.wire));put32(temp.wire+20,SEAL);
    *out=temp;return true;
}
bool yellow_antidote_inventory_restore(const YellowAntidoteInventorySlot* slot,
    uint32_t parent,uint32_t generation,uint8_t* stock_out) {
    if(!slot || !stock_out || !parent || !generation)return false;
    const uint8_t* p=slot->wire;
    if(get32(p)!=MAGIC || p[4]!=1u || p[5]!=0u || p[6]>99u || p[7]!=0u ||
       get32(p+8)!=parent || get32(p+12)!=generation ||
       get32(p+16)!=hash16(p) || get32(p+20)!=SEAL)return false;
    *stock_out=p[6];return true;
}
