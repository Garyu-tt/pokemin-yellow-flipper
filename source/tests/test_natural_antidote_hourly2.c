#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_antidote_cure.c"
#include "../yellow_core.c"
#include "../yellow_antidote_game.c"
#include "../yellow_pewter_progress.c"
#include "../yellow_atomic_snapshot.c"
#include "../yellow_exact_position_v1.c"
#include "../yellow_viridian_mart_purchase.c"
#include "../yellow_mart_runtime.c"
#include "../yellow_money_stock_slot.c"
static void world(YellowGame* g){
 yellow_init(g);
 g->story=YStoryPikachuFollows;g->screen=YScreenWorld;
 g->has_pikachu=1;g->hp=10;g->follower_valid=0;
 g->map=YMapViridian;g->x=29;g->y=20;g->facing=YUp;
 g->oak_parcel_flags=YOakParcelAll;
}
static void enter_and_walk(YellowGame* g){
 yellow_input(g,YUp);
 assert(g->map==YMapViridianMart && g->x==3 && g->y==6);
 assert(g->screen==YScreenWorld);
 yellow_input(g,YLeft);assert(g->x==2 && g->y==6);
 yellow_input(g,YUp);assert(g->x==2 && g->y==5);
 yellow_input(g,YLeft);assert(g->x==2 && g->y==5);
}
static void buy_one(YellowGame* g){
 uint8_t before=g->antidotes;
 yellow_input(g,YOk);
 assert(g->screen==YScreenDialog && g->pending==YPendingBuyAntidote);
 yellow_input(g,YOk);
 assert(g->antidotes==before+1u && g->screen==YScreenDialog);
 yellow_input(g,YOk);assert(g->screen==YScreenWorld);
}
int main(void){
 YellowGame g;
 world(&g);
 assert(g.money_bcd[0]==0 && g.money_bcd[1]==0x30 && g.money_bcd[2]==0);
 enter_and_walk(&g);
 yellow_input(&g,YOk);
 assert(g.screen==YScreenDialog && g.pending==YPendingBuyAntidote);
 yellow_input(&g,YBack);
 assert(g.screen==YScreenWorld && g.pending==YPendingNone && g.antidotes==0);
 assert(g.money_bcd[1]==0x30);
 buy_one(&g);
 assert(g.antidotes==1 && g.money_bcd[1]==0x29 && g.money_bcd[2]==0);
 g.pika_status=YStatusPoisoned;
 assert(yellow_game_apply_antidote(&g,0,&g.antidotes));
 assert(g.pika_status==YStatusNone && g.antidotes==0);
 assert(g.money_bcd[1]==0x29);
 YellowMoneyStockSlot side={{0}};
 assert(yellow_money_stock_create(g.money_bcd,g.antidotes,0x12345678u,7u,&side));
 uint8_t funds[3]={0},stock=255;
 assert(yellow_money_stock_restore(&side,0x12345678u,7u,funds,&stock));
 assert(memcmp(funds,g.money_bcd,3)==0 && stock==0);
 side.wire[6]^=1u;
 assert(!yellow_money_stock_restore(&side,0x12345678u,7u,funds,&stock));
 /* Native v13 parent + paired sidecar restore: money is not silently reset. */
 YellowAtomicSnapshot parent={0};
 assert(yellow_atomic_make(&g,8u,&parent));
 YellowMoneyStockSlot paired={{0}};
 assert(yellow_money_stock_create(g.money_bcd,g.antidotes,
        parent.parent.checksum,parent.generation,&paired));
 YellowGame recovered;
 assert(yellow_atomic_unpack(&parent,&recovered));
 assert(yellow_money_stock_restore(&paired,parent.parent.checksum,
        parent.generation,recovered.money_bcd,&recovered.antidotes));
 assert(recovered.money_bcd[1]==0x29 && recovered.antidotes==0);
 assert(!yellow_money_stock_restore(&paired,parent.parent.checksum,
        parent.generation+1u,recovered.money_bcd,&recovered.antidotes));
 /* No parcel: never offer/purchase; no stock injection. */
 world(&g);g.oak_parcel_flags=YOakParcelGot;enter_and_walk(&g);
 yellow_input(&g,YOk);
 assert(g.screen==YScreenDialog && g.pending==YPendingNone);
 yellow_input(&g,YOk);assert(g.screen==YScreenWorld && g.antidotes==0);
 /* Corruption must never purchase even if a confirmation is pending. */
 world(&g);enter_and_walk(&g);yellow_input(&g,YOk);
 assert(g.pending==YPendingBuyAntidote);
 g.money_integrity_error=true;yellow_input(&g,YOk);
 assert(g.antidotes==0 && g.money_bcd[1]==0x30);
 yellow_input(&g,YOk);
 /* Check stock cap and insufficient funds without altering the account. */
 world(&g);enter_and_walk(&g);g.antidotes=99;yellow_input(&g,YOk);
 yellow_input(&g,YOk);assert(g.antidotes==99 && g.money_bcd[1]==0x30);
 yellow_input(&g,YOk);
 world(&g);enter_and_walk(&g);g.money_bcd[1]=0;g.money_bcd[2]=0x99;
 yellow_input(&g,YOk);yellow_input(&g,YOk);
 assert(g.antidotes==0 && g.money_bcd[2]==0x99);
 yellow_input(&g,YOk);
 /* A failed purchase cannot silently cure poison. */
 assert(g.pika_status==YStatusNone && g.antidotes==0);
 puts("NATURAL_MART_ANTIDOTE_DOOR_WALK_TALK_CANCEL_BUY_CURE_GATES_SIDECAR_PASS");
 return 0;
}
