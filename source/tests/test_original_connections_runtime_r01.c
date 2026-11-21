#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
static void source_edge(const char* source,const char* line) {
 char path[256],buffer[256];int found=0;
 snprintf(path,sizeof(path),"../upstream/pokeyellow/data/maps/headers/%s.asm",source);
 FILE *file=fopen(path,"rb");assert(file);
 while(fgets(buffer,sizeof(buffer),file))
   if(strstr(buffer,line))found++;
 assert(found==1);
 fclose(file);
}
static void edge(uint8_t from,uint8_t key,uint8_t x,uint8_t y,
                 uint8_t to,uint8_t expected_x,uint8_t expected_y) {
 YellowGame game;yellow_init(&game);
 game.map=from;game.x=x;game.y=y;game.facing=key;
 game.story=YStoryPikachuFollows;game.screen=YScreenWorld;
 game.has_pikachu=1;game.follower_valid=0;
 const uint16_t steps=game.steps;
 yellow_input(&game,(YellowKey)key);
 assert(game.map==to && game.x==expected_x && game.y==expected_y);
 assert(game.steps==steps+1);
 assert(game.screen==YScreenWorld);
}
int main(void) {
 source_edge("PalletTown","connection north, Route1, ROUTE_1, 0");
 source_edge("Route1","connection north, ViridianCity, VIRIDIAN_CITY, -5");
 source_edge("ViridianCity","connection north, Route2, ROUTE_2, 5");
 source_edge("Route1","connection south, PalletTown, PALLET_TOWN, 0");
 source_edge("ViridianCity","connection south, Route1, ROUTE_1, 5");
 source_edge("Route2","connection south, ViridianCity, VIRIDIAN_CITY, -5");
 edge(YMapPallet,YUp,10,0,YMapRoute1,10,35);
 edge(YMapPallet,YUp,11,0,YMapRoute1,11,35);
 edge(YMapRoute1,YUp,10,0,YMapViridian,20,35);
 edge(YMapRoute1,YUp,11,0,YMapViridian,21,35);
 edge(YMapViridian,YUp,17,0,YMapRoute2,7,71);
 edge(YMapViridian,YUp,19,0,YMapRoute2,9,71);
 edge(YMapRoute1,YDown,10,35,YMapPallet,10,0);
 edge(YMapRoute1,YDown,11,35,YMapPallet,11,0);
 edge(YMapViridian,YDown,20,35,YMapRoute1,10,0);
 edge(YMapViridian,YDown,21,35,YMapRoute1,11,0);
 edge(YMapRoute2,YDown,7,71,YMapViridian,17,0);
 edge(YMapRoute2,YDown,9,71,YMapViridian,19,0);
 uint8_t map=255,x=255,y=255;
 assert(!yellow_source_connection(YMapRoute1,0,9,-1,&map,&x,&y));
 assert(!yellow_source_connection(YMapRoute1,1,10,35,&map,&x,&y));
 assert(!yellow_source_connection(YMapRoute1,0,10,0,&map,&x,&y));
 assert(!yellow_source_connection(YMapRoute1,1,10,35,NULL,&x,&y));
 assert(!yellow_source_connection(YMapRedsHouse,0,2,-1,&map,&x,&y));
 assert(yellow_source_connection(YMapRoute1,0,10,-1,&map,&x,&y));
 assert(map==YMapViridian && x==20 && y==35);
 /* No north boundary crossing without obtained Pikachu. */
 YellowGame gated;yellow_init(&gated);
 gated.map=YMapRoute1;gated.x=10;gated.y=0;
 gated.story=YStoryBeforeOak;gated.screen=YScreenWorld;
 yellow_input(&gated,YUp);
 assert(gated.map==YMapRoute1 && gated.y==0);
 /* V13 save from one of the generated connections still loads identically. */
 YellowGame g;yellow_init(&g);g.map=YMapRoute1;g.x=10;g.y=35;
 g.story=YStoryPikachuFollows;g.has_pikachu=1;g.screen=YScreenWorld;
 yellow_input(&g,YDown);
 YellowSave saved;yellow_save_create(&g,&saved);
 YellowGame restored;assert(yellow_save_load(&restored,&saved));
 assert(saved.version==13 && restored.map==YMapPallet &&
        restored.x==10 && restored.y==0);
 puts("PINNED_SIX_WORLD_CONNECTIONS_GAMEPLAY_BOUNDS_SAVE_V13_PASS");
 return 0;
}
