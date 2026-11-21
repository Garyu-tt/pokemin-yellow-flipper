#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "../yellow_whiteout_slot.c"
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
#include "../yellow_pewter_progress.c"
#include "../yellow_atomic_snapshot.c"
#include "../yellow_exact_position_v1.c"
int main(void) {
    YWhiteoutSlot r={0}, before={0};
    uint8_t map=99u;
    assert(sizeof(YWhiteoutSlot)==24u && offsetof(YWhiteoutSlot,seal)==20u);
    assert(y_whiteout_slot_make(0u,0x13579bdfu,7u,&r));
    assert(y_whiteout_slot_restore(&map,0x13579bdfu,7u,&r) && map==0u);
    assert(y_whiteout_slot_make(1u,0x13579bdfu,7u,&r));
    map=99u;
    assert(y_whiteout_slot_restore(&map,0x13579bdfu,7u,&r) && map==1u);
    map=99u;
    assert(!y_whiteout_slot_restore(&map,0x13579bdfu,8u,&r) && map==99u);
    assert(!y_whiteout_slot_restore(&map,0x13579bdeu,7u,&r) && map==99u);
    before=r;
    r.checksum^=1u;
    assert(!y_whiteout_slot_restore(&map,0x13579bdfu,7u,&r) && map==99u);
    r=before;r.reserved[2]=1u;
    assert(!y_whiteout_slot_restore(&map,0x13579bdfu,7u,&r) && map==99u);
    r=before;r.original_last_map=0x29u;
    assert(!y_whiteout_slot_restore(&map,0x13579bdfu,7u,&r) && map==99u);
    r=before;r.seal=0u;
    assert(!y_whiteout_slot_restore(&map,0x13579bdfu,7u,&r) && map==99u);
    assert(!y_whiteout_slot_make(0x29u,0x13579bdfu,7u,&r));
    assert(!y_whiteout_slot_make(1u,0u,7u,&r));
    assert(!y_whiteout_slot_make(1u,0x13579bdfu,0u,&r));
    assert(!y_whiteout_slot_make(1u,0x13579bdfu,7u,NULL));
    assert(!y_whiteout_slot_restore(NULL,0x13579bdfu,7u,&before));
    YellowGame g;yellow_init(&g);
    assert(g.last_blackout_original_map==0u && !g.last_blackout_integrity_error);
    yellow_heal_pikachu(&g); /* generic heal is NOT nurse/last-map event */
    assert(g.last_blackout_original_map==0u);
    g.map=YMapViridianCenter;g.x=3;g.y=3;g.facing=YUp;
    g.story=YStoryPikachuFollows;g.has_pikachu=1;g.screen=YScreenWorld;
    interact(&g); /* real native nurse interaction, no invented remote trigger */
    assert(g.last_blackout_original_map==1u);
    g.last_blackout_original_map=0u;g.last_blackout_integrity_error=true;
    interact(&g);
    assert(g.last_blackout_original_map==0u && g.last_blackout_integrity_error);
    puts("R13_SOURCE_BACKED_NURSE_ANCHOR_AND_ATOMIC_SLOT_GUARDS_PASS");
    puts("NO_WHITEOUT_WARP_OR_MONEY_DEBIT_NO_FULL_RELEASE");
    return 0;
}
