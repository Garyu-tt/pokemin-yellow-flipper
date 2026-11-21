#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
int main(void){
 FILE* f=fopen("../upstream/pokeyellow/scripts/Route1.asm","rb");
 assert(f);char line[128]={0};unsigned recognized=0;
 while(fgets(line,sizeof(line),f)) {
  if(strstr(line,"call EnableAutoTextBoxDrawing"))recognized|=1u;
  if(recognized==1u && strstr(line,"ret")) {recognized|=2u;break;}
 }
 fclose(f);assert(recognized==3u);
 YellowGame g;yellow_init(&g);
 assert(g.source_script_state==0);
 g.story=YStoryPikachuFollows;g.screen=YScreenWorld;
 g.has_pikachu=1;g.map=YMapPallet;g.x=10;g.y=0;g.follower_valid=0;
 yellow_input(&g,YUp);
 assert(g.map==YMapRoute1 && g.source_script_state==3);
 g.x=10;g.y=35;g.follower_valid=0;g.screen=YScreenWorld;
 yellow_input(&g,YDown);
 assert(g.map==YMapPallet && g.source_script_state==0);
 set_map(&g,YMapRoute1,10,20);
 assert(g.source_script_state==3);
 g.source_script_state=1;
 assert(!source_event_safe_opcode(&g,255));
 assert(!source_event_safe_opcode(&g,YEIR_OP_JUMP_TEXTBOX));
 assert(g.source_script_state==1);
 assert(source_event_safe_opcode(&g,YEIR_OP_RETURN));
 assert(g.source_script_state==3);
 YellowSave saved;yellow_save_create(&g,&saved);
 YellowGame restored;assert(saved.version==13);
 assert(yellow_save_load(&restored,&saved));
 assert(restored.map==YMapRoute1 && restored.source_script_state==3);
 set_map(&g,YMapViridianForest,16,46);
 assert(g.source_script_state==0);
 puts("PINNED_ROUTE1_TEXTBOX_AND_RET_EXECUTE_ON_WORLD_ENTRY_AND_SAVE_RESTORE_PASS");
 return 0;
}
