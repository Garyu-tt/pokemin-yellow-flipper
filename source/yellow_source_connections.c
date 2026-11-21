#include "yellow_source_connections.h"
/* Exact header connection offsets from pret/pokeyellow
   b6696f24b3be84fe64cf702f5a15ba40c66b7f39.
   X corridors remain the old accepted navigable paths, no new story flags. */
typedef struct {
 uint8_t from,dir,to,min_x,max_x,entry_y,world_height;
 int8_t offset_blocks;
} SourceEdge;
static const SourceEdge edges[]={
 {0,0,4,10,11,35,18,0},
 {4,0,5,10,11,35,36,-5},
 {5,0,7,17,19,71,36,5},
 {4,1,0,10,11,0,36,0},
 {5,1,4,20,21,0,36,5},
 {7,1,5,7,9,0,72,-5},
 /* Pinned pret/pokeyellow Route2 north -5 / Pewter City south +5.
    Native step corridor x=9 maps to collision-open x=19 at y=35. */
 {7,0,11,9,9,35,72,-5},
 {11,1,7,19,19,0,36,5},
};
bool yellow_source_connection(uint8_t map,uint8_t direction,int x,int y,
                              uint8_t* next,uint8_t* dest_x,uint8_t* dest_y) {
 if(!next||!dest_x||!dest_y||x<0)return false;
 for(unsigned i=0;i<sizeof(edges)/sizeof(edges[0]);i++) {
  const SourceEdge* edge=&edges[i];
  if(edge->from!=map || edge->dir!=direction ||
     x<edge->min_x || x>edge->max_x || !((direction==0 && y<0) ||
                                         (direction==1 && y>=edge->world_height)))continue;
  int x_target=x-(int)edge->offset_blocks*2;
  if(x_target<0||x_target>255)return false;
  *next=edge->to;*dest_x=(uint8_t)x_target;*dest_y=edge->entry_y;
  return true;
 }
 return false;
}

/* Gen1 source header reciprocal east/west connections:
 * PewterCity EAST Route3 +4 blocks, Route3 WEST PewterCity -4.
 * 8 small movement tiles of vertical displacement. The exact passable
 * corridor is four rows; no phantom Route4/MtMoon connections. */
bool yellow_route3_horizontal_connection(uint8_t map,int x,int y,
    uint8_t* to,uint8_t* tx,uint8_t* ty){
    if(!to||!tx||!ty)return false;
    if(map==YMapPewterCity && x==40 && y>=16 && y<=19){
        *to=YMapRoute3;*tx=0;*ty=(uint8_t)(y-8);return true;
    }
    if(map==YMapRoute3 && x==-1 && y>=8 && y<=11){
        *to=YMapPewterCity;*tx=39;*ty=(uint8_t)(y+8);return true;
    }
    // Do not warp from corrupted/off-grid coordinates beyond one tile.
    // Exact-position restoration is handled by the generation-bound sidecar.
    return false;
}

/* Verified reciprocal header offsets:
 * Route3 north -> Route4 +25; Route4 south -> Route3 -25.
 * Restrict the link to seven original-source passable pairs. */
bool yellow_route4_vertical_connection(uint8_t map,int x,int y,
       uint8_t* next,uint8_t* tx,uint8_t* ty) {
    if(!next||!tx||!ty)return false;
    if(map==YMapRoute3 && y==-1 && x>=57 && x<=63) {
        *next=YMapRoute4;*tx=(uint8_t)(x-50);*ty=17;
        return true;
    }
    if(map==YMapRoute4 && y==18 && x>=7 && x<=13) {
        *next=YMapRoute3;*tx=(uint8_t)(x+50);*ty=0;
        return true;
    }
    // Do not warp from corrupted/off-grid coordinates beyond one tile.
    // Exact-position restoration is handled by the generation-bound sidecar.
    return false;
}
