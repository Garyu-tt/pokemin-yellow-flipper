#include <assert.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
int main(void){YellowGame g;yellow_init(&g);g.running=true;g.screen=YScreenWorld;g.story=YStoryPikachuFollows;g.has_pikachu=1;g.native_zubat_extension=true;g.map=YMapMtMoon1F;g.pika_level=60;g.hp=y_calc_stats(YK_PIKACHU,60).hp;g.experience=y_exp_threshold(YK_PIKACHU,60);g.follower_valid=0;g.facing=YRight;int found=0;for(int y=2;y<32&&!found;y++)for(int x=2;x<36&&!found;x++)if(yellow_mtmoon1f_passable(x,y)&&yellow_mtmoon1f_passable(x+1,y)&&yellow_mtmoon1f_passable(x+2,y)){g.x=x;g.y=y;found=1;}assert(found);uint32_t seed=0;for(uint32_t n=1;n<300000;n++){YellowGame tmp={0};tmp.rng=n;if((next_rng(&tmp)&255u)>=10)continue;if((next_rng(&tmp)&255u)<51){seed=n;break;}}assert(seed);g.rng=seed;yellow_input(&g,YRight);assert(g.screen==YScreenRivalBattle);assert(g.foe_kind==YK_ZUBAT);assert(g.foe_level>=6&&g.foe_level<=11);assert(g.steps==1);return 0;}
