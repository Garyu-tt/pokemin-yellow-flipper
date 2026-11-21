#include "yellow_route1_viewport.h"
#include <string.h>
static uint8_t reverse_bits(uint8_t v) {
 v=(uint8_t)(((v&0x55u)<<1)|((v&0xaau)>>1));
 v=(uint8_t)(((v&0x33u)<<2)|((v&0xccu)>>2));
 return (uint8_t)((v<<4)|(v>>4));
}
bool yellow_source_tile(YellowRoute1TileReader reader,void* ctx,
                        uint16_t col,uint16_t row,uint16_t max_cols,
                        uint16_t max_rows,uint8_t result[8]) {
 uint8_t src[8];
 if(!reader || !result ||
    col>=max_cols || row>=max_rows ||
    !reader(ctx,col,row,src))
    return false;
 /* PYTG is MSB-first from original PNG; canvas_draw_xbm is LSB-first. */
 for(unsigned y=0;y<8;y++)result[y]=reverse_bits(src[y]);
 return true;
}
bool yellow_route1_tile(YellowRoute1TileReader reader,void* ctx,uint16_t col,
                        uint16_t row,uint8_t result[8]) {
    return yellow_source_tile(reader,ctx,col,row,40,72,result);
}
static YellowRoute1Tile* slot(YellowRoute1Viewport* cache,int x,int y) {
 return &cache->tile[((unsigned)y%ROUTE1_CACHE_ROWS)*ROUTE1_CACHE_COLS+
                     ((unsigned)x%ROUTE1_CACHE_COLS)];
}
const uint8_t* yellow_route1_cached_tile(const YellowRoute1Viewport* cache,
                                          int x,int y) {
 if(!cache || !cache->initialized || x<0 || y<0)return NULL;
 const YellowRoute1Tile* item=
    &cache->tile[((unsigned)y%ROUTE1_CACHE_ROWS)*ROUTE1_CACHE_COLS+
                 ((unsigned)x%ROUTE1_CACHE_COLS)];
 return (item->valid==1 && item->x==x && item->y==y)?item->rows:NULL;
}
bool yellow_route1_prefetch(YellowRoute1Viewport* cache,
                            YellowRoute1TileReader reader,void* ctx,
                            int first_col,int first_row,
                            int world_cols,int world_rows,
                            unsigned budget) {
 if(!cache || !reader)return true;
 if(!cache->initialized) {
     memset(cache,0,sizeof(*cache));
     cache->initialized=1;
 }
 bool complete=true;
 /* Cover one extra tile on each edge while the camera pans smoothly. */
 /* Prioritize never-fetched tiles over transient failures. Otherwise a
  * permanently unreadable first tile monopolizes a one-read budget. */
 for(unsigned pass=0;pass<2;pass++) {
  for(int y=first_row;y<first_row+(int)ROUTE1_CACHE_ROWS && y<world_rows;y++) {
   for(int x=first_col;x<first_col+(int)ROUTE1_CACHE_COLS && x<world_cols;x++) {
    if(x<0||y<0)continue;
    YellowRoute1Tile* item=slot(cache,x,y);
    bool same=item->x==x && item->y==y;
    if(same && item->valid==1)continue;
    bool retry=same && item->valid==2;
    if(retry != (pass==1))continue;
    complete=false;
    if(!budget)continue;
    budget--;
    uint8_t image[8];
    bool ok=yellow_source_tile(reader,ctx,(uint16_t)x,(uint16_t)y,
                             (uint16_t)world_cols,(uint16_t)world_rows,image);
    item->x=(int16_t)x;item->y=(int16_t)y;
    item->valid=ok?1:2;
    if(ok)memcpy(item->rows,image,8);
   }
  }
 }
 return complete;
}
