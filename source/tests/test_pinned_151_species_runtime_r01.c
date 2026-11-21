#include <assert.h>
#include <stdio.h>
#include "yellow_core.h"
#include "pydp_reader.h"
#include "yellow_source_species.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
#include "../pydp_reader.c"
static int bounded_read(void* ctx,uint32_t pos,uint8_t* out,size_t count) {
 FILE* f=(FILE*)ctx;
 if(count==0 || count>128 || fseek(f,(long)pos,SEEK_SET))return 0;
 return fread(out,1,count,f)==count;
}
int main(void) {
 FILE* f=fopen("../../../bulk-pokeyellow-r01/pokeyellow_data.pydb","rb");
 assert(f);PydpPack source;
 assert(pydp_open(&source,bounded_read,f)&&pydp_verify(&source));
 unsigned checked=0;
 for(unsigned dex=1;dex<=151;dex++) {
  YSourceSpecies native;PydpSpecies original;
  assert(y_source_species((uint8_t)dex,&native));
  assert(pydp_species(&source,(uint8_t)dex,&original));
  assert(native.dex==original.dex && native.hp==original.hp);
  assert(native.attack==original.atk && native.defense==original.def);
  assert(native.speed==original.spd && native.special==original.spc);
  assert(native.catch_rate==original.catch_rate);
  assert(native.base_exp==original.base_exp);
  assert(native.type1<=26 && native.type2<=26);
  assert(native.growth<=3);
  YBattleStats level5,level50;
  assert(y_source_calc_stats((uint8_t)dex,5,&level5));
  assert(y_source_calc_stats((uint8_t)dex,50,&level50));
  assert(level5.hp>=10 && level50.hp>=level5.hp);
  assert(y_source_growth_threshold((uint8_t)dex,50)>0);
  checked++;
 }
 assert(checked==151);
 const uint8_t internal[]={133,16,19,25,10,11,17,29,32};
 for(unsigned i=0;i<sizeof(internal);i++) {
  YSourceSpecies raw;assert(y_source_species(internal[i],&raw));
  const YSpeciesInfo* old=y_species_info((uint8_t)i);
  assert(old && old->hp==raw.hp && old->attack==raw.attack);
  assert(old->defense==raw.defense && old->speed==raw.speed);
  assert(old->special==raw.special && old->exp_yield==raw.base_exp);
  assert(old->type1==raw.type1 && old->type2==raw.type2);
  assert(old->growth==raw.growth);
  for(uint8_t lv=2;lv<=90;lv+=11) {
   YBattleStats a,b;
   assert(y_source_calc_stats(internal[i],lv,&a));
   b=y_calc_stats((uint8_t)i,lv);
   assert(a.hp==b.hp&&a.attack==b.attack&&a.defense==b.defense);
   assert(a.speed==b.speed&&a.special==b.special);
   assert(y_source_growth_threshold(internal[i],lv)==y_exp_threshold((uint8_t)i,lv));
  }
 }
 YSourceSpecies preserved={0};
 assert(!y_source_species(0,&preserved));
 assert(!y_source_species(152,&preserved));
 assert(!y_source_species(1,NULL));
 YBattleStats bad={0};
 assert(!y_source_calc_stats(1,0,&bad));
 assert(!y_source_calc_stats(151,101,&bad));
 assert(!y_source_calc_stats(152,10,&bad));
 assert(!y_source_calc_stats(1,10,NULL));
 assert(y_source_growth_threshold(0,5)==0);
 assert(y_source_growth_threshold(152,5)==0);
 YellowGame g;yellow_init(&g);
 g.story=YStoryPikachuFollows;g.has_pikachu=1;
 g.map=YMapRoute1;g.x=10;g.y=20;
 YellowSave save;yellow_save_create(&g,&save);
 YellowGame loaded;assert(save.version==13);
 assert(yellow_save_load(&loaded,&save));
 assert(loaded.map==YMapRoute1 && loaded.pika_level==g.pika_level);
 fclose(f);
 puts("PINNED_151_SOURCE_SPECIES_STATS_TYPES_NINE_NATIVE_BATTLES_SAVE_PASS");
 return 0;
}
