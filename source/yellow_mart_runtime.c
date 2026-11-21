#include "yellow_core.h"
#include "yellow_oak_parcel_sidecar.h"
#include "yellow_viridian_mart_purchase.h"
#include <string.h>
/* A pure transaction on runtime state; caller commits with a paired sidecar.
 * No UI is enabled by this module. Failures do not change money or stock. */
bool yellow_game_buy_antidote(YellowGame* g,uint8_t quantity) {
    if(!g || g->money_integrity_error || g->antidote_integrity_error ||
       g->oak_parcel_integrity_error ||
       !(g->oak_parcel_flags & YOakParcelDelivered))return false;
    uint8_t new_money[3],new_stock=0;
    if(!yellow_viridian_mart_buy_antidote(g->money_bcd,g->antidotes,
           true,quantity,new_money,&new_stock))return false;
    memcpy(g->money_bcd,new_money,3u);
    g->antidotes=new_stock;
    return true;
}

/* Sale is atomic with respect to in-memory inventory and money.
 * Persistence must commit both paired sidecars before reporting success. */
bool yellow_game_sell_antidote(YellowGame* g,uint8_t quantity) {
    if(!g || g->money_integrity_error || g->antidote_integrity_error ||
       g->oak_parcel_integrity_error ||
       !(g->oak_parcel_flags & YOakParcelDelivered)) return false;
    uint8_t next_money[3],next_stock=0;
    if(!yellow_viridian_mart_sell_antidote(g->money_bcd,g->antidotes,
           quantity,next_money,&next_stock)) return false;
    memcpy(g->money_bcd,next_money,3u);
    g->antidotes=next_stock;
    return true;
}
