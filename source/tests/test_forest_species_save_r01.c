#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
static void active(YellowGame* g,uint8_t kind,uint8_t level){
 yellow_init(g);g->map=YMapViridianForest;g->x=15;g->y=16;
 g->story=YStoryPikachuFollows;g->has_pikachu=1;
 g->screen=YScreenWorld;g->hp=yellow_player_max_hp(g);
 yellow_begin_battle(g,kind,level);
 assert(g->foe_kind==kind&&g->foe_level==level&&g->foe_hp>0);
}
static void catch_copy(YellowGame* g,uint8_t kind,uint8_t level){
 YCapturedMon* m=&g->caught[0];memset(m,0,sizeof(*m));
 m->species=kind;m->level=level;m->hp=y_calc_stats(kind,level).hp;
 m->experience=y_exp_threshold(kind,level);
 y_build_moveset(kind,level,m->moves);
 for(int i=0;i<4;i++)m->pp[i]=y_max_pp(m->moves[i]);
 g->caught_count=1;
}
int main(void){
 YellowGame g,loaded;
 const uint8_t kinds[]={YK_CATERPIE,YK_METAPOD,YK_PIDGEOTTO};
 const uint8_t levels[]={3,4,9};
 assert(y_wild_move(YK_CATERPIE,3,0)==YM_TACKLE);
 assert(y_wild_move(YK_CATERPIE,3,1)==YM_STRING_SHOT);
 assert(y_wild_move(YK_METAPOD,4,0)==YM_HARDEN);
 assert(y_wild_move(YK_PIDGEOTTO,9,0)==YM_GUST);
 assert(y_wild_move(YK_PIDGEOTTO,9,1)==YM_SAND_ATTACK);
 assert(y_species_catch_rate(YK_CATERPIE)==255);
 assert(y_species_catch_rate(YK_METAPOD)==120);
 assert(y_species_catch_rate(YK_PIDGEOTTO)==120);
 for(int i=0;i<3;i++) {
  active(&g,kinds[i],levels[i]);
  catch_copy(&g,kinds[i],levels[i]);
  YellowSave save={0};yellow_save_create(&g,&save);
  assert(save.version==13 && save.caught[0].species==kinds[i]);
  assert(yellow_save_load(&loaded,&save));
  assert(loaded.caught_count==1&&loaded.caught[0].species==kinds[i]);
  assert(loaded.foe_kind==kinds[i]&&loaded.foe_level==levels[i]);
  save.caught[0].species=YK_PIKACHU;
  save.checksum=checksum(&save);
  assert(!yellow_save_load(&loaded,&save));
 }
 active(&g,YK_CATERPIE,3);
 apply_move_effect(&g,false,YM_STRING_SHOT,0,false);
 assert(g.pika_speed_stage==-1);
 active(&g,YK_METAPOD,4);
 apply_move_effect(&g,false,YM_HARDEN,0,false);
 assert(g.foe_defense_stage==1);
 puts("FOREST_SPECIES_CAPTURE_SAVE_MOVE_RUNTIME_PASS");return 0;
}
