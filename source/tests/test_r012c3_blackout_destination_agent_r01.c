#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../yellow_blackout_destination.c"

int main(void) {
    uint8_t result;
    uint8_t preserved = 0xAA;

    /* === Positive case: VIRIDIAN_CITY=0x01, all conditions met === */
    assert(yellow_resolve_blackout_destination(0x01, true, true, false, &result) == true);
    assert(result == YMapViridian);
    assert(result == 5);

    /* === Invalid original ID 0x29 (not VIRIDIAN_CITY) === */
    preserved = 0xBB;
    result = 0xFF;
    assert(yellow_resolve_blackout_destination(0x29, true, true, false, &result) == false);
    assert(result == 0xFF); /* output preserved on rejection */

    /* === Invalid original ID 0x00 === */
    result = 0xCC;
    assert(yellow_resolve_blackout_destination(0x00, true, true, false, &result) == false);
    assert(result == 0xCC); /* preserved */

    /* === Invalid original ID 0x02 (wrong city) === */
    result = 0xDD;
    assert(yellow_resolve_blackout_destination(0x02, true, true, false, &result) == false);
    assert(result == 0xDD); /* preserved */

    /* === Missing anchor (anchor=false) === */
    result = 0xEE;
    assert(yellow_resolve_blackout_destination(0x01, false, true, false, &result) == false);
    assert(result == 0xEE); /* preserved */

    /* === Party not fainted (all_party_fainted=false) === */
    result = 0xFF;
    assert(yellow_resolve_blackout_destination(0x01, true, false, false, &result) == false);
    assert(result == 0xFF); /* preserved */

    /* === Corruption present (corrupted=true) === */
    result = 0x01;
    assert(yellow_resolve_blackout_destination(0x01, true, true, true, &result) == false);
    assert(result == 0x01); /* preserved, not overwritten */

    /* === NULL output pointer === */
    assert(yellow_resolve_blackout_destination(0x01, true, true, false, NULL) == false);

    /* === Combination failures: anchor missing + party not fainted === */
    result = 0x11;
    assert(yellow_resolve_blackout_destination(0x01, false, false, false, &result) == false);
    assert(result == 0x11); /* preserved */

    /* === Combination failures: corrupted + invalid map === */
    result = 0x22;
    assert(yellow_resolve_blackout_destination(0x29, true, true, true, &result) == false);
    assert(result == 0x22); /* preserved */

    /* === Large invalid original map ID === */
    result = 0x33;
    assert(yellow_resolve_blackout_destination(0xFF, true, true, false, &result) == false);
    assert(result == 0x33); /* preserved */

    printf("BLACKOUT_DESTINATION_BOUNDED_R012C3_PASS\n");
    return 0;
}
