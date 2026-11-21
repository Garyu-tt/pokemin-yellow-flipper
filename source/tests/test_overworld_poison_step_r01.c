/* Owner-created independent R0.12-C3 four-step overworld poison oracle.
 * Pinned pret/pokeyellow engine/events/poison.asm, actual yellow_input.
 * Partial mechanic only: original blackout/whiteout not accepted here.
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
#include "../yellow_pewter_progress.c"
#include "../yellow_atomic_snapshot.c"
#include "../yellow_exact_position_v1.c"

static void ready(YellowGame* g) {
    yellow_init(g);
    g->story=YStoryPikachuFollows;g->has_pikachu=1;
    g->screen=YScreenWorld;g->map=YMapRoute9;g->follower_valid=0;
    g->hp=yellow_player_max_hp(g);g->steps=0;
    /* Choose real connected passable cells from the original Route9 bitmap,
     * not invented movement coordinates. Stay away from source map boundaries.
     */
    for(int y=2;y<yellow_map_h(YMapRoute9)-2;y++)
      for(int x=3;x<yellow_map_w(YMapRoute9)-8;x++) {
        int valid=1;
        for(int i=0;i<7;i++)
            if(!yellow_walkable(g,x+i,y))valid=0;
        if(valid) {g->x=x;g->y=y;return;}
      }
    assert(!"original route9 seven-cell passable strip not found");
}
static void mon(YellowGame* g,int i,uint8_t species,uint8_t level,
                uint8_t hp,uint8_t status) {
    YCapturedMon* m=&g->caught[i];memset(m,0,sizeof(*m));
    m->species=species;m->level=level;m->hp=hp;m->status=status;
    m->experience=y_exp_threshold(species,level);
    y_build_moveset(species,level,m->moves);
    for(unsigned j=0;j<YELLOW_MOVE_SLOTS;j++)m->pp[j]=y_max_pp(m->moves[j]);
}
static void four_step_party(void) {
    YellowGame g;ready(&g);
    g.pika_status=YStatusToxic2;
    mon(&g,0,YK_PIDGEY,5,5,YStatusPoisoned);
    mon(&g,1,YK_RATTATA,4,0,YStatusPoisoned);
    g.caught_count=2;
    uint8_t hp=g.hp;int8_t x=g.x;
    for(int i=1;i<=4;i++) {
        yellow_input(&g,YRight);
        assert(g.x==x+i && g.steps==(uint32_t)i);
        assert(g.hp==(uint8_t)(hp-(i==4)));
        assert(g.caught[0].hp==(uint8_t)(5-(i==4)));
        assert(g.caught[1].hp==0);
        assert(g.pika_status==YStatusToxic2); /* no battle Toxic ramp */
    }
    assert(g.screen==YScreenWorld);
    puts("SOURCE_GEN1_FOUR_STEPS_MULTI_PARTY_TOXIC_COUNTER_UNCHANGED_PASS");
}
static void no_false_steps(void) {
    YellowGame g;ready(&g);
    g.pika_status=YStatusPoisoned;g.steps=3;
    uint8_t hp=g.hp;int8_t x=g.x;
    yellow_input(&g,YBack); /* world -> menu */
    assert(g.screen==YScreenMenu);
    yellow_input(&g,YRight); /* menu input, no world movement */
    assert(g.steps==3 && g.hp==hp);
    yellow_input(&g,YBack);
    assert(g.screen==YScreenWorld);
    /* A facing change on a blocked interior collision must not tick. */
    int found=0;
    for(int y=2;y<yellow_map_h(YMapRoute9)-2&&!found;y++)
      for(int xx=2;xx<yellow_map_w(YMapRoute9)-2&&!found;xx++)
        if(yellow_walkable(&g,xx,y)&&!yellow_walkable(&g,xx-1,y)) {
            g.x=xx;g.y=y;g.follower_valid=0;g.facing=YRight;
            uint32_t steps=g.steps;
            yellow_input(&g,YLeft);
            assert(g.x==xx && g.y==y && g.steps==steps && g.hp==hp);
            found=1;
        }
    assert(found);
    /* Unpoisoned never loses HP on an otherwise valid four-step boundary. */
    ready(&g);g.steps=3;g.pika_status=YStatusNone;hp=g.hp;x=g.x;
    yellow_input(&g,YRight);
    assert(g.x==x+1 && g.steps==4 && g.hp==hp);
    puts("BLOCKED_MENU_UNPOISONED_NEGATIVE_CONTROLS_PASS");
}
static void faint_and_v13(void) {
    YellowGame g,loaded;YellowSave saved;ready(&g);
    mon(&g,0,YK_PIDGEY,5,10,YStatusNone);g.caught_count=1;
    g.hp=1;g.pika_status=YStatusPoisoned;g.steps=3;
    yellow_input(&g,YRight);
    assert(g.steps==4 && g.hp==0 && g.pika_status==YStatusNone);
    assert(g.caught[0].hp==10 && g.caught[0].status==YStatusNone);
    yellow_save_create(&g,&saved);
    assert(saved.version==13 && sizeof(YellowSave)==132);
    assert(yellow_save_load(&loaded,&saved));
    assert(loaded.hp==0 && loaded.pika_status==YStatusNone);
    assert(loaded.caught_count==1 && loaded.caught[0].hp==10);
    assert(loaded.active_party==1 && loaded.steps==4);
    puts("POISON_FAINT_WITH_SURVIVING_PARTY_V13_RELOAD_PASS");
}
static void wrap_boundary(void) {
    YellowGame g;ready(&g);
    g.steps=65535u;g.pika_status=YStatusPoisoned;g.hp=10;
    yellow_input(&g,YRight);
    assert(g.steps==0 && g.hp==9 && g.pika_status==YStatusPoisoned);
    puts("NATIVE_U16_STEP_WRAP_FOUR_STEP_CADENCE_PASS");
}
static void real_connection_and_two_step_ledge(void) {
    YellowGame g;ready(&g);
    g.x=59;g.y=8;g.facing=YRight;g.steps=3;
    g.pika_status=YStatusPoisoned;g.hp=10;
    assert(yellow_route9_passable(59,8));
    yellow_input(&g,YRight);
    assert(g.map==YMapRoute10 && g.x==0 && g.y==8);
    assert(g.steps==4 && g.hp==9);
    yellow_input(&g,YLeft);
    assert(g.map==YMapRoute9 && g.x==59 && g.y==8);
    assert(g.steps==5 && g.hp==9);
    ready(&g);g.x=20;g.y=4;g.facing=YDown;
    g.steps=2;g.pika_status=YStatusPoisoned;g.hp=10;
    yellow_input(&g,YDown);
    assert(g.x==20 && g.y==6 && g.steps==4 && g.hp==9);
    yellow_input(&g,YUp); /* Reverse ledge is blocked. */
    assert(g.x==20 && g.y==6 && g.steps==4 && g.hp==9);
    puts("REAL_ROUTE9_ROUTE10_BOUNDARY_AND_TWO_STEP_LEDGE_POISON_PASS");
}
int main(void) {
    four_step_party();no_false_steps();faint_and_v13();wrap_boundary();
    real_connection_and_two_step_ledge();
    puts("R012C3_POISON_STEP_PARTIAL_MECHANIC_PASS_NO_BLACKOUT_ACCEPTANCE");
    return 0;
}
