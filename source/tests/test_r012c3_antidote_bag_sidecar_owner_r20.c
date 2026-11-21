#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../yellow_antidote_inventory_slot.c"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_antidote_cure.c"
#include "../yellow_core.c"
#include "../yellow_antidote_game.c"
#include "../yellow_pewter_progress.c"
#include "../yellow_atomic_snapshot.c"
#include "../yellow_exact_position_v1.c"
int main(void) {
    YellowGame g; yellow_init(&g);
    assert(g.antidotes==0 && !g.antidote_integrity_error);
    g.has_pikachu=1;g.hp=12;g.pika_status=YStatusPoisoned;
    g.antidotes=2;
    YellowAtomicSnapshot parent;
    assert(yellow_atomic_make(&g,4,&parent));
    YellowAntidoteInventorySlot slot,broken;
    uint8_t restored=99;
    assert(yellow_antidote_inventory_create(g.antidotes,parent.parent.checksum,4,&slot));
    assert(yellow_antidote_inventory_restore(&slot,parent.parent.checksum,4,&restored));
    assert(restored==2);
    assert(!yellow_antidote_inventory_restore(&slot,parent.parent.checksum,5,&restored));
    assert(restored==2);
    assert(!yellow_antidote_inventory_restore(&slot,parent.parent.checksum^1u,4,&restored));
    assert(restored==2);
    for(unsigned i=0;i<24;i++)for(unsigned bit=0;bit<8;bit++){
        broken=slot;broken.wire[i]^=(uint8_t)(1u<<bit);
        restored=77;
        assert(!yellow_antidote_inventory_restore(&broken,parent.parent.checksum,4,&restored));
        assert(restored==77);
    }
    assert(yellow_game_apply_antidote(&g,0,&g.antidotes));
    assert(g.pika_status==YStatusNone && g.antidotes==1);
    assert(!yellow_game_apply_antidote(&g,0,&g.antidotes));
    assert(g.antidotes==1);
    g.pika_status=YStatusToxic7;
    assert(yellow_game_apply_antidote(&g,0,&g.antidotes));
    assert(g.antidotes==0 && g.pika_status==YStatusNone);
    assert(!yellow_game_apply_antidote(&g,0,&g.antidotes));
    YellowSave native;
    yellow_save_create(&g,&native);
    assert(native.version==13);
    for(unsigned stock=0;stock<=99;stock++){
        assert(yellow_antidote_inventory_create((uint8_t)stock,parent.parent.checksum,4,&slot));
        restored=255;
        assert(yellow_antidote_inventory_restore(&slot,parent.parent.checksum,4,&restored));
        assert(restored==stock);
    }
    assert(!yellow_antidote_inventory_create(100,parent.parent.checksum,4,&slot));
    puts("R20_ANTIDOTE_BAG_CODEC_MUTATION_192_CASES_V13_UNCHANGED_PASS_PARTIAL");
    return 0;
}
