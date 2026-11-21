#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "../yellow_viridian_mart_purchase.c"
#include "../yellow_mart_runtime.c"
int main(void) {
    YellowGame g={0};
    g.money_bcd[1]=0x30; /* original starting balance 3000 */
    YellowGame before=g;
    assert(!yellow_game_buy_antidote(&g,1u));
    assert(!memcmp(&g,&before,sizeof(g)));
    g.oak_parcel_flags=YOakParcelGot;
    before=g;
    assert(!yellow_game_buy_antidote(&g,1u));
    assert(!memcmp(&g,&before,sizeof(g)));
    g.oak_parcel_flags=YOakParcelGot|YOakParcelDelivered;
    assert(yellow_game_buy_antidote(&g,1u));
    assert(g.antidotes==1u);
    assert(g.money_bcd[0]==0 && g.money_bcd[1]==0x29 && g.money_bcd[2]==0);
    assert(yellow_game_buy_antidote(&g,29u));
    assert(g.antidotes==30u && g.money_bcd[0]==0 &&
           g.money_bcd[1]==0 && g.money_bcd[2]==0);
    before=g;
    assert(!yellow_game_buy_antidote(&g,1u));
    assert(!memcmp(&g,&before,sizeof(g)));
    g.money_integrity_error=true;before=g;
    assert(!yellow_game_buy_antidote(&g,1u));
    assert(!memcmp(&g,&before,sizeof(g)));
    g.money_integrity_error=false;g.antidote_integrity_error=true;before=g;
    assert(!yellow_game_buy_antidote(&g,1u));
    assert(!memcmp(&g,&before,sizeof(g)));
    g.antidote_integrity_error=false;g.oak_parcel_integrity_error=true;before=g;
    assert(!yellow_game_buy_antidote(&g,1u));
    assert(!memcmp(&g,&before,sizeof(g)));
    assert(!yellow_game_buy_antidote(NULL,1u));
    puts("R22_MART_RUNTIME_PARCEL_MONEY_STOCK_INTEGRITY_PASS");
    return 0;
}
