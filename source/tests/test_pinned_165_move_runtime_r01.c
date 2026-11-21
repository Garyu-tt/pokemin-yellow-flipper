#include <assert.h>
#include <stdio.h>
#include "yellow_battle.h"
#include "yellow_source_moves.h"
#include "pydp_reader.h"
#include "../yellow_battle.c"
#include "../pydp_reader.c"
static int from_file(void* c,uint32_t where,uint8_t* out,size_t n) {
 FILE* f=(FILE*)c;
 if(!n||n>128||fseek(f,(long)where,SEEK_SET))return 0;
 return fread(out,1,n,f)==n;
}
int main(void) {
 FILE* f=fopen("../../../bulk-pokeyellow-r01/pokeyellow_data.pydb","rb");
 assert(f);PydpPack original;
 assert(pydp_open(&original,from_file,f)&&pydp_verify(&original));
 unsigned checked=0,active=0;
 assert(y_source_supported_native_moves()==19);
 for(unsigned id=1;id<=165;id++) {
  YSourceMove data;PydpMove pinned;
  assert(y_source_move((uint8_t)id,&data));
  assert(pydp_move(&original,(uint8_t)id,&pinned));
  assert(data.id==pinned.id && data.power==pinned.power);
  /* The PYDP pack uses alphabetically sorted type/effect dictionaries,
     while this source table uses original Gen1 raw numeric type/effect IDs. */
  assert(pinned.type_id<16 && pinned.effect_id<82);
  assert(data.accuracy==pinned.accuracy);
  assert(data.pp==pinned.pp);
  uint8_t native=255;
  if(y_source_native_index((uint8_t)id,&native)) {
   assert(native<19);
   const YMoveInfo* move=y_move_info(native);
   assert(move&&move->power==data.power && move->accuracy==data.accuracy);
   assert(move->type==data.type && move->pp==data.pp);
   assert(y_max_pp(native)==data.pp);
   active++;
  } else assert(native==255);
  checked++;
 }
 assert(checked==165 && active==19);
 YSourceMove thunder,spore;
 assert(y_source_move(84,&thunder));
 assert(thunder.type==23 && thunder.effect==6);
 assert(y_source_move(79,&spore));
 assert(spore.type==22 && spore.effect==32);
 assert(y_source_move(165,&(YSourceMove){0}));
 assert(!y_source_native_index(165,&(uint8_t){0}));
 YSourceMove invalid={0};
 assert(!y_source_move(0,&invalid));
 assert(!y_source_move(166,&invalid));
 assert(!y_source_move(1,NULL));
 assert(!y_source_native_move(19,&invalid));
 assert(!y_source_native_move(255,&invalid));
 assert(!y_source_native_index(0,&(uint8_t){0}));
 assert(!y_source_native_index(1,NULL));
 assert(y_move_info(19)==NULL);
 assert(y_max_pp(YM_NONE)==0);
 puts("ORIGINAL_165_MOVES_PARAMETERS_EFFECTS_19_ACTIVE_PP_PARITY_AND_GUARDS_PASS");
 fclose(f);return 0;
}
