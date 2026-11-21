#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
static void viewer_is_read_only_and_original(void){
 YellowGame g;yellow_init(&g);g.running=true;
 g.screen=YScreenWorld;g.story=YStoryPikachuFollows;
 g.has_pikachu=1;g.map=YMapViridianCenter;g.x=3;g.y=6;
 assert(!strcmp(yellow_dex_name(1),"BULBASAUR"));
 assert(!strcmp(yellow_dex_name(25),"PIKACHU"));
 assert(!strcmp(yellow_dex_name(133),"EEVEE"));
 assert(!strcmp(yellow_dex_name(151),"MEW"));
 assert(!yellow_dex_name(0) && !yellow_dex_name(152));
 for(unsigned n=1;n<=151;n++)assert(yellow_dex_name((uint8_t)n));
 (void)yellow_dex_mark_seen(&g.dex,133);
 (void)yellow_dex_mark_caught(&g.dex,25);
 YellowDex before=g.dex;
 YellowSave main_before,main_after;
 yellow_save_create(&g,&main_before);
 yellow_input(&g,YBack);
 assert(g.screen==YScreenMenu && g.selected==0);
 YellowGame previous_route=g;
 previous_route.map=YMapRoute1;
 yellow_input(&previous_route,YRight);
 assert(previous_route.screen==YScreenMenu); /* old route contract */
 yellow_input(&g,YRight);
 assert(g.screen==YScreenPokedex && g.selected==1);
 yellow_input(&g,YUp);assert(g.selected==1);
 yellow_input(&g,YRight);assert(g.selected==11);
 yellow_input(&g,YLeft);assert(g.selected==1);
 for(unsigned i=0;i<150;i++)yellow_input(&g,YDown);
 assert(g.selected==151);
 yellow_input(&g,YDown);assert(g.selected==151);
 yellow_input(&g,YRight);assert(g.selected==151);
 for(unsigned i=0;i<15;i++)yellow_input(&g,YLeft);
 assert(g.selected==1);
 for(unsigned i=0;i<132;i++)yellow_input(&g,YDown);
 assert(g.selected==133);
 assert(yellow_dex_seen(&g.dex,g.selected));
 assert(!yellow_dex_caught(&g.dex,g.selected));
 yellow_input(&g,YOk);assert(g.selected==133);
 yellow_input(&g,YBack);
 assert(g.screen==YScreenMenu && g.selected==0);
 yellow_input(&g,YBack);assert(g.screen==YScreenWorld);
 assert(!memcmp(&before,&g.dex,sizeof(before)));
 yellow_save_create(&g,&main_after);
 assert(!memcmp(&main_before,&main_after,sizeof(main_before)));
}
static void integrated_pc_roundtrip_preserves_pokedex(void) {
 YellowGame g;yellow_init(&g);
 g.running=true;g.screen=YScreenPC;g.map=YMapViridianCenter;
 g.story=YStoryPikachuFollows;g.has_pikachu=1;g.follower_valid=0;
 YCapturedMon member={0};
 member.species=YK_PIDGEY;member.level=3;member.hp=y_calc_stats(member.species,3).hp;
 member.experience=y_exp_threshold(member.species,3);
 y_build_moveset(member.species,member.level,member.moves);
 for(unsigned i=0;i<4;i++)member.pp[i]=y_max_pp(member.moves[i]);
 g.caught[0]=member;g.caught_count=1;
 (void)yellow_dex_mark_caught(&g.dex,yellow_dex_number(YK_PIKACHU));
 (void)yellow_dex_mark_caught(&g.dex,yellow_dex_number(YK_PIDGEY));
 assert(yellow_pc_deposit(&g,1));
 assert(g.pc.count==1 && g.caught_count==0);
 assert(yellow_dex_caught(&g.dex,16));
 YellowSave parent;yellow_save_create(&g,&parent);
 YellowPCSave pc;YellowDexSave dex;
 assert(yellow_pc_save(&g.pc,parent.checksum,&pc));
 assert(yellow_dex_save(&g.dex,parent.checksum,&dex));
 YellowGame loaded;
 assert(yellow_save_load(&loaded,&parent));
 assert(yellow_pc_restore(&loaded.pc,parent.checksum,&pc));
 assert(yellow_dex_restore(&loaded.dex,parent.checksum,&dex));
 assert(yellow_dex_caught(&loaded.dex,16));
 assert(yellow_dex_caught(&loaded.dex,25));
 assert(yellow_pc_withdraw(&loaded,0)==false); /* must use PC screen */
 loaded.screen=YScreenPC;loaded.map=YMapViridianCenter;
 assert(yellow_pc_withdraw(&loaded,0));
 assert(loaded.caught_count==1 && loaded.pc.count==0);
 assert(loaded.caught[0].species==YK_PIDGEY);
 assert(yellow_dex_caught(&loaded.dex,16));
 YellowDex original=loaded.dex;
 dex.parent_checksum^=1u;
 assert(!yellow_dex_restore(&loaded.dex,parent.checksum,&dex));
 assert(!memcmp(&original,&loaded.dex,sizeof(original)));
}
int main(void){
 viewer_is_read_only_and_original();
 integrated_pc_roundtrip_preserves_pokedex();
 puts("DEX_VIEW_151_ENTRIES_READONLY_PC_BOX_INTEGRATION_LEGACY_V13_PASS");
 return 0;
}
