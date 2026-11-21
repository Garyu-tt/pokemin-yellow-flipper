#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
static YCapturedMon make_member(uint8_t species,uint8_t level) {
    YCapturedMon m={0};
    m.species=species;m.level=level;
    m.hp=y_calc_stats(species,level).hp;
    m.experience=y_exp_threshold(species,level);
    y_build_moveset(species,level,m.moves);
    for(unsigned i=0;i<4;i++)m.pp[i]=y_max_pp(m.moves[i]);
    assert(yellow_pc_mon_valid(&m));
    return m;
}
static void setup(YellowGame* g) {
    yellow_init(g);
    g->running=true;g->screen=YScreenWorld;
    g->map=YMapViridianCenter;g->x=3;g->y=6;
    g->story=YStoryPikachuFollows;g->has_pikachu=1;
    g->follower_valid=0;
    g->caught[0]=make_member(YK_PIDGEY,3);
    g->caught[1]=make_member(YK_RATTATA,4);
    g->caught_count=2;g->active_party=2;
    (void)yellow_dex_mark_caught(&g->dex,yellow_dex_number(YK_PIDGEY));
    (void)yellow_dex_mark_caught(&g->dex,yellow_dex_number(YK_RATTATA));
}
static void player_pc_input_roundtrip(void) {
    YellowGame g;setup(&g);
    yellow_input(&g,YBack);
    assert(g.screen==YScreenMenu && g.selected==0);
    for(unsigned i=0;i<5;i++)yellow_input(&g,YDown);
    assert(g.selected==5);
    yellow_input(&g,YOk);
    assert(g.screen==YScreenPC && g.pc_mode==0 && g.selected==0);
    yellow_input(&g,YOk);
    assert(g.pc_mode==1 && g.selected==0);
    yellow_input(&g,YOk);
    assert(g.pc.count==1 && g.pc.slots[0].species==YK_PIDGEY);
    assert(g.caught_count==1 && g.caught[0].species==YK_RATTATA);
    assert(g.active_party==1 && g.caught[1].species==0);
    assert(yellow_party_size(&g)==2);
    yellow_input(&g,YBack); /* deposit list -> top menu */
    assert(g.pc_mode==0);
    yellow_input(&g,YDown);
    assert(g.selected==1);
    yellow_input(&g,YOk);
    assert(g.pc_mode==2);
    yellow_input(&g,YOk);
    assert(g.pc.count==0 && g.caught_count==2);
    assert(g.caught[1].species==YK_PIDGEY);
    assert(g.active_party==1);
    yellow_input(&g,YBack);
    yellow_input(&g,YBack);
    assert(g.screen==YScreenMenu && g.selected==5);
    yellow_input(&g,YBack);
    assert(g.screen==YScreenWorld);
    assert(yellow_dex_caught(&g.dex,16)&&yellow_dex_caught(&g.dex,19));
}
static void box_sidecar_checks(void) {
    YellowGame g;setup(&g);
    g.screen=YScreenPC;
    assert(yellow_pc_deposit(&g,1));
    YellowSave parent;
    yellow_save_create(&g,&parent);
    assert(parent.version==13 && sizeof(parent)==132);
    YellowPCSave saved;
    assert(sizeof(saved)==336);
    assert(yellow_pc_save(&g.pc,parent.checksum,&saved));
    assert(saved.magic==YELLOW_PC_MAGIC && saved.version==1 && saved.count==1);
    YellowPCBox restored={0};
    assert(yellow_pc_restore(&restored,parent.checksum,&saved));
    assert(restored.count==1 && restored.slots[0].species==YK_PIDGEY);
    YellowPCBox copy=restored;
    YellowPCSave damaged=saved;
    damaged.checksum^=1u;
    assert(!yellow_pc_restore(&restored,parent.checksum,&damaged));
    assert(memcmp(&copy,&restored,sizeof(restored))==0);
    assert(!yellow_pc_restore(&restored,parent.checksum+1u,&saved));
    damaged=saved;damaged.count=21;
    assert(!yellow_pc_restore(&restored,parent.checksum,&damaged));
    damaged=saved;damaged.slots[1][2]=1;
    assert(!yellow_pc_restore(&restored,parent.checksum,&damaged));
    damaged=saved;damaged.slots[0][0]=YK_EEVEE;
    damaged.checksum=yellow_pc_hash(&damaged);
    assert(!yellow_pc_restore(&restored,parent.checksum,&damaged));
    assert(memcmp(&copy,&restored,sizeof(restored))==0);
    /* v13 parent serialization unaffected by PC storage. */
    YellowSave repeat;yellow_save_create(&g,&repeat);
    assert(memcmp(&parent,&repeat,sizeof(parent))==0);
    YellowGame loaded;
    assert(yellow_save_load(&loaded,&parent));
    assert(loaded.caught_count==1); /* legacy save includes party only */
    assert(loaded.pc.count==0); /* opt-in sidecar recovered separately */
}
static void overflow_and_invalid_state(void) {
    YellowGame g;setup(&g);
    assert(!yellow_pc_deposit(&g,0)); /* starter Pikachu cannot be deposited */
    g.screen=YScreenPC;
    assert(!yellow_pc_deposit(&g,0));
    for(unsigned i=0;i<YELLOW_PC_CAPACITY;i++)
        g.pc.slots[i]=make_member(YK_PIDGEY,4);
    g.pc.count=YELLOW_PC_CAPACITY;
    assert(!yellow_pc_deposit(&g,1));
    assert(g.caught_count==2);
    assert(yellow_pc_save(&g.pc,1234u,&(YellowPCSave){0}));
    g.pc.slots[0].species=255;
    assert(!yellow_pc_save(&g.pc,1234u,&(YellowPCSave){0}));
    assert(!yellow_pc_withdraw(&g,0));
    g.pc.slots[0]=make_member(YK_PIDGEY,4);
    assert(!yellow_pc_withdraw(&g,20));
    g.caught_count=YELLOW_CAPTURE_SLOTS;
    assert(!yellow_pc_withdraw(&g,0)); /* party full */
    g.caught_count=2;
    g.map=YMapRoute1;
    assert(!yellow_pc_withdraw(&g,0)); /* no remote PC teleport */
}
int main(void) {
    player_pc_input_roundtrip();
    box_sidecar_checks();
    overflow_and_invalid_state();
    puts("PC_20_SLOT_GAME_INPUT_DEPOSIT_WITHDRAW_SAVE_CRC_V13_PASS");
    return 0;
}
