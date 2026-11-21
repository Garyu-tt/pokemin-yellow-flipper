#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "yellow_owner_pewter_source.h"
#include "pydp_reader.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
#include "../yellow_owner_pewter_source.c"
#include "../pydp_reader.c"
static int read_db(void* ctx,uint32_t pos,uint8_t* out,size_t n) {
 FILE* f=(FILE*)ctx;
 if(!n || n>128 || fseek(f,(long)pos,SEEK_SET))return 0;
 return fread(out,1,n,f)==n;
}
static void check_party(PydpPack* db,unsigned slot,
                        uint8_t klasse,uint8_t number,
                        const uint8_t native_expected[2]) {
 YPewterTrainerParty original={0};PydpEncounter from_db[6];
 size_t count=0;
 assert(y_pewter_trainer((uint8_t)slot,&original));
 assert(original.trainer_class==klasse && original.party_number==number);
 assert(pydp_trainer_party(db,klasse,number,from_db,6,&count));
 assert(count==2 && original.count==count);
 for(unsigned i=0;i<count;i++) {
  uint8_t native=native_expected[i];
  assert(y_species_info(native)!=NULL);
  assert(from_db[i].dex==original.mon[i].dex);
  assert(from_db[i].level==original.mon[i].level);
  YSourceSpecies species={0};
  assert(y_source_species(original.mon[i].dex,&species));
  const YSpeciesInfo* legacy=y_species_info(native);
  assert(legacy->hp==species.hp && legacy->attack==species.attack);
  assert(legacy->defense==species.defense && legacy->special==species.special);
  assert(legacy->speed==species.speed && legacy->type1==species.type1);
  assert(legacy->type2==species.type2 && legacy->exp_yield==species.base_exp);
  assert(legacy->growth==species.growth);
  YBattleStats calculated,expected;
  assert(y_source_calc_stats(original.mon[i].dex,original.mon[i].level,&expected));
  calculated=y_calc_stats(native,original.mon[i].level);
  assert(memcmp(&expected,&calculated,sizeof(expected))==0);
  assert(y_exp_threshold(native,original.mon[i].level)==
         y_source_growth_threshold(original.mon[i].dex,original.mon[i].level));
  YellowGame battle;yellow_init(&battle);
  battle.story=YStoryPikachuFollows;battle.has_pikachu=1;
  battle.screen=YScreenWorld;
  yellow_begin_battle(&battle,native,original.mon[i].level);
  assert(battle.foe_kind==native && battle.foe_level==original.mon[i].level);
  assert(battle.foe_max_hp==expected.hp && battle.foe_hp==expected.hp);
 }
}
int main(void) {
 FILE* file=fopen("../../../bulk-pokeyellow-r01/pokeyellow_data.pydb","rb");
 assert(file);
 PydpPack db;assert(pydp_open(&db,read_db,file) && pydp_verify(&db));
 const uint8_t jr[]={YK_DIGLETT,YK_SANDSHREW};
 const uint8_t brock[]={YK_GEODUDE,YK_ONIX};
 check_party(&db,0,4,1,jr);
 check_party(&db,1,33,1,brock);
 assert(!y_move_info(YM_RESERVED_19));
 assert(!y_move_available(YK_GEODUDE,10,YM_SCRATCH));
 assert(y_wild_move(YK_GEODUDE,10,0)==YM_TACKLE);
 assert(y_wild_move(YK_GEODUDE,10,1)==YM_NONE);
 assert(y_wild_move(YK_ONIX,12,0)==YM_TACKLE);
 assert(y_wild_move(YK_ONIX,12,1)==YM_SCREECH);
 assert(y_wild_move(YK_DIGLETT,9,0)==YM_SCRATCH);
 assert(y_wild_move(YK_SANDSHREW,9,0)==YM_SCRATCH);
 YSourceMove move={0};
 assert(y_source_move(10,&move));
 assert(y_move_info(YM_SCRATCH)->power==move.power);
 assert(y_move_info(YM_SCRATCH)->accuracy==move.accuracy);
 assert(y_max_pp(YM_SCRATCH)==move.pp);
 assert(y_source_move(103,&move));
 assert(y_move_info(YM_SCREECH)->power==move.power);
 assert(y_move_info(YM_SCREECH)->accuracy==move.accuracy);
 assert(y_max_pp(YM_SCREECH)==move.pp);
 YellowGame g;yellow_init(&g);
 g.story=YStoryPikachuFollows;g.has_pikachu=1;
 g.map=YMapRoute1;g.x=10;g.y=20;g.screen=YScreenWorld;
 yellow_begin_battle(&g,YK_ONIX,12);
 int8_t before=g.pika_defense_stage;
 apply_move_effect(&g,false,YM_SCREECH,0,false);
 assert(g.pika_defense_stage==before-2);
 apply_move_effect(&g,false,YM_SCREECH,0,false);
 assert(g.pika_defense_stage==before-4);
 yellow_begin_battle(&g,YK_GEODUDE,10);
 assert(g.pika_defense_stage==0);
 assert(y_type_effectiveness(YT_ELECTRIC,YT_ROCK,YT_GROUND)==0u);
 YellowSave saved;yellow_save_create(&g,&saved);
 assert(saved.version==13);
 YellowGame copy;
 assert(yellow_save_load(&copy,&saved));
 assert(copy.foe_kind==YK_GEODUDE && copy.foe_level==10);
 assert(copy.map==YMapRoute1 && copy.x==10 && copy.y==20);
 fclose(file);
 puts("PEWTER_4_SOURCE_SPECIES_2_ORIGINAL_TEAMS_SCRATCH_SCREECH_BATTLE_SAVE_V13_PASS");
 return 0;
}
