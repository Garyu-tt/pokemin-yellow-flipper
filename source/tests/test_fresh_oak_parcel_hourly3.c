/* Real native YellowKey sequence from yellow_init, no story/map/parcel/stock injection.
 * Battle randomness follows the production fixed seed. The poison state is a
 * separately labelled fixture after the entire original quest and purchase. */
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
typedef struct {int x,y,prev,dir;} Node;
static void dialog(YellowGame* g){
  int guard=20;
  while(g->screen==YScreenDialog && guard-->0)yellow_input(g,YOk);
  assert(guard>0);
}
static void fight(YellowGame* g){
  int guard=60;
  while((g->screen==YScreenRivalBattle||g->screen==YScreenFightMenu) && guard-->0){
    if(g->screen==YScreenRivalBattle)yellow_input(g,YOk);
    else {assert(g->selected==0);yellow_input(g,YOk);}
  }
  assert(guard>0);
  if(g->screen==YScreenDialog)dialog(g);
}
static void route(YellowGame* g,int tx,int ty){
  int width=yellow_map_w(g->map),height=yellow_map_h(g->map);
  assert(width>0 && height>0 && width*height<=2048);
  assert(tx>=0 && ty>=0 && tx<width && ty<height);
  Node nodes[2048];int seen[2048];for(int i=0;i<2048;i++)seen[i]=-1;
  int count=1,end=-1;nodes[0]=(Node){g->x,g->y,-1,-1};
  seen[g->y*width+g->x]=0;
  const int dirs[4][2]={{0,-1},{0,1},{-1,0},{1,0}};
  for(int i=0;i<count;i++){
    Node n=nodes[i];if(n.x==tx&&n.y==ty){end=i;break;}
    for(int k=0;k<4;k++){
      int x=n.x+dirs[k][0],y=n.y+dirs[k][1];
      if(x<0||y<0||x>=width||y>=height||seen[y*width+x]>=0)continue;
      if(!yellow_walkable(g,x,y))continue;
      assert(count<2048);seen[y*width+x]=count;
      nodes[count++]=(Node){x,y,i,k};
    }
  }
  if(end<0)fprintf(stderr,"NO_PATH map=%u %d,%d -> %d,%d\n",g->map,g->x,g->y,tx,ty);
  assert(end>=0);
  int path[2048],length=0;
  while(nodes[end].prev>=0){path[length++]=nodes[end].dir;end=nodes[end].prev;}
  uint8_t start_map=g->map;
  while(length--){
    YellowKey key=(YellowKey)path[length];
    int x=g->x,y=g->y;
    yellow_input(g,key);
    if(g->screen==YScreenRivalBattle||g->screen==YScreenFightMenu)fight(g);
    if(g->screen==YScreenDialog)dialog(g);
    if(g->x==x && g->y==y && g->map==start_map)yellow_input(g,key);
    if(g->screen==YScreenRivalBattle||g->screen==YScreenFightMenu)fight(g);
    if(g->screen==YScreenDialog)dialog(g);
    if(g->map!=start_map || g->screen!=YScreenWorld){
      fprintf(stderr,"ROUTE_INTERRUPTED map=%u pos=%d,%d screen=%u\n",g->map,g->x,g->y,g->screen);
      assert(0);
    }
  }
  if(g->x!=tx||g->y!=ty)fprintf(stderr,"ROUTE_MISMATCH map=%u %d,%d -> %d,%d\n",g->map,g->x,g->y,tx,ty);
  assert(g->x==tx&&g->y==ty);
}
static void north(YellowGame* g,uint8_t next){
  yellow_input(g,YUp);
  assert(g->map==next);
}
static void south(YellowGame* g,uint8_t next){
  yellow_input(g,YDown);
  assert(g->map==next);
}
static void to_viridian(YellowGame* g){
  assert(g->map==YMapPallet);
  route(g,10,0);north(g,YMapRoute1);
  route(g,10,0);north(g,YMapViridian);
}
static void to_pallet(YellowGame* g){
  assert(g->map==YMapViridian);
  route(g,20,35);south(g,YMapRoute1);
  route(g,10,35);south(g,YMapPallet);
}
static void enter_lab(YellowGame* g){
  assert(g->map==YMapPallet);
  route(g,12,12);north(g,YMapOaksLab);
}
static void exit_lab(YellowGame* g){
  assert(g->map==YMapOaksLab);
  route(g,4,10);south(g,YMapPallet);
}
int main(void){
  YellowGame g;yellow_init(&g);
  g.native_mart_intro_extension=true; /* same opt-in as actual yellow_flipper.c */
  assert(g.map==YMapRedsHouse && g.story==YStoryBeforeOak && g.oak_parcel_flags==0);
  assert(g.antidotes==0 && g.money_bcd[1]==0x30);
  dialog(&g);route(&g,2,6);south(&g,YMapPallet);
  route(&g,10,1);yellow_input(&g,YUp);
  assert(g.screen==YScreenDialog);dialog(&g);
  assert(g.screen==YScreenOakEncounter);
  yellow_input(&g,YOk);dialog(&g);dialog(&g);
  assert(g.screen==YScreenWorld && g.map==YMapOaksLab);
  route(&g,7,4);yellow_input(&g,YUp);yellow_input(&g,YOk);
  dialog(&g);dialog(&g);fight(&g);
  if(g.screen==YScreenDialog)dialog(&g);
  assert(g.story==YStoryPikachuFollows && g.has_pikachu);
  printf("FRESH_RIVAL_PASS hp=%u steps=%u\n",g.hp,g.steps);
  exit_lab(&g);to_viridian(&g);
  assert(g.oak_parcel_flags==0 && g.antidotes==0);
  route(&g,29,20);north(&g,YMapViridianMart);
  assert(g.screen==YScreenDialog && g.oak_parcel_flags==0);
  dialog(&g);dialog(&g);
  assert(g.screen==YScreenWorld && g.oak_parcel_flags==YOakParcelGot);
  printf("FRESH_MART_PARCEL_PASS map=%u steps=%u\n",g.map,g.steps);
  route(&g,2,5);yellow_input(&g,YLeft);yellow_input(&g,YOk);
  assert(g.screen==YScreenDialog && g.pending==YPendingNone);
  assert(g.antidotes==0 && g.money_bcd[1]==0x30);
  dialog(&g);assert(g.screen==YScreenWorld);
  printf("FRESH_PRE_DELIVERY_PURCHASE_DENIED_PASS\n");
  route(&g,3,6);south(&g,YMapViridian);
  to_pallet(&g);enter_lab(&g);
  route(&g,5,3);yellow_input(&g,YUp);yellow_input(&g,YOk);
  assert(g.pending==YPendingDeliverOakParcel && g.oak_parcel_flags==YOakParcelGot);
  dialog(&g);
  assert(g.oak_parcel_flags==YOakParcelAll);
  printf("FRESH_OAK_DELIVERY_PASS steps=%u\n",g.steps);
  exit_lab(&g);to_viridian(&g);
  route(&g,29,20);north(&g,YMapViridianMart);
  assert(g.screen==YScreenWorld && g.oak_parcel_flags==YOakParcelAll);
  route(&g,2,5);yellow_input(&g,YLeft);yellow_input(&g,YOk);
  assert(g.pending==YPendingBuyAntidote && g.antidotes==0);
  yellow_input(&g,YOk);assert(g.antidotes==1);
  dialog(&g);
  assert(g.money_bcd[1]==0x29);
  printf("FRESH_MART_BUY_PASS steps=%u money=%02x%02x%02x\n",
    g.steps,g.money_bcd[0],g.money_bcd[1],g.money_bcd[2]);
  /* Poison is a test fixture, not claimed as a naturally obtained status. */
  g.pika_status=YStatusPoisoned;
  assert(yellow_game_apply_antidote(&g,0,&g.antidotes));
  assert(g.pika_status==YStatusNone && g.antidotes==0);
  YellowAtomicSnapshot snap={0};
  assert(yellow_atomic_make(&g,7,&snap));
  YellowMoneyStockSlot slot={{0}};
  assert(yellow_money_stock_create(g.money_bcd,g.antidotes,
    snap.parent.checksum,snap.generation,&slot));
  YellowGame restored;
  assert(yellow_atomic_unpack(&snap,&restored));
  assert(yellow_money_stock_restore(&slot,snap.parent.checksum,
    snap.generation,restored.money_bcd,&restored.antidotes));
  assert(restored.antidotes==0 && restored.money_bcd[1]==0x29);
  YOakParcelSidecar parcel;
  assert(y_oak_parcel_save(g.oak_parcel_flags,snap.parent.checksum,&parcel));
  assert(y_oak_parcel_restore(&restored.oak_parcel_flags,
    snap.parent.checksum,&parcel));
  assert(restored.oak_parcel_flags==YOakParcelAll);
  assert(!y_oak_parcel_restore(&restored.oak_parcel_flags,
    snap.parent.checksum^1u,&parcel));
  assert(restored.oak_parcel_flags==YOakParcelAll);
  slot.wire[6]^=1;
  assert(!yellow_money_stock_restore(&slot,snap.parent.checksum,
    snap.generation,restored.money_bcd,&restored.antidotes));
  printf("FRESH_PARCEL_BUY_CURE_ATOMIC_RESTART_NEGATIVE_PASS\n");
  return 0;
}
