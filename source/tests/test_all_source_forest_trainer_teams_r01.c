#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "pydp_reader.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
#include "../pydp_reader.c"
static int read_file(void* ctx,uint32_t offset,uint8_t* result,size_t len) {
 FILE* f=(FILE*)ctx;
 if(!len||len>128||fseek(f,(long)offset,SEEK_SET))return 0;
 return fread(result,1,len,f)==len;
}
static void all_teams(PydpPack* p) {
 const uint8_t klass[5]={1,1,1,1,2};
 const uint8_t numbers[5]={1,2,3,15,19};
 const uint8_t source_dex[9]={133,16,19,25,10,11,17,29,32};
 for(uint8_t t=0;t<5;t++) {
  const YSourceForestParty* party=y_source_forest_party(t);
  assert(party && party->class_id==klass[t] && party->party_number==numbers[t]);
  PydpEncounter originals[6];size_t n=0;
  assert(pydp_trainer_party(p,klass[t],numbers[t],originals,6,&n));
  assert(n==party->count && n==forest_party_counts[t]);
  for(uint8_t mon=0;mon<party->count;mon++) {
   uint8_t k=0,level=0;
   YellowGame g;yellow_init(&g);
   forest_party_mon(&g,t,mon,&k,&level);
   assert(y_species_info(k));
   assert(source_dex[k]==originals[mon].dex);
   assert(level==originals[mon].level);
   assert(k==forest_party_species[t][mon]&&level==forest_party_levels[t]);
  }
 }
 assert(y_source_forest_party(5)==NULL);
 assert(!y_source_forest_mon(5,0,&(uint8_t){0},&(uint8_t){0}));
 assert(!y_source_forest_mon(0,3,&(uint8_t){0},&(uint8_t){0}));
 assert(!y_source_forest_mon(0,0,NULL,&(uint8_t){0}));
}
static void battle_and_defeat_save(void) {
 YellowGame g;yellow_init(&g);
 g.story=YStoryPikachuFollows;g.map=YMapViridianForest;
 g.screen=YScreenWorld;g.has_pikachu=1;g.follower_valid=0;g.x=29;g.y=33;
 forest_start_battle(&g,1);
 assert(g.forest_active_trainer==1 && g.foe_kind==YK_METAPOD&&g.foe_level==6);
 finalize_battle(&g,true);
 assert(g.forest_active_trainer==1&&g.forest_party_index==1&&g.foe_kind==YK_CATERPIE);
 finalize_battle(&g,true);
 assert(g.forest_active_trainer==1&&g.forest_party_index==2&&g.foe_kind==YK_METAPOD);
 finalize_battle(&g,true);
 assert(g.forest_active_trainer==255&&(g.forest_defeated_mask&2));
 assert(g.screen==YScreenDialog);
 yellow_input(&g,YOk);
 assert(g.screen==YScreenWorld);
 YellowSave parent;yellow_save_create(&g,&parent);
 YellowTrainerSave side;yellow_trainer_save_create(&g,&parent,&side);
 YellowGame restored;assert(yellow_save_load(&restored,&parent));
 assert(yellow_trainer_save_load(&restored,&parent,&side));
 assert(restored.forest_defeated_mask&2);
 assert(parent.version==13 && side.version==1);
 side.parent_checksum^=1;
 assert(!yellow_trainer_save_load(&restored,&parent,&side));
}
int main(void) {
 FILE* f=fopen("../../../bulk-pokeyellow-r01/pokeyellow_data.pydb","rb");
 assert(f);PydpPack src;
 assert(pydp_open(&src,read_file,f)&&pydp_verify(&src));
 all_teams(&src);
 battle_and_defeat_save();
 fclose(f);
 puts("ALL_5_ORIGINAL_FOREST_TRAINERS_PYDP_PARTIES_BATTLE_DEFEAT_V13_SIDECAR_PASS");
 return 0;
}
