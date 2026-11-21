#include "yellow_bulk_data.h"
#include <string.h>
static bool dex_to_internal(uint8_t dex,uint8_t* out) {
    if(!out)return false;
    switch(dex) {
    case 10:*out=YK_CATERPIE;return true;
    case 11:*out=YK_METAPOD;return true;
    case 16:*out=YK_PIDGEY;return true;
    case 17:*out=YK_PIDGEOTTO;return true;
    case 19:*out=YK_RATTATA;return true;
    case 25:*out=YK_PIKACHU;return true;
    case 29:*out=YK_NIDORAN_F;return true;
    case 32:*out=YK_NIDORAN_M;return true;
    case 133:*out=YK_EEVEE;return true;
    default:return false;
    }
}
static bool load_map(PydpPack* pack,uint8_t original_map,YellowWildSlots* out) {
    PydpEncounter encounters[10];
    size_t count=0;
    uint8_t rate=0;
    if(!pydp_wild(pack,original_map,0,&rate,encounters,10,&count) ||
       count!=10 || rate!=25) return false;
    out->rate=rate;
    for(size_t i=0;i<10;i++) {
        if(!dex_to_internal(encounters[i].dex,&out->kind[i]))return false;
        out->level[i]=encounters[i].level;
    }
    return true;
}
bool yellow_bulk_prepare(PydpPack* pack,YellowBulkWild* result) {
    if(!pack || !result)return false;
    memset(result,0,sizeof(*result));
    return load_map(pack,12,&result->route1) &&
           load_map(pack,51,&result->forest);
}

/* TrainerDataPointers -> Lass class index 2, numbered original Lass #19.
   Validate both imported data and pinned base-stat values before adoption. */
bool yellow_bulk_prepare_lass(PydpPack* pack,YellowBulkLass* out) {
    if(!pack || !out)return false;
    PydpEncounter team[2];
    PydpSpecies female,male;
    size_t count=0;
    if(!pydp_species(pack,29,&female) || !pydp_species(pack,32,&male) ||
       female.hp!=55 || female.atk!=47 || female.def!=52 ||
       female.spd!=41 || female.spc!=40 || female.base_exp!=59 ||
       male.hp!=46 || male.atk!=57 || male.def!=40 ||
       male.spd!=50 || male.spc!=40 || male.base_exp!=60 ||
       !pydp_trainer_party(pack,2,19,team,2,&count) || count!=2)
        return false;
    YellowBulkLass candidate={0};
    candidate.count=(uint8_t)count;
    for(unsigned i=0;i<2;i++) {
        if(!dex_to_internal(team[i].dex,&candidate.kind[i]) ||
           team[i].level!=6)return false;
        candidate.level[i]=team[i].level;
    }
    if(candidate.kind[0]!=YK_NIDORAN_F ||
       candidate.kind[1]!=YK_NIDORAN_M)return false;
    *out=candidate;
    return true;
}
