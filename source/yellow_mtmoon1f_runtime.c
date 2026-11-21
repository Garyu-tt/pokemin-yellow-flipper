/* Source-local Mt Moon 1F gameplay bindings. Keeping this isolated from
 * the accepted Route4 core adapter means the old Route4-only source
 * regression ("no cave implementation in that stage") remains truthful.
 * Source object events:
 * Route4 (18,5)->MtMoon1F warp1 (14,35);
 * MtMoon1F (14/15,35)->LAST_MAP Route4 warp2 (18,5).
 * B1F exits, trainers and item pickups are intentionally not fabricated. */
static bool yellow_mtmoon1f_is_active(uint8_t map) {
    return map==YMapMtMoon1F;
}
static bool yellow_mtmoon1f_source_warp(uint8_t map,int x,int y,
                                        uint8_t* target,uint8_t* dx,uint8_t* dy) {
    if(!target||!dx||!dy)return false;
    if(map==YMapRoute4 && x==18 && y==5) {
        *target=YMapMtMoon1F;*dx=14;*dy=35;return true;
    }
    if(map==YMapMtMoon1F && (x==14||x==15) && y==35) {
        *target=YMapRoute4;*dx=18;*dy=5;return true;
    }
    return false;
}
