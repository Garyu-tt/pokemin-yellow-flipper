#include <assert.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
int main(void){YellowGame g,h;yellow_init(&g);g.running=true;g.has_pikachu=1;g.story=YStoryPikachuFollows;g.map=YMapMtMoon1F;g.native_zubat_extension=true;g.pika_level=50;g.hp=y_calc_stats(YK_PIKACHU,50).hp;g.experience=y_exp_threshold(YK_PIKACHU,50);g.balls=35;yellow_begin_battle(&g,YK_ZUBAT,10);g.foe_hp=1;g.foe_status=YStatusSleep7;for(unsigned i=0;i<25&&!g.caught_count;i++){assert(g.screen==YScreenRivalBattle);yellow_input(&g,YOk);g.selected=yellow_active_move_count(&g);yellow_input(&g,YOk);}assert(g.caught_count==1);assert(g.caught[0].species==YK_ZUBAT);assert(g.caught[0].moves[0]==YM_LEECH_LIFE);assert(g.caught[0].moves[1]==YM_SUPERSONIC);YellowSave s;yellow_save_create(&g,&s);assert(sizeof(s)==132);assert(yellow_save_load(&h,&s));assert(h.caught_count==1&&h.caught[0].species==YK_ZUBAT);g.map=YMapViridianCenter;g.screen=YScreenPC;assert(yellow_pc_deposit(&g,1));YellowPCSave b;YellowPCBox restore;assert(yellow_pc_save(&g.pc,s.checksum,&b));assert(yellow_pc_restore(&restore,s.checksum,&b));assert(restore.slots[0].species==YK_ZUBAT);puts("ZUBAT_CAPTURE_V13_PC_PASS");return 0;}
