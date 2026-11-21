#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
static void prepare(YellowGame* g) {
    yellow_init(g);
    g->running=true;g->screen=YScreenWorld;g->story=YStoryPikachuFollows;
    g->has_pikachu=1;g->map=YMapMtMoon1F;g->facing=YRight;g->follower_valid=0;
    g->pika_level=75;
    g->experience=y_exp_threshold(YK_PIKACHU,75);
    g->hp=y_calc_stats(YK_PIKACHU,75).hp;
    g->balls=40;
}
static unsigned slot_for_roll(uint8_t roll) {
    unsigned sum=0;
    for(unsigned i=0;i<10;i++) {
        sum+=encounter_weights[i];
        if(roll<sum)return i;
    }
    return 9;
}
static uint32_t matching_seed(unsigned slot) {
    for(uint32_t seed=1;seed<1000000;seed++) {
        YellowGame temp={0};temp.rng=seed;
        if((next_rng(&temp)&255u)>=YELLOW_MTMOON_WILD_RATE)continue;
        if(slot_for_roll((uint8_t)(next_rng(&temp)&255u))==slot)
            return seed;
    }
    assert(0);return 0;
}
static void metadata_and_failclosed(void) {
    static const uint8_t dex[]={41,41,74,41,41,41,74,41,27,35};
    static const uint8_t level[]={8,9,10,6,7,10,10,11,12,11};
    static const uint8_t playable[]={0,0,1,0,0,0,1,0,1,0};
    assert(YELLOW_MTMOON_WILD_RATE==10 && YELLOW_MTMOON_WILD_SLOTS==10);
    unsigned enabled=0;
    YellowGame g;prepare(&g);
    for(unsigned i=0;i<10;i++) {
        YellowMtMoonSlot o={0};
        assert(yellow_mtmoon_original_wild_slot(i,&o));
        assert(o.original_dex_id==dex[i] && o.level==level[i]);
        assert(o.native_supported==playable[i]);
        uint8_t kind=200,lev=201;
        bool native=yellow_wild_slot(&g,YMapMtMoon1F,(uint8_t)i,&kind,&lev);
        assert(native==playable[i]);
        if(native) {
            assert(lev==level[i]);
            assert(kind==(i==8?YK_SANDSHREW:YK_GEODUDE));
            enabled++;
        } else {
            assert(kind==200 && lev==201);
        }
    }
    assert(enabled==3);
    assert(!yellow_mtmoon_original_wild_slot(10,&(YellowMtMoonSlot){0}));
    assert(!yellow_mtmoon_original_wild_slot(0,NULL));
    assert(!yellow_mtmoon_native_wild_slot(10,(uint8_t[1]){0},(uint8_t[1]){0}));
    assert(!yellow_mtmoon_native_wild_slot(2,NULL,(uint8_t[1]){0}));
    assert(!yellow_mtmoon_native_wild_slot(2,(uint8_t[1]){0},NULL));
    assert(y_learned_move_at(YK_SANDSHREW,10)==YM_SAND_ATTACK);
    assert(!y_move_available(YK_SANDSHREW,9,YM_SAND_ATTACK));
    assert(y_move_available(YK_SANDSHREW,12,YM_SAND_ATTACK));
    uint8_t moves[4];
    y_build_moveset(YK_SANDSHREW,12,moves);
    assert(moves[0]==YM_SCRATCH && moves[1]==YM_SAND_ATTACK);
    assert(y_wild_move(YK_SANDSHREW,12,1)==YM_SAND_ATTACK);
    assert(y_wild_move(YK_GEODUDE,10,0)==YM_TACKLE);
    assert(y_wild_move(YK_GEODUDE,10,1)==YM_NONE);
    assert(y_evolution_at(YK_SANDSHREW,22)==YK_SANDSHREW); /* missing Sandslash */
    assert(y_evolution_at(YK_GEODUDE,25)==YK_GEODUDE); /* missing Graveler */
}
static void find_safe_step(int* x,int* y) {
    for(int row=2;row<33;row++)
        for(int col=2;col<37;col++)
            if(yellow_mtmoon1f_passable(col,row)&&
               yellow_mtmoon1f_passable(col+1,row) &&
               yellow_mtmoon1f_passable(col+2,row)) {
                *x=col;*y=row;return;
            }
    assert(0);
}
static void real_input_encounter(unsigned slot) {
    YellowGame g;prepare(&g);
    int x=0,y=0;find_safe_step(&x,&y);
    g.x=(int8_t)x;g.y=(int8_t)y;
    g.rng=matching_seed(slot);
    yellow_input(&g,YRight);
    assert(g.x==x+1 && g.y==y && g.steps==1);
    bool supported=slot==2||slot==6||slot==8;
    assert((g.screen==YScreenRivalBattle)==supported);
    if(supported) {
        assert(g.foe_kind==(slot==8?YK_SANDSHREW:YK_GEODUDE));
        assert(g.foe_level==(slot==8?12:10));
    } else assert(g.screen==YScreenWorld);
}
static void catch_save_and_pc(unsigned kind,unsigned level) {
    YellowGame g,from_save;prepare(&g);
    g.x=14;g.y=34;
    yellow_begin_battle(&g,kind,level);
    assert(g.screen==YScreenRivalBattle && g.foe_kind==kind);
    g.foe_hp=1;g.foe_status=YStatusSleep7;
    unsigned trials=0;
    while(g.caught_count==0 && trials++<20) {
        g.foe_hp=1;
        assert(g.screen==YScreenRivalBattle);
        yellow_input(&g,YOk);
        assert(g.screen==YScreenFightMenu);
        g.selected=yellow_active_move_count(&g);
        yellow_input(&g,YOk);
    }
    assert(g.caught_count==1 && g.screen==YScreenDialog);
    assert(g.caught[0].species==kind && g.caught[0].level==level);
    assert(yellow_dex_caught(&g.dex,yellow_dex_number(kind)));
    yellow_input(&g,YOk);
    assert(g.screen==YScreenWorld);
    YellowSave parent;
    yellow_save_create(&g,&parent);
    assert(parent.version==13 && parent.map==YMapRoute2 && sizeof(parent)==132);
    assert(yellow_save_load(&from_save,&parent));
    assert(from_save.caught_count==1 && from_save.caught[0].species==kind);
    assert(yellow_valid_moves(kind,level,from_save.caught[0].moves,from_save.caught[0].pp));
    g.map=YMapViridianCenter;g.screen=YScreenPC;
    assert(yellow_pc_deposit(&g,1));
    assert(g.caught_count==0 && g.pc.count==1);
    YellowPCSave pc={0};YellowPCBox restored={0};
    assert(yellow_pc_save(&g.pc,parent.checksum,&pc));
    assert(yellow_pc_restore(&restored,parent.checksum,&pc));
    assert(restored.count==1 && restored.slots[0].species==kind);
    pc.checksum^=1u;
    assert(!yellow_pc_restore(&restored,parent.checksum,&pc));
    assert(restored.count==1);
    assert(yellow_pc_withdraw(&g,0));
    assert(g.pc.count==0 && g.caught_count==1 && g.caught[0].species==kind);
}
int main(void) {
    metadata_and_failclosed();
    for(unsigned slot=0;slot<10;slot++)real_input_encounter(slot);
    catch_save_and_pc(YK_GEODUDE,10);
    catch_save_and_pc(YK_SANDSHREW,12);
    puts("CAVE_10_SOURCE_SLOTS_3_NATIVE_ALL_INPUT_2_CAUGHT_V13_PC_CHECKSUM_PASS");
    return 0;
}
