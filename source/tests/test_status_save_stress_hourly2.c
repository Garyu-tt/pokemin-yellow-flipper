/* Independent 16,384-case deterministic status/save roundtrip regression. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
int main(void) {
    const uint8_t statuses[]={YStatusNone,YStatusBurned,YStatusPoisoned,
        YStatusParalyzed,YStatusFrozen,YStatusSleep1,YStatusSleep2,
        YStatusSleep3,YStatusSleep4,YStatusToxic1,YStatusToxic15};
    for(uint32_t i=0;i<16384;i++) {
        YellowGame g,copy;
        YellowSave s;
        yellow_init(&g);
        g.story=YStoryPikachuFollows;g.has_pikachu=1;
        g.map=YMapRoute1;g.x=10;g.y=28;
        g.hp=yellow_player_max_hp(&g);
        g.pika_status=statuses[i%(sizeof(statuses)/sizeof(statuses[0]))];
        g.rng=i*2654435761u+1u;
        yellow_save_create(&g,&s);
        assert(yellow_save_load(&copy,&s));
        assert(copy.pika_status==g.pika_status);
        assert(copy.rng==g.rng);
        s.pika_status^=1u;
        assert(!yellow_save_load(&copy,&s));
    }
    puts("STATUS_SAVE_16384_ROUNDTRIP_AND_TAMPER_PASS");
    return 0;
}
