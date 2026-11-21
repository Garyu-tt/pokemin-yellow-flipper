#include <assert.h>
#include <stdio.h>
#include "../yellow_whiteout_lastmap.c"
int main(void) {
    const uint32_t parent=0x11223344u;
    YWhiteoutLastMapRecord r={0};
    assert(y_whiteout_lastmap_create(1u,parent,&r));
    assert(r.magic==YWHITEOUT_LASTMAP_MAGIC && r.version==1u);
    uint8_t map=0xeeu;
    assert(y_whiteout_lastmap_restore(&r,parent,&map));
    assert(map==1u);
    map=0xeeu;
    assert(!y_whiteout_lastmap_restore(&r,parent+1u,&map) && map==0xeeu);
    r.checksum^=1u;
    assert(!y_whiteout_lastmap_restore(&r,parent,&map) && map==0xeeu);
    r.checksum^=1u;
    r.original_last_map=0x29u;
    assert(!y_whiteout_lastmap_restore(&r,parent,&map) && map==0xeeu);
    r.original_last_map=1u;
    r.reserved[0]=1u;
    assert(!y_whiteout_lastmap_restore(&r,parent,&map) && map==0xeeu);
    r.reserved[0]=0u;
    r.version=2u;
    assert(!y_whiteout_lastmap_restore(&r,parent,&map) && map==0xeeu);
    r.version=1u;
    assert(!y_whiteout_lastmap_create(0x29u,parent,&r));
    assert(!y_whiteout_lastmap_create(1u,0u,&r));
    assert(!y_whiteout_lastmap_create(1u,parent,0));
    assert(!y_whiteout_lastmap_restore(0,parent,&map));
    assert(!y_whiteout_lastmap_restore(&r,parent,0));
    puts("R012C3_ORIGINAL_LAST_BLACKOUT_MAP_VIRIDIAN_CHECKSUM_GUARD_PASS");
    puts("NOT_WARP_NOT_NATIVE_SD_NOT_COMPLETE_WHITEOUT_RECOVERY");
    return 0;
}
