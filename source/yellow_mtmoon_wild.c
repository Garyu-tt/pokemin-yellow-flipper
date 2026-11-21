/* Source-verified MtMoon1F WildMons 10 original weighted slots.
 * Pinned pret/pokeyellow/data/wild/maps/MtMoon1F.asm.
 * Dex IDs are original: Zubat 41, Geodude 74, Sandshrew 27,
 * Clefairy 35. The existing native runtime supports only Geodude
 * (2/10 slots) and Sandshrew (1/10 slot). Unsupported 7/10 slots
 * do NOT turn into substituted encounters or get reweighted.
 * Full encounter parity and unsupported species behavior are NOT claimed.
 */
typedef struct {
    uint8_t original_dex_id;
    uint8_t level;
    uint8_t native_supported;
    uint8_t native_species;
} YellowMtMoonSlot;
#define YELLOW_MTMOON_WILD_RATE 10u
#define YELLOW_MTMOON_WILD_SLOTS 10u
static const YellowMtMoonSlot mtmoon_wild_source[10]={
    {41,8,0,0}, /* Lv8 source Zubat */
    {41,9,0,0},
    {74,10,1,YK_GEODUDE},
    {41,6,0,0},
    {41,7,0,0},
    {41,10,0,0},
    {74,10,1,YK_GEODUDE},
    {41,11,0,0},
    {27,12,1,YK_SANDSHREW},
    {35,11,0,0} /* Clefairy */
};
bool yellow_mtmoon_original_wild_slot(unsigned slot,YellowMtMoonSlot* out) {
    if(!out||slot>=YELLOW_MTMOON_WILD_SLOTS)return false;
    *out=mtmoon_wild_source[slot];
    return true;
}
bool yellow_mtmoon_native_wild_slot(unsigned slot,uint8_t* kind,uint8_t* level) {
    if(!kind||!level||slot>=YELLOW_MTMOON_WILD_SLOTS)return false;
    const YellowMtMoonSlot* s=&mtmoon_wild_source[slot];
    if(!s->native_supported)return false;
    *kind=s->native_species;
    *level=s->level;
    return true;
}

/* Separate opt-in extension: original slot weights and species remain
 * immutable; never substitute or silently increase legacy support. */
bool yellow_mtmoon_zubat_wild_slot(unsigned slot,uint8_t* kind,uint8_t* level) {
    if(!kind||!level||slot>=YELLOW_MTMOON_WILD_SLOTS)return false;
    const YellowMtMoonSlot* item=&mtmoon_wild_source[slot];
    if(item->original_dex_id!=41)return false;
    *kind=YK_ZUBAT;*level=item->level;
    return true;
}

/* Clefairy is original source slot 9 but intentionally excluded from
 * legacy v052 metadata. Opt-in live runtime overlay preserves weights. */
bool yellow_mtmoon_clefairy_wild_slot(unsigned slot,uint8_t* kind,uint8_t* level) {
    if(!kind||!level||slot!=9u)return false;
    const YellowMtMoonSlot* s=&mtmoon_wild_source[slot];
    if(s->original_dex_id!=35 || s->level!=11)return false;
    *kind=YK_CLEFAIRY;*level=s->level;
    return true;
}
