#include "yellow_source_warp_graph.h"
/* Generated from pret/pokeyellow pinned original Object.asm warp_event
   coordinates/target pointers. Native arrival positions are existing v034
   accepted values, not fabricated ROM data. Unsupported destinations are
   excluded rather than routed to placeholder game scenes. */
static const YOriginalWarp source_warps[]={
 {0,5,5,1,2,6,1,1},
 {0,13,5,2,2,6,2,1},
 {0,12,11,3,4,10,3,2},
 {1,2,7,0,5,6,1,1},
 {1,3,7,0,5,6,2,1},
 {2,2,7,0,13,6,1,2},
 {2,3,7,0,13,6,2,2},
 {3,4,11,0,12,12,1,3},
 {3,5,11,0,12,12,2,3},
 {5,23,25,6,3,6,1,1},
 {6,3,7,5,23,26,1,1},
 {6,4,7,5,23,26,2,1},
 {7,3,43,9,4,6,6,3},
 {7,3,11,10,4,6,2,2},
 {8,15,47,9,4,1,3,2},
 {8,16,47,9,4,1,4,2},
 {8,17,47,9,4,1,5,2},
 {8,18,47,9,4,1,6,2},
 {8,1,0,10,4,6,1,3},
 {8,2,0,10,4,6,2,3},
 {9,4,0,8,16,46,1,4},
 {9,5,0,8,16,46,2,4},
 {9,4,7,7,3,44,3,6},
 {9,5,7,7,3,44,4,6},
 {10,4,7,8,1,1,3,1},
 {10,5,7,8,1,1,4,1},
 {10,4,0,7,3,10,1,2},
 {10,5,0,7,3,10,2,2},
};
/* Original MtMoonB1F/1F reciprocal warp events. Unsupported B2F/LAST_MAP
   are deliberately excluded until their destinations exist. The protected
   source_warps[] array above remains exactly 28 records. */
static const YOriginalWarp mtmoon_b1f_supported[]={
 {16,5,5,15,5,5,1,3},
 {16,25,9,15,17,11,3,4},
 {16,25,15,15,25,15,4,5},
 {15,5,5,16,5,5,3,1},
 {15,17,11,16,25,9,4,3},
 {15,25,15,16,25,15,5,4},
};

uint8_t yellow_original_warp_count(void) {
 return (uint8_t)(sizeof(source_warps)/sizeof(source_warps[0]));
}
bool yellow_original_warp_record(uint8_t index,YOriginalWarp* out) {
 if(!out||index>=yellow_original_warp_count())return false;
 *out=source_warps[index];return true;
}
bool yellow_original_warp_at(uint8_t map,int x,int y,
                             uint8_t* target,uint8_t* tx,uint8_t* ty) {
 if(!target||!tx||!ty||x<0||y<0||x>255||y>255)return false;
 for(uint8_t i=0;i<yellow_original_warp_count();i++) {
  const YOriginalWarp* w=&source_warps[i];
  if(w->src==map && w->x==(uint8_t)x && w->y==(uint8_t)y) {
   *target=w->target; *tx=w->target_x;*ty=w->target_y;return true;
  }
 }
 for(size_t i=0;i<sizeof(mtmoon_b1f_supported)/sizeof(mtmoon_b1f_supported[0]);i++) {
  const YOriginalWarp* w=&mtmoon_b1f_supported[i];
  if(w->src==map && w->x==(uint8_t)x && w->y==(uint8_t)y) {
   *target=w->target; *tx=w->target_x; *ty=w->target_y; return true;
  }
 }
 return false;
}
