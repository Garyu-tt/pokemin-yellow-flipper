/* Independent owner executable oracle, pinned Route9 Overworld LedgeTiles.
 * This test exercises the actual yellow_input path, not a mocked function.
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
#include "../yellow_pewter_progress.c"
#include "../yellow_atomic_snapshot.c"
#include "../yellow_exact_position_v1.c"

static void ready(YellowGame* g,int x,int y,uint8_t facing) {
    yellow_init(g);g->running=true;g->screen=YScreenWorld;
    g->story=YStoryPikachuFollows;g->has_pikachu=0;
    g->follower_valid=0;g->map=YMapRoute9;g->x=x;g->y=y;
    g->facing=facing;g->steps=0;
}
static void hop(int x,int y,int dx,int dy,uint8_t key,uint8_t st,uint8_t fr) {
    YellowGame g;
    ready(&g,x,y,key);
    assert(yellow_route9_passable(x,y));
    assert(!yellow_route9_passable(x+dx,y+dy));
    assert(yellow_route9_passable(x+2*dx,y+2*dy));
    assert(yellow_route9_tile_id(2*x,2*y+1)==st);
    assert(yellow_route9_tile_id(2*(x+dx),2*(y+dy)+1)==fr);
    yellow_input(&g,(YellowKey)key);
    assert(g.map==YMapRoute9);
    assert(g.x==x+2*dx&&g.y==y+2*dy&&g.steps==2);
}
int main(void) {
    /* The exact source LedgeTiles matrix, including RIGHT 0x1d exception. */
    assert(yellow_route9_ledge_rule(YDown,0x2c,0x37));
    assert(yellow_route9_ledge_rule(YDown,0x39,0x36));
    assert(yellow_route9_ledge_rule(YDown,0x39,0x37));
    assert(yellow_route9_ledge_rule(YLeft,0x2c,0x27));
    assert(yellow_route9_ledge_rule(YLeft,0x39,0x27));
    assert(yellow_route9_ledge_rule(YRight,0x2c,0x0d));
    assert(yellow_route9_ledge_rule(YRight,0x2c,0x1d));
    assert(yellow_route9_ledge_rule(YRight,0x39,0x0d));
    assert(!yellow_route9_ledge_rule(YRight,0x39,0x1d));
    assert(!yellow_route9_ledge_rule(YUp,0x2c,0x37));
    assert(!yellow_route9_ledge_rule(YDown,0x2c,0x36));
    assert(!yellow_route9_ledge_rule(YRight,0x2c,0x27));
    /* Real source Route9 coordinates, atlas tile IDs and collision bitmap. */
    hop(20,4,0,1,YDown,0x2c,0x37);
    hop(20,8,0,1,YDown,0x39,0x36);
    hop(13,10,0,1,YDown,0x39,0x37);
    hop(51,4,0,1,YDown,0x2c,0x37);
    YellowGame g;
    ready(&g,20,4,YLeft);
    yellow_input(&g,YDown); /* turning alone must not hop over the ledge */
    assert(g.x==20&&g.y==4&&g.steps==0&&g.facing==YDown);
    yellow_input(&g,YDown); /* facing now matches; real hop */
    assert(g.x==20&&g.y==6&&g.steps==2);
    ready(&g,20,6,YUp);
    yellow_input(&g,YUp); /* reverse jump forbidden */
    assert(g.x==20&&g.y==6&&g.steps==0);
    ready(&g,20,4,YDown);
    yellow_input(&g,YRight); /* normal unblocked input is 1 cell, not a hop */
    assert(g.x==21&&g.y==4&&g.steps==1);
    /* Same exact-position sidecar and v13 parent snapshot, no ABI changes. */
    ready(&g,20,4,YDown);
    yellow_input(&g,YDown);
    g.has_pikachu=1;g.pika_level=8;
    g.hp=y_calc_stats(YK_PIKACHU,8).hp;
    g.experience=y_exp_threshold(YK_PIKACHU,8);
    YellowAtomicSnapshot snap={0};
    YellowExactPositionV1 position={0};
    YellowGame restored;
    assert(sizeof(YellowSave)==132&&sizeof(YellowExactPositionV1)==28);
    assert(yellow_atomic_make(&g,37,&snap));
    assert(yellow_exact_position_make(&g,&snap,&position));
    assert(yellow_atomic_unpack(&snap,&restored));
    assert(yellow_exact_position_restore(&restored,&snap,&position));
    assert(restored.map==YMapRoute9&&restored.x==20&&restored.y==6);
    puts("ROUTE9_OWNER_SOURCE_BACKED_LEDGE_INPUT_SAVE_PASS");
    return 0;
}
