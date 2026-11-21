#ifndef UX_VISUAL_MATH_H
#define UX_VISUAL_MATH_H
#include <stdint.h>
/* Visual-only helpers, independent of game-state and save format. */
static inline uint8_t ux_reverse_bits(uint8_t v) {
    v=(uint8_t)((v>>4)|(v<<4));
    v=(uint8_t)(((v&0xcc)>>2)|((v&0x33)<<2));
    return (uint8_t)(((v&0xaa)>>1)|((v&0x55)<<1));
}
/* Actual pret/pokeyellow gfx/sprites sheets: 0=down, 1=up,
   2=left; 3=walking-down, 4=walking-up, 5=walking-left.
   Walking frames are NOT adjacent to the corresponding idle pose. */
static inline int ux_sprite_frame(uint8_t direction,int walking) {
    int base=direction==0?1:direction==1?0:2;
    return base+(walking?3:0);
}
/* Constant physical speed between adjacent tiles. No per-tile easing. */
static inline uint16_t ux_linear_q8(uint32_t elapsed,uint32_t duration) {
    if(!duration || elapsed>=duration)return 256;
    return (uint16_t)(elapsed*256/duration);
}
/* Stand -> stride -> stand -> stride. Alternate mirrored foot each step,
   but NEVER mirror lateral sprites: doing so reverses facing. */
static inline int ux_walk_pose(uint16_t t) {
    return (t>=32 && t<112) || (t>=144 && t<224);
}
static inline uint16_t ux_ease_q8(uint32_t elapsed,uint32_t duration) {
    if(!duration||elapsed>=duration)return 256;
    uint32_t t=elapsed*256/duration;
    return (uint16_t)(t*t*(768-2*t)/65536);
}
static inline int ux_interpolate(int from,int to,uint16_t t) {
    return from+(int)(((int32_t)(to-from)*t)/256);
}
#endif
