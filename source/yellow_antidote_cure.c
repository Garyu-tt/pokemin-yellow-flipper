#include "yellow_antidote_cure.h"
#include "yellow_battle.h"

bool yellow_antidote_cure(uint8_t status, uint8_t hp, uint8_t stock,
                         uint8_t* out_status, uint8_t* out_stock) {
    /* Original Antidote only removes PSN; Toxic uses the same PSN party
     * status bit and additionally clears the volatile toxic counter.
     * A fainted Pokémon, unpoisoned Pokémon or empty bag must not consume. */
    if(!out_status || !out_stock || out_status==out_stock || !hp || !stock)
        return false;
    if(status!=YStatusPoisoned &&
       !(status>=YStatusToxic1 && status<=YStatusToxic15))
        return false;
    *out_status=YStatusNone;
    *out_stock=(uint8_t)(stock-1u);
    return true;
}
