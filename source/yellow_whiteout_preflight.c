#include "yellow_whiteout_preflight.h"

/* Source-backed preflight only. A valid original Viridian overworld map
 * does not imply a valid native recovery tile or a persisted last-heal
 * anchor. No automatic teleport, healing or money deduction occurs here. */
bool y_whiteout_preflight(uint8_t map, uint8_t party_count, bool all_fainted,
                          bool scripted_movement, bool link_connected,
                          YWhiteoutPreflight* out) {
    if(!out || map!=YWHITEOUT_ORIGINAL_VIRIDIAN_CITY || !party_count ||
       !all_fainted || scripted_movement || link_connected)return false;
    const YWhiteoutPreflight result={map,true,false};
    *out=result;
    return true;
}

bool y_whiteout_half_money_bcd(const uint8_t money[3],uint8_t out[3]) {
    if(!money||!out)return false;
    uint32_t decimal=0;
    for(unsigned i=0;i<3;i++) {
        const unsigned high=(unsigned)(money[i]>>4);
        const unsigned low=(unsigned)(money[i]&15u);
        if(high>9u || low>9u)return false;
        decimal=decimal*100u+high*10u+low;
    }
    decimal/=2u;
    uint8_t tmp[3]={0};
    for(int i=2;i>=0;i--) {
        unsigned two=decimal%100u;
        tmp[i]=(uint8_t)(((two/10u)<<4u)|(two%10u));
        decimal/=100u;
    }
    for(unsigned i=0;i<3;i++)out[i]=tmp[i];
    return true;
}
