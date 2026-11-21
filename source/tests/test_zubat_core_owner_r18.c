#include <assert.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
int main(void){assert(y_species_info(13)==0);assert(y_species_info(YK_ZUBAT)!=0);assert(y_max_pp(YM_LEECH_LIFE)==15);assert(y_max_pp(YM_SUPERSONIC)==20);assert(yellow_dex_number(YK_ZUBAT)==41);return 0;}
