/* Route22 static 1-bit tile projection pinned to original source. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../yellow_route22_crisp_tiles.c"
static uint32_t fnv(const uint8_t* p,unsigned n){
 uint32_t h=2166136261u;
 for(unsigned i=0;i<n;i++)h=(h^p[i])*16777619u;
 return h;
}
static unsigned bits(uint8_t b){
 unsigned n=0;for(;b;b>>=1)n+=b&1u;return n;
}
static unsigned ink(unsigned tile){
 unsigned n=0;for(unsigned i=0;i<8;i++)
  n+=bits(ytiles_overworld_route22_crisp[tile*8+i]);
 return n;
}
int main(void){
 assert(sizeof(ytiles_overworld_route22_crisp)==768);
 assert(fnv(ytiles_overworld_route22_crisp,768)==0x7d31a303u);
 assert(ink(17)<=10 && ink(91)==0 && ink(44)<=8);
 assert(ink(55)>=20);
 unsigned total=0;
 for(unsigned i=0;i<768;i++)
  total+=bits(ytiles_overworld_route22_crisp[i]);
 assert(total==1961);
 puts("ROUTE22_SOURCE_STATIC_XBM_CRSIP_96_TILES_PASS");
 return 0;
}
