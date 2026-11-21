#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
static void safe_readonly_preflight(void) {
    YellowGame g;yellow_init(&g);
    YellowFlipperSmokeContract c={0};
    assert(!yellow_flipper_smoke_contract(NULL,&c));
    assert(!yellow_flipper_smoke_contract(&g,NULL));
    YellowGame backup=g;
    assert(yellow_flipper_smoke_contract(&g,&c));
    assert(!memcmp(&backup,&g,sizeof(g)));
    assert(c.screen_width==128 && c.screen_height==64);
    assert(c.native_save_v13_bytes==132);
    assert(c.native_scene_identifiers==13);
    assert(c.pokedex_names==151 && c.pc_capacity==20);
    assert(c.requires_owner_device_approval);
    assert(!c.physical_flipper_verified && !c.real_input_save_roundtrip_verified);
    assert(!c.full_original_game_playthrough_verified);
    YellowSave before,after;
    yellow_save_create(&g,&before);
    assert(yellow_flipper_smoke_contract(&g,&c));
    yellow_save_create(&g,&after);
    assert(!memcmp(&before,&after,sizeof(before)));
    g.map=YELLOW_GYM_MAP_COUNT;
    YellowFlipperSmokeContract sentinel=c;
    assert(!yellow_flipper_smoke_contract(&g,&c));
    assert(!memcmp(&c,&sentinel,sizeof(c)));
    g.map=YMapPewterGym;
    assert(yellow_flipper_smoke_contract(&g,&c));
    g.has_pikachu=1;g.pika_level=0;
    sentinel=c;
    assert(!yellow_flipper_smoke_contract(&g,&c));
    assert(!memcmp(&c,&sentinel,sizeof(c)));
}
int main(void) {
    safe_readonly_preflight();
    puts("FLIPPER_128x64_V13_READ_ONLY_SMOKE_NEGATIVE_HARDWARE_CLAIMS_PASS");
    return 0;
}
