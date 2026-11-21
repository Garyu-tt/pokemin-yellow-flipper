#include <assert.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
int main(void){YellowGame g;yellow_init(&g);g.native_zubat_extension=true;g.map=YMapMtMoon1F;for(unsigned i=0;i<10;i++){uint8_t k=0,l=0;bool ok=yellow_wild_slot(&g,YMapMtMoon1F,i,&k,&l);assert(ok==(i!=9));if(ok&&i!=2&&i!=6&&i!=8)assert(k==YK_ZUBAT&&l>=6&&l<=11);}assert(yellow_dex_number(YK_ZUBAT)==41);assert(y_max_pp(YM_LEECH_LIFE)==15);assert(y_max_pp(YM_SUPERSONIC)==20);return 0;}
