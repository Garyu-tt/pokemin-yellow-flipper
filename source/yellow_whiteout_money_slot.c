#include "yellow_whiteout_money_slot.h"
#include <string.h>

/* Do not hash raw C struct bytes: padding makes the on-disk checksum
 * dependent on compiler/ABI and can hide corrupt fields. */
static uint32_t hash_byte(uint32_t h, uint8_t x) {
    return (h ^ x) * 16777619u;
}
static uint32_t hash_u32(uint32_t h, uint32_t x) {
    for(unsigned i=0;i<4u;i++)h=hash_byte(h,(uint8_t)(x>>(i*8u)));
    return h;
}
static uint32_t slot_hash(const YellowWhiteoutMoneySlot* s) {
    uint32_t h=2166136261u;
    h=hash_u32(h,s->magic);
    h=hash_byte(h,(uint8_t)s->version);
    h=hash_byte(h,(uint8_t)(s->version>>8u));
    for(unsigned i=0;i<3u;i++)h=hash_byte(h,s->money[i]);
    h=hash_u32(h,s->parent_checksum);
    return hash_u32(h,s->generation);
}
static bool valid_bcd(const uint8_t* b) {
    if(!b)return false;
    for(unsigned i=0;i<3u;i++)
        if((b[i]>>4u)>9u || (b[i]&15u)>9u)return false;
    return true;
}
static bool valid_slot(const YellowWhiteoutMoneySlot* s,uint32_t parent) {
    return s && parent && s->magic==YELLOW_WHITEOUT_MONEY_MAGIC &&
        s->version==YELLOW_WHITEOUT_MONEY_VERSION &&
        s->seal==YELLOW_WHITEOUT_MONEY_SEAL && s->generation!=0u &&
        s->parent_checksum==parent && valid_bcd(s->money) &&
        s->checksum==slot_hash(s);
}
bool yellow_whiteout_money_slot_create(const uint8_t money_bcd[3],
    uint32_t parent_checksum, uint32_t generation, YellowWhiteoutMoneySlot* out) {
    if(!out || !parent_checksum || !generation || !valid_bcd(money_bcd))
        return false;
    YellowWhiteoutMoneySlot tmp={0};
    tmp.magic=YELLOW_WHITEOUT_MONEY_MAGIC;
    tmp.version=YELLOW_WHITEOUT_MONEY_VERSION;
    memcpy(tmp.money,money_bcd,3u);
    tmp.parent_checksum=parent_checksum;
    tmp.generation=generation;
    tmp.checksum=slot_hash(&tmp);
    tmp.seal=YELLOW_WHITEOUT_MONEY_SEAL;
    *out=tmp;
    return true;
}
bool yellow_whiteout_money_slot_restore(const YellowWhiteoutMoneySlot* slot,
    uint32_t parent_checksum, uint8_t money_out[3], uint32_t* out_generation) {
    if(!money_out || !out_generation || !valid_slot(slot,parent_checksum))
        return false;
    uint8_t copy[3];
    memcpy(copy,slot->money,3u);
    uint32_t generation=slot->generation;
    memcpy(money_out,copy,3u);
    *out_generation=generation;
    return true;
}
bool yellow_whiteout_money_slot_half(const YellowWhiteoutMoneySlot* slot,
    uint32_t parent_checksum, uint32_t generation, uint8_t money_out[3]) {
    if(!money_out || !generation || !valid_slot(slot,parent_checksum) ||
        slot->generation!=generation)return false;
    /* Packed BCD != hexadecimal. 0x999999 represents 999999, not 10066329.
     * Source DivideBCDPredef3 rounds odd money down. */
    uint32_t decimal=0u;
    for(unsigned i=0;i<3u;i++)
        decimal=decimal*100u+(uint32_t)(slot->money[i]>>4u)*10u+
                (uint32_t)(slot->money[i]&15u);
    decimal/=2u;
    uint8_t half[3];
    for(int i=2;i>=0;i--) {
        uint32_t pair=decimal%100u;
        half[i]=(uint8_t)(((pair/10u)<<4u)|(pair%10u));
        decimal/=100u;
    }
    memcpy(money_out,half,3u);
    return true;
}
