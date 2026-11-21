#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "yellow_source_event_ir.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
static void item(uint8_t x,uint8_t y,uint8_t bit) {
 YellowEventIR record;
 assert(yellow_ir_at_position(YMapViridianForest,x,y,YEIR_ITEM,&record));
 assert(record.extra==bit);
 assert(yellow_ir_forest_item_mask(x,y)==bit);
 YellowGame game;yellow_init(&game);
 game.map=YMapViridianForest;
 game.story=YStoryPikachuFollows;
 game.screen=YScreenWorld;
 game.forest_pickups=0;
 unsigned matched=0;
 for(uint8_t i=0;i<yellow_npc_count;i++) {
  const YellowNpc* npc=&yellow_npcs[i];
  if(npc->map!=YMapViridianForest||npc->x!=x||npc->y!=y)continue;
  assert(npc->type==YActorPokeBall);
  assert(yellow_npc_visible(&game,npc));
  game.forest_pickups|=bit;
  assert(!yellow_npc_visible(&game,npc));
  matched++;
 }
 assert(matched==1);
}
int main(void) {
 assert(yellow_ir_count()==253);
 unsigned counts[9]={0};
 for(uint16_t i=0;i<yellow_ir_count();i++) {
  YellowEventIR rec={0};
  assert(yellow_ir_at(i,&rec));
  assert(rec.map<YELLOW_MAP_COUNT);
  assert(rec.kind>=YEIR_WARP&&rec.kind<=YEIR_SAFE_OPCODE);
  counts[rec.kind]++;
 }
 assert(counts[YEIR_WARP]==38);
 assert(counts[YEIR_BG]==20);
 assert(counts[YEIR_NPC]==36);
 assert(counts[YEIR_TRAINER]==6);
 assert(counts[YEIR_ITEM]==5);
 assert(counts[YEIR_TEXT_POINTER]==88);
 assert(counts[YEIR_SCRIPT_POINTER]==49);
 assert(counts[YEIR_SAFE_OPCODE]==11);
 item(25,11,1);
 item(12,29,2);
 item(1,31,4);
 assert(yellow_ir_forest_item_mask(0,0)==0);
 assert(yellow_ir_forest_item_mask(1,30)==0);
 YellowEventIR rec;
 assert(yellow_ir_at_position(YMapViridianForest,2,41,YEIR_TRAINER,&rec));
 assert(rec.value==19);
 assert(yellow_ir_at_position(YMapRoute1,9,27,YEIR_BG,&rec));
 assert(yellow_ir_at_position(YMapPallet,5,5,YEIR_WARP,&rec));
 assert(rec.value==1);
 assert(!yellow_ir_at_position(255,25,11,YEIR_ITEM,&rec));
 assert(!yellow_ir_at_position(YMapRoute1,-1,0,YEIR_WARP,&rec));
 assert(!yellow_ir_at_position(YMapRoute1,256,0,YEIR_WARP,&rec));
 assert(!yellow_ir_at_position(YMapRoute1,9,27,255,&rec));
 assert(!yellow_ir_at_position(YMapRoute1,9,27,YEIR_BG,NULL));
 assert(!yellow_ir_at(253,&rec));
 assert(!yellow_ir_at(0,NULL));
 assert(yellow_ir_opcode_supported(YEIR_OP_TEXTBOX));
 assert(yellow_ir_opcode_supported(YEIR_OP_JUMP_TEXTBOX));
 assert(yellow_ir_opcode_supported(YEIR_OP_RETURN));
 assert(!yellow_ir_opcode_supported(0));
 assert(!yellow_ir_opcode_supported(4));
 assert(!yellow_ir_opcode_supported(255));
 YellowSave save;YellowGame player;yellow_init(&player);
 player.map=YMapRoute1;player.x=10;player.y=20;player.story=YStoryPikachuFollows;
 player.has_pikachu=1;player.forest_pickups=3;
 yellow_save_create(&player,&save);
 YellowGame loaded;
 assert(save.version==13);
 assert(yellow_save_load(&loaded,&save));
 assert(loaded.map==YMapRoute1 && loaded.x==10 && loaded.y==20);
 puts("SOURCE_MAP_EVENT_IR_253_STRICT_NPC_TRAINER_ITEM_SCRIPT_SAVE_PASS");
 return 0;
}
