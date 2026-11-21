/* Small in-memory 151-species Pokédex. Original numbers follow pinned
 * pret/pokeyellow/constants/pokedex_constants.asm (DEX_* constants).
 * The existing native 13-species battle enum is NOT the National Dex ID.
 * No artificial claims that all 151 have executable native battle behavior.
 * Compiled into yellow_core.c to preserve legacy test/link contracts. */
#include "yellow_pokedex_ledger.h"
#include <string.h>
static const uint8_t native_to_dex[] = {
    133,16,19,25,10,11,17,29,32,74,95,50,27
};
uint8_t yellow_dex_number(uint8_t species) {
    return species==YK_CLEFAIRY?35:(species==YK_ZUBAT?41:
           (species<sizeof(native_to_dex)?native_to_dex[species]:0));
}
static bool dex_slot(uint8_t number,unsigned* byte,uint8_t* mask) {
    if(number==0 || number>YELLOW_DEX_SPECIES)return false;
    unsigned bit=(unsigned)number-1u;
    *byte=bit>>3u;*mask=(uint8_t)(1u<<(bit&7u));
    return true;
}
bool yellow_dex_mark_seen(YellowDex* dex,uint8_t number) {
    unsigned i;uint8_t bit;
    if(!dex||!dex_slot(number,&i,&bit))return false;
    dex->seen[i]|=bit;return true;
}
bool yellow_dex_mark_caught(YellowDex* dex,uint8_t number) {
    unsigned i;uint8_t bit;
    if(!dex||!dex_slot(number,&i,&bit))return false;
    dex->seen[i]|=bit;dex->caught[i]|=bit;return true;
}
bool yellow_dex_seen(const YellowDex* dex,uint8_t number) {
    unsigned i;uint8_t bit;
    return dex&&dex_slot(number,&i,&bit)&&(dex->seen[i]&bit)!=0;
}
bool yellow_dex_caught(const YellowDex* dex,uint8_t number) {
    unsigned i;uint8_t bit;
    return dex&&dex_slot(number,&i,&bit)&&(dex->caught[i]&bit)!=0;
}
bool yellow_dex_valid(const YellowDex* dex) {
    if(!dex||((dex->seen[18]|dex->caught[18])&0x80u))return false;
    for(unsigned i=0;i<YELLOW_DEX_BYTES;i++)
        if(dex->caught[i] & (uint8_t)~dex->seen[i])return false;
    return true;
}
static uint16_t popcount(const uint8_t* data) {
    uint16_t count=0;
    for(unsigned i=0;i<YELLOW_DEX_BYTES;i++) {
        uint8_t x=data[i];while(x){count+=(uint16_t)(x&1u);x>>=1u;}
    }
    return count;
}
uint16_t yellow_dex_seen_count(const YellowDex* dex) {
    return yellow_dex_valid(dex)?popcount(dex->seen):0;
}
uint16_t yellow_dex_caught_count(const YellowDex* dex) {
    return yellow_dex_valid(dex)?popcount(dex->caught):0;
}
static uint32_t dex_checksum(const YellowDexSave* record) {
    uint32_t hash=2166136261u;
    const uint32_t numbers[]={record->magic,(uint32_t)record->version,
                              record->parent_checksum};
    /* Always hash packed deterministic bytes, not compiler struct padding. */
    for(unsigned n=0;n<2;n++)
        for(unsigned j=0;j<(n==0?4u:2u);j++) {
            hash^=(uint8_t)(numbers[n]>>(8u*j));hash*=16777619u;
        }
    for(unsigned i=0;i<YELLOW_DEX_BYTES;i++) {
        hash^=record->seen[i];hash*=16777619u;
    }
    for(unsigned i=0;i<YELLOW_DEX_BYTES;i++) {
        hash^=record->caught[i];hash*=16777619u;
    }
    for(unsigned j=0;j<4;j++) {
        hash^=(uint8_t)(numbers[2]>>(8u*j));hash*=16777619u;
    }
    return hash;
}
bool yellow_dex_save(const YellowDex* dex,uint32_t parent_checksum,YellowDexSave* out) {
    if(!out||!yellow_dex_valid(dex))return false;
    YellowDexSave record={0};
    record.magic=YELLOW_DEX_SAVE_MAGIC;
    record.version=YELLOW_DEX_SAVE_VERSION;
    memcpy(record.seen,dex->seen,YELLOW_DEX_BYTES);
    memcpy(record.caught,dex->caught,YELLOW_DEX_BYTES);
    record.parent_checksum=parent_checksum;
    record.checksum=dex_checksum(&record);
    *out=record;return true;
}
bool yellow_dex_restore(YellowDex* dex,uint32_t parent_checksum,
                        const YellowDexSave* source) {
    if(!dex||!source ||source->magic!=YELLOW_DEX_SAVE_MAGIC||
       source->version!=YELLOW_DEX_SAVE_VERSION||
       source->parent_checksum!=parent_checksum||
       source->checksum!=dex_checksum(source))return false;
    YellowDex result={0};
    memcpy(result.seen,source->seen,YELLOW_DEX_BYTES);
    memcpy(result.caught,source->caught,YELLOW_DEX_BYTES);
    if(!yellow_dex_valid(&result))return false;
    *dex=result;return true;
}
