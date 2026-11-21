#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_core.h"

int main(void) {
    // Verify that the new map IDs are correctly assigned
    assert(YMapMtMoon1F == 15);
    assert(YMapMtMoonB1F == 16);
    assert(YMapMtMoonB2F == 17);
    
    puts("MTMOON_MAP_IDS_CORRECT_PASS");
    return 0;
}