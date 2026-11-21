#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
static void setup(YellowGame* g,uint8_t species,uint8_t level) {
 yellow_init(g);
 g->story=YStoryPikachuFollows;g->screen=YScreenWorld;
 g->has_pikachu=1;g->hp=yellow_player_max_hp(g);
 g->caught_count=1;
 YCapturedMon* p=&g->caught[0];
 memset(p,0,sizeof(*p));
 p->species=species;p->level=level;
 p->experience=y_exp_threshold(species,(uint8_t)(level+1))-1;
 p->hp=y_calc_stats(species,level).hp;
 y_build_moveset(species,level,p->moves);
 for(unsigned n=0;n<4;n++)p->pp[n]=y_max_pp(p->moves[n]);
 g->foe_kind=YK_RATTATA;g->foe_level=10;
 g->participants_mask=2;g->active_party=1;
}
static void assert_saved(YellowGame* g,uint8_t expected) {
 YellowSave save;yellow_save_create(g,&save);
 YellowGame copy;
 assert(yellow_save_load(&copy,&save));
 assert(copy.caught_count==1 && copy.caught[0].species==expected);
 assert(copy.caught[0].level==g->caught[0].level);
}
int main(void) {
 assert(y_evolution_at(YK_CATERPIE,6)==YK_CATERPIE);
 assert(y_evolution_at(YK_CATERPIE,7)==YK_METAPOD);
 assert(y_evolution_at(YK_PIDGEY,17)==YK_PIDGEY);
 assert(y_evolution_at(YK_PIDGEY,18)==YK_PIDGEOTTO);
 assert(y_evolution_at(YK_METAPOD,10)==YK_METAPOD); /* Butterfree not yet ported */
 assert(y_evolution_at(YK_PIKACHU,100)==YK_PIKACHU); /* Yellow starter */
 YellowGame g;setup(&g,YK_CATERPIE,6);
 grant_exp(&g);
 assert(g.caught[0].species==YK_METAPOD && g.caught[0].level==7);
 assert(g.caught[0].hp==y_calc_stats(YK_METAPOD,7).hp);
 assert(g.caught[0].moves[0]==YM_TACKLE);
 assert(g.caught[0].moves[1]==YM_STRING_SHOT);
 assert(g.caught[0].moves[2]==YM_HARDEN);
 assert(y_move_available(YK_METAPOD,7,YM_TACKLE));
 assert_saved(&g,YK_METAPOD);
 setup(&g,YK_PIDGEY,17);
 grant_exp(&g);
 assert(g.caught[0].species==YK_PIDGEOTTO && g.caught[0].level>=18);
 assert(g.caught[0].hp==y_calc_stats(YK_PIDGEOTTO,g.caught[0].level).hp);
 assert_saved(&g,YK_PIDGEOTTO);
 setup(&g,YK_RATTATA,6);
 grant_exp(&g);
 assert(g.caught[0].species==YK_RATTATA);
 assert_saved(&g,YK_RATTATA);
 puts("PINNED_EVOLUTIONS_CATERPIE_PIDGEY_SAVE_PASS");
 return 0;
}
