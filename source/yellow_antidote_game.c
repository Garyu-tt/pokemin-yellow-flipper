#include "yellow_core.h"
#include "yellow_antidote_cure.h"

bool yellow_game_apply_antidote(YellowGame* g,uint8_t party_index,uint8_t* stock) {
    /* A separate linkable module keeps the v13 core and legacy host tests
     * untouched. The caller owns inventory and its durable persistence. */
    if(!g || !stock || party_index>=yellow_party_size(g))return false;
    uint8_t next_status=0, next_stock=0;
    if(!yellow_antidote_cure(yellow_party_status(g,party_index),
                            yellow_party_hp(g,party_index),*stock,
                            &next_status,&next_stock))return false;
    if(party_index==0u)g->pika_status=next_status;
    else g->caught[party_index-1u].status=next_status;
    *stock=next_stock;
    return true;
}
