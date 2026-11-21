#include "yellow_core.h"
#include "yellow_oak_parcel_sidecar.h"
#include "yellow_original_collision.c"
#include "yellow_source_connections.c"
#include "yellow_source_warp_graph.c" /* upstream-backed native door/forest connection tables */
#include "yellow_source_event_ir.c"
#include "yellow_source_forest_parties.c" /* original 1-based forest trainer numbering */ /* pinned object/script event IR */
#include "yellow_assets.h"
#include "yellow_pokedex_ledger.c" /* bounded source-indexed ledger */
#include "yellow_dex_names.c" /* pinned 151 full source names */
#include "yellow_pewter_progress.h"
#include "yellow_pewter_gym_scene.c" /* original imported GYM tileset + collision */
#include "yellow_pewter_city_scene.c" /* original sourced overlay, legacy asset table immutable */
#include "yellow_route3_scene.c" /* original Route 3 geometry and collisions */
#include "yellow_route4_scene.c" /* original Route4 geography and collisions */
#include "yellow_cerulean_city_scene.c" /* source exact native Cerulean geography */
#include "yellow_route9_scene.c" /* source exact Route 9 overworld */
#include "yellow_route10_scene.c" /* original Route10 Overworld 10x36 + source collision */
#include "yellow_route22_scene.c" /* source exact Route22 */
#include "yellow_viridian_mart_scene.c" /* source exact original mart */
#include "yellow_mtmoon1f_scene.c" /* original CAVERN layout, tiles and collision */
#include "yellow_mtmoonb1f_scene.c" /* original B1F Cavern grid + 784 collision steps */
#include "yellow_mtmoonb2f_scene.c" /* original B2F Cavern grid + 1440 collision steps */
#include "yellow_mtmoon1f_runtime.c" /* isolated original cave entrance event */
#include "yellow_mtmoon_wild.c" /* pinned original 10 slots, fail-closed unsupported */
#include "yellow_route3_trainers.c" /* sourced 3/8 supported native trainer parties */
#include "yellow_original_game_audit.c" /* evidence-limited coverage introspection */
#include "yellow_flipper_smoke_contract.c" /* read-only host/hardware preflight contract */
#include <string.h>

/* R0.15 offline-only deterministic frame. No radio, USB, addresses, pairing,
 * device reads, remote inputs, saves, or network operations are present. */
static uint16_t yellow_link_crc16(const uint8_t* data,size_t length) {
    uint16_t crc=0xffffu;
    for(size_t i=0;i<length;i++) {
        crc^=(uint16_t)data[i]<<8;
        for(unsigned bit=0;bit<8;bit++)
            crc=(uint16_t)(((uint32_t)crc<<1)^((crc&0x8000u)?0x1021u:0u));
    }
    return crc;
}
static bool yellow_link_valid(const YellowLinkSnapshot* item) {
    if(!item)return false;
    if(item->kind==0)
        return item->species==0 && item->level==0 &&
               item->hp==0 && item->status==0;
    if(item->kind!=1 || item->level<1 || item->level>100 ||
       !y_species_info(item->species) || !y_status_valid(item->status))
        return false;
    return item->hp<=y_calc_stats(item->species,item->level).hp;
}
bool yellow_link_encode(const YellowLinkSnapshot* input,
                        uint8_t output[YELLOW_LINK_PACKET_SIZE]) {
    if(!output || !yellow_link_valid(input))return false;
    uint8_t temp[YELLOW_LINK_PACKET_SIZE]={
        'Y','L',1,input->kind,
        (uint8_t)input->sequence,(uint8_t)(input->sequence>>8),
        input->species,input->level,input->hp,input->status,0,0
    };
    uint16_t crc=yellow_link_crc16(temp,10);
    temp[10]=(uint8_t)crc;
    temp[11]=(uint8_t)(crc>>8);
    memcpy(output,temp,sizeof(temp));
    return true;
}
bool yellow_link_decode(const uint8_t* packet,size_t length,
                        YellowLinkSnapshot* output) {
    if(!packet || !output || length!=YELLOW_LINK_PACKET_SIZE ||
       packet[0]!='Y' || packet[1]!='L' ||
       packet[2]!=1 || packet[3]>1)return false;
    uint16_t crc=yellow_link_crc16(packet,10);
    if(packet[10]!=(uint8_t)crc || packet[11]!=(uint8_t)(crc>>8))
        return false;
    YellowLinkSnapshot parsed={
        .sequence=(uint16_t)(packet[4]|((uint16_t)packet[5]<<8)),
        .kind=packet[3],.species=packet[6],.level=packet[7],
        .hp=packet[8],.status=packet[9]
    };
    if(!yellow_link_valid(&parsed))return false;
    *output=parsed;
    return true;
}

/*
 * Native game logic; original Gen 1 maps/tiles/overworld actors originate
 * from pret/pokeyellow. This is a limited adaptation of the opening sequence,
 * NOT a byte-identical implementation of Pokemon Yellow's battle formula,
 * movement scripts, or complete event engine.
 */

enum {
    TalkOak,TalkGirl,TalkFisher,TalkMom,TalkDaisy,TalkRival,
    TalkLabOak,TalkLabGirl,TalkScientist,TalkTownSign,TalkLabSign,
    TalkHomeSign,TalkRivalSign,TalkMap,TalkTV,TalkExit,TalkRouteYoungster,TalkRouteSign,TalkViridianResident,TalkViridianSign,TalkViridianCenter,TalkReception,TalkRoute2Item,TalkForestTrainer,TalkForestItem,TalkForestGate
};
const YellowNpc yellow_npcs[]={
    {YMapPallet,10,4,YActorOak,TalkOak},
    {YMapPallet,3,8,YActorGirl,TalkGirl},
    {YMapPallet,11,14,YActorFisher,TalkFisher},
    {YMapRedsHouse,5,4,YActorMom,TalkMom},
    {YMapBluesHouse,2,3,YActorDaisy,TalkDaisy},
    {YMapBluesHouse,6,4,YActorDaisy,TalkDaisy},
    {YMapBluesHouse,3,3,YActorPokedex,TalkMap},
    {YMapOaksLab,4,3,YActorBlue,TalkRival},
    {YMapOaksLab,7,3,YActorPokeBall,TalkLabOak},
    {YMapOaksLab,5,2,YActorOak,TalkLabOak},
    {YMapOaksLab,5,10,YActorOak,TalkLabOak},
    {YMapOaksLab,1,9,YActorGirl,TalkLabGirl},
    {YMapOaksLab,2,10,YActorScientist,TalkScientist},
    {YMapOaksLab,8,10,YActorScientist,TalkScientist},
    /* route/city object event coordinates copied from original object files */
    {YMapRoute1,5,24,YActorYoungster,TalkRouteYoungster},
    {YMapRoute1,15,13,YActorYoungster,TalkRouteYoungster},
    {YMapViridian,13,20,YActorYoungster,TalkViridianResident},
    {YMapViridian,30,8,YActorGambler,TalkViridianResident},
    {YMapViridian,30,25,YActorYoungster,TalkViridianResident},
    {YMapViridian,17,9,YActorGirl,TalkViridianResident},
    {YMapViridian,18,9,YActorGambler,TalkViridianResident},
    {YMapViridian,6,23,YActorFisher,TalkViridianResident},
    {YMapViridian,17,5,YActorGambler,TalkViridianResident},
    {YMapViridianCenter,3,1,YActorNurse,TalkViridianCenter},
    {YMapViridianCenter,10,5,YActorGentleman,TalkViridianResident},
    {YMapViridianCenter,4,3,YActorCooltrainer,TalkViridianResident},
    {YMapViridianCenter,11,2,YActorReceptionist,TalkReception},
    {YMapViridianCenter,4,1,YActorChansey,TalkViridianResident},
    /* R0.13 map objects from pinned data/maps/objects/{Route2,ViridianForest}.asm.
       Trainer battles and item pickup effects are deliberately not implemented. */
    {YMapRoute2,13,54,YActorPokeBall,TalkRoute2Item},
    {YMapRoute2,13,45,YActorPokeBall,TalkRoute2Item},
    {YMapViridianForest,16,43,YActorYoungster,TalkForestTrainer},
    {YMapViridianForest,30,33,YActorYoungster,TalkForestTrainer},
    {YMapViridianForest,30,19,YActorYoungster,TalkForestTrainer},
    {YMapViridianForest,2,18,YActorYoungster,TalkForestTrainer},
    {YMapViridianForest,2,41,YActorCooltrainer,TalkForestTrainer},
    {YMapViridianForest,13,17,YActorYoungster,TalkForestTrainer},
    {YMapViridianForest,25,11,YActorPokeBall,TalkForestItem},
    {YMapViridianForest,12,29,YActorPokeBall,TalkForestItem},
    {YMapViridianForest,1,31,YActorPokeBall,TalkForestItem},
    {YMapViridianForest,27,40,YActorYoungster,TalkForestTrainer},
    {YMapForestSouthGate,8,4,YActorGirl,TalkForestGate},
    {YMapForestSouthGate,2,4,YActorGirl,TalkForestGate},
    {YMapForestNorthGate,3,2,YActorGambler,TalkForestGate},
    {YMapForestNorthGate,2,5,YActorGambler,TalkForestGate},
    /* Original PewterGym Object.asm IDs, not invented trainer positions. */
    {YMapPewterGym,4,1,YActorCooltrainer,TalkForestTrainer},
    {YMapPewterGym,3,6,YActorCooltrainer,TalkForestTrainer},
    {YMapPewterGym,7,10,YActorGambler,TalkForestGate},
    /* Real original object-event coordinates for the three parties fully
       representable by current 13 native species. Never fake the other 5. */
    {YMapRoute3,16,9,YActorCooltrainer,TalkForestTrainer}, /* LASS 1 */
    {YMapRoute3,23,4,YActorCooltrainer,TalkForestTrainer}, /* LASS 2 */
    {YMapRoute3,24,6,YActorYoungster,TalkForestTrainer}, /* BUG_CATCHER 6 */
};
const uint8_t yellow_npc_count=(uint8_t)(sizeof(yellow_npcs)/sizeof(yellow_npcs[0]));

static const char* const intro[]={
    "PALLET TOWN","YOUR JOURNEY BEGINS",
    "ORIGINAL MAP ART","NATIVE 128X64 VIEW",
    "D-PAD: MOVE","OK: INTERACT"
};
static const char* const oak_regular[]={"OAK: Be careful!","Tall grass is risky."};
static const char* const girl[]={
    "I'm raising","POKEMON too!",
    "When they get","strong, they can",
    "protect me!",""
};
static const char* const fisher[]={
    "Technology is","incredible!",
    "Items and POKEMON","can be stored",
    "as data in a PC.",""
};
static const char* const mom[]={"MOM: Travel safe!","Come back sometime."};
static const char* const daisy[]={"DAISY: Hi!","Nice to see you."};
static const char* const rival[]={"RIVAL: I'll show","you how it's done."};
static const char* const lab_oak[]={"OAK: Choose wisely.","(more events TODO)"};
static const char* const lab_girl[]={"OAK'S LAB ASSISTANT","HELLO!"};
static const char* const scientist[]={"RESEARCHER:","POKEMON ARE AMAZING!"};
static const char* const town_sign[]={"PALLET TOWN","Shades of your","journey await!",""};
static const char* const lab_sign[]={"OAK POKEMON","RESEARCH LAB"};
static const char* const home_sign[]={"PLAYER'S HOUSE",""};
static const char* const rival_sign[]={"RIVAL'S HOUSE",""};
static const char* const townmap[]={"KANTO TOWN MAP","(not ported yet)"};
static const char* const tv[]={"A TV SET.",""};
static const char* const exit_text[]={"ROUTE CONNECTION","(not ported yet)"};
static const char* const oak_warning[]={
    "OAK: HEY! WAIT!","DON'T GO OUT!",
    "WILD POKEMON LIVE","IN TALL GRASS!",
    "LET ME HELP YOU.",""
};
/* Pinned OaksLabOakDontGoAwayYetText; fits the 128x64 UI. */
static const char* const oak_dont_leave[]={ "OAK: HEY! DON'T", "GO AWAY YET!" };
static const char* const capture_text[]={
    "OAK: PHEW!","THAT WAS CLOSE!",
    "OAK CAUGHT A","WILD PIKACHU.",
    "COME WITH ME TO","MY LAB!"
};
static const char* const lab_intro[]={
    "IN OAK'S LAB...","",
    "OAK: CHOOSE A","POKEMON.",
    "RIVAL: HMPH!","I WANT THAT BALL!"
};
static const char* const starter_text[]={
    "RIVAL GRABS","THE EEVEE BALL!",
    "OAK: TAKE THIS","PIKACHU INSTEAD.",
    "PIKACHU JOINED","YOUR PARTY!"
};
static const char* const challenge_text[]={
    "RIVAL: LET'S SEE","WHO IS STRONGER!",
    "PIKACHU VS EEVEE","LEVEL 5"
};
static const char* const victory_text[]={"RIVAL: HMPH!","I'LL WIN NEXT TIME!"};
static const char* const defeat_text[]={"PIKACHU FAINTED!","OAK HEALED IT."};
/* pret/pokeyellow engine/events/poison.asm TEXT_BLACKED_OUT. Recovery map
 * routing is not ported; this dialogue is only the source-backed event prefix. */
static const char* const poison_blackout_text[]={"BLACKED OUT!"};
/* pret/pokeyellow text/ViridianMart.asm, original clerk auto-greeting.
 * The subsequent scripted walk and parcel grant are NOT ported here.
 */
static const char* const mart_clerk_auto_hello[]={"HEY! YOU CAME","FROM PALLET TOWN?"};
static const char* const mart_clerk_parcel_text[]={"OAK HAS AN ORDER","TAKE IT TO HIM!","GOT OAKS PARCEL!"};
/* Source: ViridianMart clerk (0,5), Antidote 100, post Oak delivery.
 * This bounded menu buys exactly ONE item. No injected stock. */
static const char* const mart_antidote_offer[]={"ANTIDOTE: 100","OK BUY / BACK NO"};
static const char* const mart_antidote_bought[]={"BOUGHT ANTIDOTE!","CHECK YOUR BAG."};
static const char* const mart_antidote_denied[]={"CANNOT BUY NOW.","FUNDS/BAG/LOCK."};
static const char* const mart_antidote_quest[]={"DELIVER OAKS","PARCEL FIRST!"};
static const char* const oak_parcel_delivery_text[]={"OAK: MY PARCEL!","THANK YOU VERY MUCH!"};
static const char* const pika_happy[]={"PIKA PIKA!","IT LOOKS HAPPY."};
static const char* const pika_normal[]={"PIKA?","IT LOOKS AT YOU."};
static const char* const early_lab[]={"OAK ISN'T HERE.","COME BACK LATER!"};
static const char* const route_youngster[]={"ROUTE 1","TALL GRASS AHEAD!"};
static const char* const route_sign[]={"ROUTE 1","PALLET - VIRIDIAN"};
static const char* const viridian_resident[]={"VIRIDIAN CITY","HELLO TRAINER!"};
static const char* const viridian_sign[]={"VIRIDIAN CITY","THE ETERNALLY GREEN"};
static const char* const viridian_center[]={"WELCOME TO OUR","POKEMON CENTER!",
    "YOUR POKEMON","IS FULLY HEALED!"};
static const char* const reception[]={"LINK CABLE ROOM","NOT PORTED YET"};
/* Byte-for-byte text from pinned text/Route2.asm, displayed across
   128x64 screens without changing battle, inventory or save semantics. */
static const char* const source_route2_sign[]={"ROUTE 2","VIRIDIAN CITY -",
                                                 "PEWTER CITY"};
static const char* const source_route2_cave_sign[]={"DIGLETT'S CAVE"};
static const char* const route2_item[]={"ROUTE 2 ITEM","PICKUP NOT PORTED"};
static const char* const forest_trainer[]={"VIRIDIAN FOREST","TRAINER BATTLE TODO"};
/* Original pret/pokeyellow/text/ViridianForest.asm, Youngster2BattleText.
 * Displayed on two 128x64-compatible pages; no trainer combat or flags. */
static const char* const forest_youngster2_challenge[]={
    "HEY! YOU HAVE","POKEMON! COME ON!",
    "LET'S BATTLE 'EM!",""
};
static const char* const forest_item[]={"FOREST ITEM","PICKUP NOT PORTED"};
static const char* const forest_potion_found[]={"FOUND POTION!","PUT IN BAG."};
static const char* const forest_ball_found[]={"FOUND POKE BALL!","PUT IN BAG."};
static const char* const forest_bag_full[]={"BAG IS FULL!","ITEM LEFT HERE."};
static const char* const forest_gate[]={"FOREST GATE","WATCH YOUR STEP"};
/* BG events from pinned pret/pokeyellow ViridianForest objects and text.
   These are signs only. Trainer AI, items, and wild forest species are NOT ported. */
static const char* const forest_tips1[]={"TRAINER TIPS", "AVOID BATTLES:",
    "STAY AWAY FROM", "GRASSY AREAS!"};
static const char* const forest_antidote_sign[]={"FOR POISON, USE", "ANTIDOTE!",
    "GET IT AT", "POKEMON MARTS!"};
static const char* const forest_tips2[]={"TRAINER TIPS", "CONTACT PROF. OAK",
    "VIA PC TO HAVE", "POKEDEX EVALUATED"};
static const char* const forest_tips3[]={"TRAINER TIPS", "NO STEALING FROM",
    "OTHER TRAINERS!", "CATCH WILD MONS!"};
static const char* const forest_tips4[]={"TRAINER TIPS", "WEAKEN POKEMON",
    "BEFORE CAPTURE!", "HEALTHY MAY ESCAPE"};
static const char* const forest_leave_sign[]={"LEAVING FOREST", "PEWTER CITY AHEAD"};
static const char* const caught_text[]={"GOTCHA! WILD","POKEMON CAUGHT!"};
static const char* const proto_kit[]={"PIKACHU WILL","FOLLOW YOU!",
    "DEV KIT: YOU GET","5 POKE BALLS.",
    "ORIGINAL PARCEL","QUEST NOT YET PORTED."};
static const char* const forest_trainer_victory[]={"BUG CATCHER","DEFEATED!"};
static const char* const forest_lass_challenge[]={"LET'S BATTLE!","NIDORAN TEAM!"};
static const char* const forest_lass_victory[]={"LASS DEFEATED!","GOOD BATTLE!"};
static const char* const forest_lass_defeat[]={"LASS WON!","BACK TO MOM!"};
/* Pinned pret/pokeyellow/text/ViridianForest.asm:
 * ViridianForestYoungster3AfterBattleText, wrapped for 128x64. */
static const char* const forest_youngster3_after[]={
    "DARN! I'M GOING","TO CATCH SOME","STRONGER ONES!",""
};
static const char* const forest_lass_after[]={"WE ALREADY","BATTLED!"};
static const char* const forest_trainer_defeat[]={"BUG CATCHER WON!","BACK TO MOM!"};
static const char* const wild_victory[]={"WILD POKEMON","FAINTED!"};
static const char* const wild_flee[]={"GOT AWAY","SAFELY!"};
static const char* const wild_defeat[]={"PIKACHU FAINTED!","BACK TO MOM!"};
static const uint8_t encounter_weights[10]={51,51,39,25,25,25,13,13,11,3};
static const uint8_t route1_levels[10]={3,4,2,3,2,3,5,4,6,7};
static const uint8_t route1_kinds[10]={1,1,2,2,1,1,1,2,1,1};
/* Pinned pret/pokeyellow/data/wild/maps/ViridianForest.asm; Gen1 Yellow. */
static const uint8_t forest_levels[10]={3,4,4,5,4,6,6,6,8,9};
static const uint8_t forest_kinds[10]={YK_CATERPIE,YK_METAPOD,YK_CATERPIE,
    YK_CATERPIE,YK_PIDGEY,YK_PIDGEY,YK_CATERPIE,YK_METAPOD,
    YK_PIDGEY,YK_PIDGEOTTO};
/* Imported catalog is attached only after verification. Existing original
   hard-coded opening encounter behavior is retained as a fail-safe fallback.
   Never use unsupported species in the existing small 7-species runtime. */
bool yellow_attach_bulk_wild(YellowGame* g,const YellowBulkWild* imported) {
    if(!g)return false;
    g->imported_wild=NULL;
    if(!imported)return true;
    const YellowWildSlots* sets[2]={&imported->route1,&imported->forest};
    for(uint8_t j=0;j<2;j++) {
        if(sets[j]->rate!=25)return false;
        for(uint8_t i=0;i<10;i++) {
            if(!y_species_info(sets[j]->kind[i]) ||
               sets[j]->level[i]<1 || sets[j]->level[i]>100)
                return false;
        }
    }
    g->imported_wild=imported;
    return true;
}
bool yellow_wild_slot(const YellowGame* g,uint8_t map,uint8_t slot,
                      uint8_t* kind,uint8_t* level) {
    if(!g || !kind || !level || slot>=10)return false;
    const YellowWildSlots* chosen=NULL;
    const uint8_t* fallback_kind=NULL;
    const uint8_t* fallback_level=NULL;
    if(map==YMapRoute1) {
        if(g->imported_wild)chosen=&g->imported_wild->route1;
        fallback_kind=route1_kinds;fallback_level=route1_levels;
    } else if(map==YMapViridianForest) {
        if(g->imported_wild)chosen=&g->imported_wild->forest;
        fallback_kind=forest_kinds;fallback_level=forest_levels;
    } else if(yellow_mtmoon1f_is_active(map)) {
        if(g->native_clefairy_extension &&
           yellow_mtmoon_clefairy_wild_slot(slot,kind,level))return true;
        if(g->native_zubat_extension &&
           yellow_mtmoon_zubat_wild_slot(slot,kind,level))return true;
        return yellow_mtmoon_native_wild_slot(slot,kind,level);
    }
    else return false;
    *kind=chosen?chosen->kind[slot]:fallback_kind[slot];
    *level=chosen?chosen->level[slot]:fallback_level[slot];
    return true;
}


static const char* const empty_ball[]={"EMPTY POKE BALL","RIVAL TOOK EEVEE"};

typedef struct {const char* const* lines;uint8_t pages;} DialogDef;
static const DialogDef dialogs[]={
    {oak_regular,1},{girl,3},{fisher,3},{mom,1},{daisy,1},
    {rival,1},{lab_oak,1},{lab_girl,1},{scientist,1},
    {town_sign,2},{lab_sign,1},{home_sign,1},{rival_sign,1},
    {townmap,1},{tv,1},{exit_text,1},
    {route_youngster,1},{route_sign,1},{viridian_resident,1},
    {viridian_sign,1},{viridian_center,2},{reception,1},
    {route2_item,1},{forest_trainer,1},{forest_item,1},{forest_gate,1}
};

static void open_dialog(YellowGame* g,const char* const* lines,uint8_t pages,YellowPending pending) {
    g->dialog_lines=lines;g->dialog_count=pages;
    g->dialog_page=0;g->pending=pending;g->screen=YScreenDialog;
}
static void dialog_id(YellowGame* g,uint8_t id) {
    if(id>=sizeof(dialogs)/sizeof(dialogs[0]))return;
    open_dialog(g,dialogs[id].lines,dialogs[id].pages,YPendingNone);
}

static uint32_t next_rng(YellowGame* g) {
    uint32_t x=g->rng?g->rng:0x96C08u;
    x^=x<<13;x^=x>>17;x^=x<<5;
    return g->rng=x?x:0xA53DCu;
}
/* Default OFF preserves accepted B1F runtime and immutable R12 tests.
 * The actual Flipper game activates B2F once at startup. The standalone
 * B2F acceptance oracles opt in explicitly, so they test the live path.
 * Must never change the v056 source_warps[] table or legacy public ABI. */
static bool yellow_b2f_feature_enabled=false;
void yellow_b2f_set_enabled(bool enabled){yellow_b2f_feature_enabled=enabled;}

/* New gameplay/renderer dispatch, preserving the accepted legacy API:
   yellow_full_scene_asset(16) must still return NULL for its pinned test. */
const YellowLevelAsset* yellow_gameplay_scene_asset(uint8_t map){
    if(map==YMapCeruleanCity)return yellow_cerulean_city_scene_asset();
    if(map==YMapRoute9)return yellow_route9_scene_asset();
    if(map==YMapRoute10)return yellow_route10_scene_asset();
    if(map==YMapRoute22)return yellow_route22_scene_asset();
    if(map==YMapViridianMart||map==YMapCeruleanMart)return yellow_viridian_mart_scene_asset();
    if(map==YELLOW_B1F_MAP_ID)return yellow_mtmoonb1f_scene_asset();
    if(map==YELLOW_B2F_MAP_ID && yellow_b2f_feature_enabled)return yellow_mtmoonb2f_scene_asset();
    return yellow_full_scene_asset(map);
}
int yellow_map_w(uint8_t map) {
    const YellowLevelAsset* a=yellow_gameplay_scene_asset(map);
    return a?a->w*2:0;
}
int yellow_map_h(uint8_t map) {
    const YellowLevelAsset* a=yellow_gameplay_scene_asset(map);
    return a?a->h*2:0;
}
uint8_t yellow_tile_id(uint8_t map,int x,int y) {
    if(map>=YELLOW_MAP_COUNT)return 0xff;
    const YellowLevelAsset* a=&yellow_levels[map];
    if(x<0||y<0||x>=a->w*4||y>=a->h*4)return 0xff;
    uint8_t block=a->map[(y/4)*a->w+x/4];
    if(block>=a->blocks_count)return 0xff;
    uint8_t id=a->blocks[block*16+(y%4)*4+(x%4)];
    return id<a->tiles_count?id:0xff;
}
/* Extended world tile API: legacy yellow_tile_id invalid-map sentinel stays intact. */
uint8_t yellow_world_tile_id(uint8_t map,int x,int y) {
    if(map==YMapPewterCity)return yellow_pewter_city_tile_id(x,y);
    if(map==YMapPewterGym)return yellow_pewter_gym_tile_id(x,y);
    if(map==YMapRoute3)return yellow_route3_tile_id(x,y);
    if(map==YMapRoute4)return yellow_route4_tile_id(x,y);
    if(map==YMapCeruleanCity)return yellow_cerulean_city_tile_id(x,y);
    if(map==YMapRoute9)return yellow_route9_tile_id(x,y);
    if(map==YMapRoute10)return yellow_route10_tile_id(x,y);
    if(map==YMapRoute22)return yellow_route22_tile_id(x,y);
    if(map==YMapViridianMart||map==YMapCeruleanMart)return yellow_viridian_mart_tile_id(x,y);
    if(yellow_mtmoon1f_is_active(map))return yellow_mtmoon1f_tile_id(x,y);
    if(map==YELLOW_B1F_MAP_ID)return yellow_mtmoonb1f_tile_id(x,y);
    if(map==YELLOW_B2F_MAP_ID && yellow_b2f_feature_enabled)return yellow_mtmoonb2f_tile_id(x,y);
    return yellow_tile_id(map,x,y);
}
/* Pinned Viridian Forest object data: Potions at (25,11),(12,29),
   Poke Ball at (1,31). Preserve real NPC interactions elsewhere. */
static uint8_t forest_item_bit(const YellowNpc* n) {
    if(n->map!=YMapViridianForest || n->type!=YActorPokeBall ||
       n->dialog_id!=TalkForestItem)return 0;
    /* Original forest object records are compiled into typed IR. Preserve
       existing per-item sidecar bits and NPC visibility semantics. */
    return yellow_ir_forest_item_mask(n->x,n->y);
}
bool yellow_npc_visible(const YellowGame* g,const YellowNpc* n) {
    if(g->map!=n->map)return false;
    uint8_t item=forest_item_bit(n);
    if(item&&(g->forest_pickups&item))return false;
    if(n->map==YMapPallet&&n->type==YActorOak)
        return g->story==YStoryBeforeOak; /* Oak leaves Pallet in intro */
    if(n->map==YMapOaksLab) {
        if(n->type==YActorOak&&n->y==10)
            return g->story==YStoryOakEncounter;
        if(n->type==YActorOak&&n->y==2)
            return g->story>=YStoryChooseStarter;
        if(n->type==YActorPokeBall)
            return g->story<YStoryRivalBattle;
    }
    return true;
}
static bool npc_at(const YellowGame* g,int x,int y) {
    for(uint8_t i=0;i<yellow_npc_count;i++)
        if(yellow_npcs[i].x==x&&yellow_npcs[i].y==y&&
           yellow_npc_visible(g,&yellow_npcs[i]))return true;
    return false;
}
static bool base_passable(uint8_t map,int x,int y) {
    if(map==YELLOW_B2F_MAP_ID && yellow_b2f_feature_enabled)
        return yellow_mtmoonb2f_passable(x,y)!=0;
    if(map==YMapCeruleanCity)return yellow_cerulean_city_passable(x,y)!=0;
    if(map==YMapRoute9)return yellow_route9_passable(x,y)!=0;
    if(map==YMapRoute10)return yellow_route10_passable(x,y)!=0;
    if(map==YMapRoute22)return yellow_route22_passable(x,y)!=0;
    if(map==YMapViridianMart||map==YMapCeruleanMart)return yellow_viridian_mart_passable(x,y)!=0;
    if(map>YELLOW_B1F_MAP_ID||x<0||y<0||x>=yellow_map_w(map)||y>=yellow_map_h(map))
        return false;
    if(map==YMapPewterCity)return yellow_pewter_city_passable(x,y)!=0;
    if(map==YMapPewterGym)return yellow_pewter_gym_passable(x,y)!=0;
    if(map==YMapRoute3)return yellow_route3_passable(x,y)!=0;
    if(map==YMapRoute4)return yellow_route4_passable(x,y)!=0;
    if(yellow_mtmoon1f_is_active(map))return yellow_mtmoon1f_passable(x,y)!=0;
    if(map==YELLOW_B1F_MAP_ID)return yellow_mtmoonb1f_passable(x,y)!=0;
    /* Gen1 collision checks the bottom-left 8x8 tile of each 16x16 step. */
    bool imported=false;
    if(yellow_original_collision(map,x,y,&imported))return imported;
    uint8_t tile=yellow_tile_id(map,x*2,y*2+1);
    return tile<96&&yellow_levels[map].collision[tile]!=0;
}
static bool destination(uint8_t map,int x,int y,uint8_t* target,uint8_t* dx,uint8_t* dy) {
    /* 28 previously available native door/forest warps verified against the
       pinned Gen1 warp-event coordinates. LAST_MAP is resolved via a strict
       source-verified whitelist; unsupported warps never become phantom maps. */
    if(target && dx && dy){
        if(yellow_b2f_feature_enabled) {
            static const struct {
                uint8_t source,sx,sy,target,tx,ty;
            } b2f_warps[]={
                {YELLOW_B1F_MAP_ID,17,11,YELLOW_B2F_MAP_ID,25,9},
                {YELLOW_B1F_MAP_ID,21,17,YELLOW_B2F_MAP_ID,21,17},
                {YELLOW_B1F_MAP_ID,13,27,YELLOW_B2F_MAP_ID,15,27},
                {YELLOW_B1F_MAP_ID,23,3,YELLOW_B2F_MAP_ID,5,7},
                {YELLOW_B2F_MAP_ID,25,9,YELLOW_B1F_MAP_ID,17,11},
                {YELLOW_B2F_MAP_ID,21,17,YELLOW_B1F_MAP_ID,21,17},
                {YELLOW_B2F_MAP_ID,15,27,YELLOW_B1F_MAP_ID,13,27},
                {YELLOW_B2F_MAP_ID,5,7,YELLOW_B1F_MAP_ID,23,3},
            };
            for(unsigned i=0;i<sizeof(b2f_warps)/sizeof(b2f_warps[0]);i++){
                const uint8_t a=b2f_warps[i].source;
                if(map==a && x==b2f_warps[i].sx && y==b2f_warps[i].sy){
                    *target=b2f_warps[i].target;
                    *dx=b2f_warps[i].tx;*dy=b2f_warps[i].ty;
                    return true;
                }
            }
        }
        /* Original pret/pokeyellow Object.asm: City (16,17) warp 1 to
           PewterGym; Gym (4,13),(5,13) LAST_MAP warp 3 to City. */
        if(map==YMapPewterCity && x==16 && y==17){
            *target=YMapPewterGym;*dx=4;*dy=12;return true;
        }
        if(map==YMapPewterGym && (x==4||x==5) && y==13){
            *target=YMapPewterCity;*dx=16;*dy=18;return true;
        }
        if(yellow_mtmoon1f_source_warp(map,x,y,target,dx,dy))return true;
    }
    return yellow_original_warp_at(map,x,y,target,dx,dy);
}

bool yellow_walkable(const YellowGame* g,int x,int y) {
    uint8_t target,dx,dy;
    if(destination(g->map,x,y,&target,&dx,&dy))return true;
    return base_passable(g->map,x,y)&&!npc_at(g,x,y);
}
static void set_follower_near(YellowGame* g) {
    g->follower_valid=0;
    if(g->story!=YStoryPikachuFollows||!g->has_pikachu)return;
    static const int8_t delta[][2]={{0,1},{0,-1},{-1,0},{1,0}};
    for(int i=0;i<4;i++) {
        int x=g->x+delta[i][0],y=g->y+delta[i][1];
        if(yellow_walkable(g,x,y)) {
            g->follower_map=g->map;g->follower_x=(uint8_t)x;
            g->follower_y=(uint8_t)y;g->follower_valid=1;return;
        }
    }
}
/* The complete upstream Route1_Script is exactly:
     call EnableAutoTextBoxDrawing
     ret
   Execute only this independently validated source prefix. No ASM guessed
   for other maps, and unsupported opcodes cannot mutate game state. */
static bool source_event_safe_opcode(YellowGame* g,uint8_t op) {
    if(!g || !yellow_ir_opcode_supported(op))return false;
    switch(op) {
    case YEIR_OP_TEXTBOX: g->source_script_state|=1u;return true;
    case YEIR_OP_RETURN: g->source_script_state|=2u;return true;
    default:return false;
    }
}
static void run_source_map_script(YellowGame* g) {
    g->source_script_state=0;
    if(g->map!=YMapRoute1)return;
    uint8_t expected=0;
    for(uint16_t i=0;i<yellow_ir_count();i++) {
        YellowEventIR record;
        if(!yellow_ir_at(i,&record))return;
        if(record.map!=YMapRoute1 || record.kind!=YEIR_SAFE_OPCODE)continue;
        /* Refuse to skip gaps or run a partial/unreviewed script. */
        if(record.index!=expected ||
           (expected==0 && record.value!=YEIR_OP_TEXTBOX) ||
           (expected==1 && record.value!=YEIR_OP_RETURN)) {
            g->source_script_state=0;return;
        }
        if(!source_event_safe_opcode(g,record.value)) {
            g->source_script_state=0;return;
        }
        expected++;
        if(expected==2)break;
    }
    if(expected!=2)g->source_script_state=0;
}
static void set_map(YellowGame* g,uint8_t map,uint8_t x,uint8_t y) {
    uint8_t previous_map=g->map;
    g->map=map;g->x=(int8_t)x;g->y=(int8_t)y;
    run_source_map_script(g);
    set_follower_near(g);
    /* Original ViridianMartDefaultScript runs automatically ONLY before
     * EVENT_GOT_OAKS_PARCEL, never after acquisition or delivery.
     * Corrupt sidecar must fail closed, not replay the parcel script.
     * Original ViridianMartDefaultScript runs automatically when entering.
     * Opt-in avoids changing the frozen legacy tests, while the actual
     * Flipper app enables this behavior after save loading.
     * Do NOT grant OAKS_PARCEL until its original persistent inventory,
     * separate EVENT_GOT_OAKS_PARCEL and scripted movement are implemented.
     */
    if(g->native_mart_intro_extension && g->story==YStoryPikachuFollows &&
       map==YMapViridianMart && previous_map!=YMapViridianMart &&
       !g->oak_parcel_integrity_error && g->oak_parcel_flags==0u) {
        g->native_mart_intro_pending=1u;
        open_dialog(g,mart_clerk_auto_hello,2,YPendingNone);
    } else g->native_mart_intro_pending=0u;
}
void yellow_init(YellowGame* g) {
    memset(g,0,sizeof(*g));g->map=YMapRedsHouse;
    g->x=3;g->y=6;g->facing=YUp;g->hp=YELLOW_MAX_HP;
    g->rng=0x5733CC11u;g->pika_level=5;
    /* Original init_player_data.asm START_MONEY=$3000, packed BCD. */
    g->money_bcd[0]=0x00u;g->money_bcd[1]=0x30u;g->money_bcd[2]=0x00u;
    g->forest_active_trainer=255;
    g->experience=y_exp_threshold(YK_PIKACHU,5);
    y_build_moveset(YK_PIKACHU,5,g->pika_moves);
    for(uint8_t i=0;i<YELLOW_MOVE_SLOTS;i++)
        g->pika_pp[i]=y_max_pp(g->pika_moves[i]);
    g->running=true;g->story=YStoryBeforeOak;
    open_dialog(g,intro,3,YPendingNone);
}
uint8_t yellow_player_max_hp(const YellowGame* g) {
    return y_calc_stats(YK_PIKACHU,g->pika_level).hp;
}
void yellow_heal_pikachu(YellowGame* g) {
    /* Recovery at an existing, real heal point clears the volatile blackout
     * latch; this is NOT the unported automatic Gen1 whiteout route. */
    g->overworld_blackout_pending=0u;
    g->hp=yellow_player_max_hp(g);
    for(unsigned i=0;i<YELLOW_MOVE_SLOTS;i++) {
        g->pika_pp[i]=y_max_pp(g->pika_moves[i]);
    }
    g->pika_status=YStatusNone;
    for(unsigned i=0;i<g->caught_count;i++) {
        YCapturedMon* mon=&g->caught[i];
        mon->status=YStatusNone;
        mon->hp=y_calc_stats(mon->species,mon->level).hp;
        /* Captured low-level species initially have their original starter moves. */
        for(unsigned j=0;j<YELLOW_MOVE_SLOTS;j++)
            mon->pp[j]=y_max_pp(mon->moves[j]);
    }
}
uint8_t yellow_party_size(const YellowGame* g) {
    return (uint8_t)(g->has_pikachu+g->caught_count);
}
static uint8_t first_healthy_party(const YellowGame* g);
void yellow_begin_battle(YellowGame* g,uint8_t kind,uint8_t level) {
    if((kind==YK_PIKACHU || kind==13 || kind>YK_CLEFAIRY)||!level||level>100) return;
    uint8_t first=first_healthy_party(g);
    if(first==255)return;
    if(yellow_party_hp(g,g->active_party)==0 ||
       g->active_party>=yellow_party_size(g))g->active_party=first;
    (void)yellow_dex_mark_seen(&g->dex,yellow_dex_number(kind));
    g->foe_kind=kind;g->foe_level=level;
    g->foe_max_hp=y_calc_stats(kind,level).hp;
    g->foe_hp=g->foe_max_hp;
    g->foe_attack_stage=0;g->foe_defense_stage=0;
    g->foe_speed_stage=0;g->pika_speed_stage=0;
    g->player_accuracy_stage=0;g->foe_accuracy_stage=0;
    g->player_evasion_stage=0;g->foe_evasion_stage=0;
    g->foe_flinched=0;
    g->foe_confusion_turns=0;
    g->foe_confusion_turns=0;
    memset(g->party_confusion_turns,0,sizeof(g->party_confusion_turns));
    g->foe_status=YStatusNone;
    g->pika_attack_stage=0;g->pika_defense_stage=0;
    g->foe_attack_debuff=0;
    g->last_exp_award=0;
    g->learn_count=0;g->learn_index=0;
    g->last_critical=0;g->last_enemy_move=YM_NONE;g->last_player_first=0;
    g->participants_mask=(uint8_t)(1u<<g->active_party);
    g->party_battle=0;g->force_switch=0;
    g->selected=0;g->screen=YScreenRivalBattle;
}
uint8_t yellow_party_species(const YellowGame* g,uint8_t i) {
    if(i==0)return YK_PIKACHU;
    return i<=g->caught_count?g->caught[i-1u].species:255u;
}
uint8_t yellow_party_level(const YellowGame* g,uint8_t i) {
    if(i==0)return g->pika_level;
    return i<=g->caught_count?g->caught[i-1u].level:0u;
}
uint8_t yellow_party_hp(const YellowGame* g,uint8_t i) {
    if(i==0)return g->hp;
    return i<=g->caught_count?g->caught[i-1u].hp:0u;
}
uint8_t yellow_party_max_hp(const YellowGame* g,uint8_t i) {
    uint8_t species=yellow_party_species(g,i),level=yellow_party_level(g,i);
    return y_species_info(species) && level?y_calc_stats(species,level).hp:0u;
}
uint32_t yellow_party_exp(const YellowGame* g,uint8_t i) {
    if(i==0)return g->experience;
    return i<=g->caught_count?g->caught[i-1u].experience:0;
}
uint8_t yellow_party_move(const YellowGame* g,uint8_t i,uint8_t slot) {
    if(slot>=YELLOW_MOVE_SLOTS)return YM_NONE;
    if(i==0)return g->pika_moves[slot];
    return i<=g->caught_count?g->caught[i-1u].moves[slot]:YM_NONE;
}
uint8_t yellow_party_status(const YellowGame* g,uint8_t i) {
    if(i==0)return g->pika_status;
    return i<=g->caught_count?g->caught[i-1u].status:YStatusNone;
}
uint8_t yellow_party_pp(const YellowGame* g,uint8_t i,uint8_t slot) {
    if(slot>=YELLOW_MOVE_SLOTS)return 0;
    if(i==0)return g->pika_pp[slot];
    return i<=g->caught_count?g->caught[i-1u].pp[slot]:0;
}
static void set_party_status(YellowGame* g,uint8_t i,uint8_t status) {
    if(i==0)g->pika_status=status;
    else if(i<=g->caught_count)g->caught[i-1u].status=status;
}
static uint8_t* pp_location(YellowGame* g,uint8_t i,uint8_t slot) {
    return i==0?&g->pika_pp[slot]:&g->caught[i-1u].pp[slot];
}
static void set_party_hp(YellowGame* g,uint8_t i,uint8_t hp) {
    if(i==0)g->hp=hp;
    else g->caught[i-1u].hp=hp;
}
uint8_t yellow_active_move_count(const YellowGame* g) {
    uint8_t count=0;
    for(uint8_t slot=0;slot<YELLOW_MOVE_SLOTS;slot++) {
        if(yellow_party_move(g,g->active_party,slot)==YM_NONE)break;
        count++;
    }
    return count;
}
bool yellow_can_switch(const YellowGame* g,uint8_t i) {
    return i<yellow_party_size(g) && i!=g->active_party &&
        yellow_party_hp(g,i)>0;
}
static uint8_t first_healthy_party(const YellowGame* g) {
    for(uint8_t i=0;i<yellow_party_size(g);i++)
        if(yellow_party_hp(g,i)>0)return i;
    return 255;
}
static void reset_player_stages(YellowGame* g) {
    g->pika_attack_stage=0;g->pika_defense_stage=0;
    g->player_accuracy_stage=0;
    g->player_evasion_stage=0;
}
static void yellow_learn_for_party(YellowGame* g,uint8_t party_index,uint8_t move) {
    if(move==YM_NONE)return;
    uint8_t* known=party_index==0?g->pika_moves:g->caught[party_index-1u].moves;
    uint8_t* pp=party_index==0?g->pika_pp:g->caught[party_index-1u].pp;
    for(uint8_t i=0;i<YELLOW_MOVE_SLOTS;i++) {
        if(known[i]==move)return;
        if(known[i]==YM_NONE) {
            known[i]=move;
            pp[i]=y_max_pp(move);
            return;
        }
    }
    /* Queued until the battle victory text is acknowledged.  Never
       silently overwrite a move.  64 slots cover all currently ported
       learnset events for the whole six-Pokemon party. */
    if(g->learn_count<YELLOW_LEARN_QUEUE_CAPACITY) {
        uint8_t n=g->learn_count++;
        g->learn_party[n]=party_index;
        g->learn_moves[n]=move;
    }
}
/* All Pokémon that participated AND survived split victory EXP here.
   Gen1 trainer/EXP bonuses and nonparticipant mechanics remain simplified. */
static void grant_exp(YellowGame* g) {
    if(!g->has_pikachu)return;
    uint8_t recipients[YELLOW_PARTY_MAX],num=0;
    for(uint8_t i=0;i<yellow_party_size(g);i++) {
        if((g->participants_mask & (1u<<i)) && yellow_party_hp(g,i)>0)
            recipients[num++]=i;
    }
    if(!num)return;
    uint16_t total=y_exp_award(g->foe_kind,g->foe_level,g->foe_kind==YK_EEVEE);
    g->last_exp_award=total;
    for(uint8_t r=0;r<num;r++) {
        uint8_t i=recipients[r];
        uint8_t species=yellow_party_species(g,i);
        uint8_t level=yellow_party_level(g,i);
        uint32_t exp=yellow_party_exp(g,i);
        uint32_t gain=total/num+(r<(total%num)?1u:0u);
        exp+=gain;
        while(level<100 && exp>=y_exp_threshold(species,(uint8_t)(level+1u))) {
            uint8_t old_hp=y_calc_stats(species,level).hp;
            level++;
            uint8_t new_hp=y_calc_stats(species,level).hp;
            uint8_t hp=yellow_party_hp(g,i);
            set_party_hp(g,i,(uint8_t)(hp+(new_hp-old_hp)));
            yellow_learn_for_party(g,i,y_learned_move_at(species,level));
        }
        if(i==0){g->experience=exp;g->pika_level=level;}
        else {
            g->caught[i-1u].experience=exp;
            g->caught[i-1u].level=level;
            /* Evolution follows experience and level-up, without reentering
               the experience loop or changing starter-Pikachu behavior. */
            /* Share the authoritative supported-evolution contract with
               battle/source helpers; do not fork species rules here. */
            uint8_t next=y_evolution_at(species,level);
            if(next!=species) {
                /* HP-only scalar formula: avoid two struct-return calls
                   across live TinyCC byte locals (a known codegen hazard).
                   Identical to Gen1 zero-StatExp HP in y_calc_stats. */
                const YSpeciesInfo* old_info=y_species_info(species);
                const YSpeciesInfo* new_info=y_species_info(next);
                uint32_t old_calc=((2u*old_info->hp+15u)*level)/100u+level+10u;
                uint32_t new_calc=((2u*new_info->hp+15u)*level)/100u+level+10u;
                uint8_t old_max=(uint8_t)(old_calc>255u?255u:old_calc);
                uint8_t new_max=(uint8_t)(new_calc>255u?255u:new_calc);
                uint16_t new_hp=(uint16_t)g->caught[i-1u].hp+new_max-old_max;
                g->caught[i-1u].hp=(uint8_t)(new_hp>new_max?new_max:new_hp);
                g->caught[i-1u].species=next;
                if(next==YK_METAPOD)yellow_learn_for_party(g,i,YM_HARDEN);
            }
        }
    }
}

/* Pinned pret/pokeyellow BugCatcherData forest parties (numbers 1,2,3).
   Remaining Lass / later Bug Catcher use unsupported species and are NOT
   falsely represented by invented encounters in this bounded stage. */
static const uint8_t forest_party_species[5][3]={
    {YK_CATERPIE,YK_CATERPIE,0},
    {YK_METAPOD,YK_CATERPIE,YK_METAPOD},
    {YK_CATERPIE,0,0},
    {YK_CATERPIE,YK_METAPOD,0},
    {YK_NIDORAN_F,YK_NIDORAN_M,0} /* original Lass #19 */
};
static const uint8_t forest_party_levels[5]={7,6,10,8,6};
static const uint8_t forest_party_counts[5]={2,3,1,2,2};

/* Runtime party is read from the authenticated CRC-checked source catalog
   when available; the pinned original Lass team remains an offline fallback. */
bool yellow_attach_bulk_lass(YellowGame* g,const YellowBulkLass* imported) {
    if(!g)return false;
    g->imported_lass=NULL;
    if(!imported)return true;
    if(imported->count!=2 ||
       imported->kind[0]!=YK_NIDORAN_F ||
       imported->kind[1]!=YK_NIDORAN_M ||
       imported->level[0]!=6 || imported->level[1]!=6)return false;
    g->imported_lass=imported;
    return true;
}
static void forest_party_mon(const YellowGame* g,uint8_t trainer,uint8_t index,
                              uint8_t* kind,uint8_t* level) {
    /* All five supported forest trainers resolve from the immutable pinned
       original class/party data, not fabricated level/party arrays. */
    if(!y_source_forest_mon(trainer,index,kind,level)) {
        *kind=forest_party_species[trainer][index];
        *level=forest_party_levels[trainer];
    }
    if(trainer==4 && g->imported_lass && index<g->imported_lass->count) {
        *kind=g->imported_lass->kind[index];
        *level=g->imported_lass->level[index];
    }
}
static int forest_trainer_at(int x,int y) {
    if(x==30&&y==33)return 0;
    if(x==30&&y==19)return 1;
    if(x==2&&y==18)return 2;
    if(x==13&&y==17)return 3;
    if(x==2&&y==41)return 4;
    return -1;
}
static void forest_start_battle(YellowGame* g,uint8_t trainer) {
    if(trainer>=5 || (g->forest_defeated_mask&(1u<<trainer)))return;
    if(!g->has_pikachu||first_healthy_party(g)==255)return;
    g->forest_active_trainer=trainer;
    g->forest_party_index=0;
    uint8_t kind,level;
    forest_party_mon(g,trainer,0,&kind,&level);
    yellow_begin_battle(g,kind,level);
}

/* Native species enums differ from original Pokedex source IDs:
   source party: Jr Diglett #50/Sandshrew #27; Brock Geodude #74/Onix #95. */
static void pewter_party_mon(uint8_t trainer,uint8_t slot,uint8_t* species,uint8_t* level) {
    static const uint8_t kinds[2][2]={{YK_DIGLETT,YK_SANDSHREW},{YK_GEODUDE,YK_ONIX}};
    static const uint8_t levels[2][2]={{9,9},{10,12}};
    *species=kinds[trainer][slot];*level=levels[trainer][slot];
}
static const char* const pewter_jr_challenge[]={"PEWTER GYM","JUNIOR TRAINER!"};
static const char* const pewter_brock_challenge[]={"BROCK: LET'S BATTLE!","ROCK HARD TEAM!"};
static const char* const pewter_jr_after[]={"JUNIOR TRAINER","DEFEATED!"};
static const char* const pewter_brock_after[]={"BOULDER BADGE!","TM34 BIDE AWAITS!"};
static const char* const pewter_gym_loss[]={"GYM BATTLE LOST!","RECOVER AND RETURN!"};

static const char* const route3_challenge[]={"ROUTE 3 TRAINER!","PREPARE TO BATTLE!"};
static const char* const route3_after[]={"ROUTE 3 TRAINER","DEFEATED!"};
static const char* const route3_loss[]={"BATTLE LOST!","RECOVER AND TRY AGAIN."};
static void route3_start_battle(YellowGame* g,unsigned index) {
    const YRoute3Trainer* team=route3_trainer(index);
    if(!team || g->map!=YMapRoute3 || g->route3_active_trainer ||
       (g->route3_defeated_mask&(1u<<team->original_index)) ||
       !g->has_pikachu || first_healthy_party(g)==255)return;
    g->route3_active_trainer=(uint8_t)(index+1u);
    g->route3_party_index=0;
    yellow_begin_battle(g,team->species[0],team->levels[0]);
}
static int route3_trainer_sight(const YellowGame* g) {
    if(g->map!=YMapRoute3 || g->screen!=YScreenWorld ||
       g->story!=YStoryPikachuFollows || !g->has_pikachu)return -1;
    for(int i=0;i<3;i++) {
        const YRoute3Trainer* t=&route3_supported[i];
        if(g->route3_defeated_mask&(1u<<t->original_index))continue;
        int dx=t->facing==YLeft?-1:t->facing==YRight?1:0;
        int dy=t->facing==YUp?-1:t->facing==YDown?1:0;
        int distance=(g->x-(int)t->x)*dx+(g->y-(int)t->y)*dy;
        if(distance<1||distance>t->range)continue;
        if((dx && g->y!=t->y)||(dy && g->x!=t->x))continue;
        bool clear=true;
        for(int j=1;j<distance;j++) {
            int x=(int)t->x+j*dx,y=(int)t->y+j*dy;
            if(!base_passable(YMapRoute3,x,y)||npc_at(g,x,y)){
                clear=false;break;
            }
        }
        if(clear)return i;
    }
    return -1;
}
static void pewter_start_battle(YellowGame* g,uint8_t trainer) {
    if(trainer>1 || !g->has_pikachu || first_healthy_party(g)==255)return;
    if(trainer==0 && (g->pewter_flags&YPewterFlagGymTrainer0))return;
    if(trainer==1 && (g->pewter_flags&YPewterFlagBrock))return;
    g->pewter_active_trainer=(uint8_t)(trainer+1);
    g->pewter_party_index=0;
    uint8_t kind,level;
    pewter_party_mon(trainer,0,&kind,&level);
    yellow_begin_battle(g,kind,level);
}
static void finalize_battle(YellowGame* g,bool won) {
    if(won)grant_exp(g);
    if(g->route3_active_trainer) {
        unsigned i=(unsigned)g->route3_active_trainer-1u;
        const YRoute3Trainer* trainer=route3_trainer(i);
        if(trainer && won && g->route3_party_index+1u<trainer->count) {
            ++g->route3_party_index;
            yellow_begin_battle(g,trainer->species[g->route3_party_index],
                                   trainer->levels[g->route3_party_index]);
            return;
        }
        if(trainer && won)
            g->route3_defeated_mask|=(uint8_t)(1u<<trainer->original_index);
        if(!won) {
            yellow_heal_pikachu(g);
            g->active_party=0;
            set_map(g,YMapRedsHouse,3,6);
        }
        g->route3_active_trainer=0;g->route3_party_index=0;
        g->foe_hp=0;g->party_battle=0;g->force_switch=0;
        open_dialog(g,won?route3_after:route3_loss,1,YPendingEndWildBattle);
        return;
    }
    if(g->pewter_active_trainer!=0) {
        uint8_t trainer=(uint8_t)(g->pewter_active_trainer-1u);
        if(won && g->pewter_party_index==0u) {
            ++g->pewter_party_index;
            uint8_t kind,level;
            pewter_party_mon(trainer,g->pewter_party_index,&kind,&level);
            yellow_begin_battle(g,kind,level);
            return;
        }
        if(won) {
            if(trainer==0) g->pewter_flags|=YPewterFlagGymTrainer0;
            else g->pewter_flags|=YPewterFlagBrock|YPewterFlagBoulderBadge;
        } else {
            yellow_heal_pikachu(g);g->active_party=0;
            set_map(g,YMapRedsHouse,3,6);
        }
        g->pewter_active_trainer=0;
        g->pewter_party_index=0;
        g->foe_hp=0;g->party_battle=0;g->force_switch=0;
        open_dialog(g,won?(trainer==0?pewter_jr_after:pewter_brock_after):pewter_gym_loss,
                    1,(won && trainer==1)?YPendingGrantPewterTM34:YPendingEndWildBattle);
        return;
    }
    if(g->forest_active_trainer<5) {
        uint8_t t=g->forest_active_trainer;
        const YSourceForestParty* source=y_source_forest_party(t);
        if(won && g->forest_party_index+1u<
           (source?source->count:forest_party_counts[t])) {
            ++g->forest_party_index;
            uint8_t kind,level;
            forest_party_mon(g,t,g->forest_party_index,&kind,&level);
            yellow_begin_battle(g,kind,level);
            return;
        }
        if(won)g->forest_defeated_mask|=(uint8_t)(1u<<t);
        else {
            yellow_heal_pikachu(g);
            g->active_party=0;
            set_map(g,YMapRedsHouse,3,6);
        }
        g->forest_active_trainer=255;
        g->foe_hp=0;g->party_battle=0;g->force_switch=0;
        open_dialog(g,t==4?(won?forest_lass_victory:forest_lass_defeat):
                    (won?forest_trainer_victory:forest_trainer_defeat),
                    1,YPendingEndWildBattle);
        return;
    }
    if(y_status_is_toxic(g->pika_status))g->pika_status=YStatusPoisoned;
    for(uint8_t i=0;i<g->caught_count;i++)
        if(y_status_is_toxic(g->caught[i].status))
            g->caught[i].status=YStatusPoisoned;
    g->foe_hp=0;g->foe_attack_debuff=0;
    g->party_battle=0;g->force_switch=0;
    if(g->foe_kind!=0) {
        if(!won) {
            yellow_heal_pikachu(g);
            g->active_party=0;
            set_map(g,YMapRedsHouse,3,6);
        }
        open_dialog(g,won?wild_victory:wild_defeat,1,YPendingEndWildBattle);
    } else {
        if(!won){yellow_heal_pikachu(g);g->active_party=0;}
        open_dialog(g,won?victory_text:defeat_text,1,YPendingEndBattle);
    }
}
static void resolve_pending(YellowGame* g) {
    /* A won battle cannot end until every learn/forget prompt is handled.
       Suppress movement and the ordinary menu during these prompts. */
    if((g->pending==YPendingEndBattle||g->pending==YPendingEndWildBattle) &&
       g->learn_index<g->learn_count) {
        g->screen=YScreenMoveForget;
        g->selected=0;
        return;
    }
    YellowPending pending=g->pending;
    g->pending=YPendingNone;
    g->learn_count=0;g->learn_index=0;
    g->screen=YScreenWorld;
    switch(pending) {
    case YPendingBeginCapture:
        g->story=YStoryOakEncounter;g->screen=YScreenOakEncounter;break;
    case YPendingEnterLab:
        g->story=YStoryChooseStarter;
        set_map(g,YMapOaksLab,4,8);
        open_dialog(g,lab_intro,3,YPendingNone);
        break;
    case YPendingStartBattle:
        g->story=YStoryRivalBattle;
        g->has_pikachu=1;yellow_heal_pikachu(g);
        (void)yellow_dex_mark_caught(&g->dex,yellow_dex_number(YK_PIKACHU));
        yellow_begin_battle(g,YK_EEVEE,5);
        open_dialog(g,challenge_text,2,YPendingBeginRivalBattle);
        break;
    case YPendingBeginRivalBattle:
        g->screen=YScreenRivalBattle;
        break;
    case YPendingBeginForestLassBattle:
        forest_start_battle(g,4);
        break;
    case YPendingBeginPewterJrBattle:
        pewter_start_battle(g,0);
        break;
    case YPendingBeginPewterBrockBattle:
        pewter_start_battle(g,1);
        break;
    case YPendingBeginRoute3TrainerBattle:
        if(g->route3_pending_trainer) {
            unsigned index=(unsigned)g->route3_pending_trainer-1u;
            g->route3_pending_trainer=0;
            route3_start_battle(g,index);
        }
        break;
    case YPendingGrantPewterTM34:
        /* Original PewterGymScriptReceiveTM34 gives Bide only after the
           Brock victory event. The limited present bag contains fewer
           than twenty distinct items, so one machine slot is available.
           y_pewter_claim_tm34 enforces one-time grant and badge gates. */
        if(!(g->pewter_flags&~YPewterAllFlags) &&
           (g->pewter_flags&(YPewterFlagBrock|YPewterFlagBoulderBadge))==
            (YPewterFlagBrock|YPewterFlagBoulderBadge) &&
           !(g->pewter_flags&YPewterFlagTM34)) {
            /* Same rule as the original sidecar's claim operation. The
               standalone core test harness intentionally does not link the
               external persistence module; only flags are mutated here. */
            g->pewter_flags|=YPewterFlagTM34;
            /* Return to overworld just as the accepted v040 victory
               acknowledgement does. Inventory displays the new TM. */
        }
        break;
    case YPendingBuyAntidote:
        /* Recheck every gate at commit time, not only on offer. */
        if(g->map==YMapViridianMart && g->story==YStoryPikachuFollows &&
           g->has_pikachu && g->x==2 && g->y==5 && g->facing==YLeft &&
           yellow_game_buy_antidote && yellow_game_buy_antidote(g,1u))
            open_dialog(g,mart_antidote_bought,1,YPendingNone);
        else open_dialog(g,mart_antidote_denied,1,YPendingNone);
        break;
    case YPendingDeliverOakParcel:
        /* Original EVENT_OAK_GOT_PARCEL: only on completed, real Oak
           dialogue in the Lab; sidecar tracks item removal atomically
           with the next native save, never modifies YellowSave v13. */
        if(g->map==YMapOaksLab && g->story==YStoryPikachuFollows &&
           g->has_pikachu && !g->oak_parcel_integrity_error &&
           g->oak_parcel_flags==YOakParcelGot)
            (void)y_oak_parcel_record_delivered(&g->oak_parcel_flags);
        break;
    case YPendingGrantOakParcel:
        /* Original EVENT_GOT_OAKS_PARCEL is set only after the clerk's
           scripted path AND the parcel dialog have both completed.
           A corrupt sidecar never grants an item; v13 remains unchanged. */
        if(g->native_mart_intro_extension &&
           g->map==YMapViridianMart && g->x==2 && g->y==4 &&
           g->story==YStoryPikachuFollows && g->has_pikachu &&
           !g->oak_parcel_integrity_error && g->oak_parcel_flags==0u)
            (void)y_oak_parcel_record_got(&g->oak_parcel_flags);
        break;
    case YPendingEndWildBattle:
        /* Wild battle ended; only state in the local overworld resumes. */
        break;
    case YPendingEndBattle:
        g->story=YStoryPikachuFollows;
        if(!g->balls)g->balls=5; /* temporary developer starter kit */
        set_follower_near(g);
        open_dialog(g,proto_kit,3,YPendingNone);
        break;
    case YPendingNone:default:break;
    }
    /* Original ViridianMartDefaultScript simulates LEFT 1, UP 2 after
       greeting, then ViridianMartOaksParcelScript gives the item. Run the
       *real native input path*, not a teleport or an invented map ID.
       This is an atomic 3-input prototype: per-frame animation is pending.
       Never grant if a collision, map transition or blackout interrupts. */
    if(pending==YPendingNone && g->native_mart_intro_pending) {
        g->native_mart_intro_pending=0u;
        if(g->screen==YScreenWorld && g->map==YMapViridianMart &&
           g->x==3 && g->y==6 && g->story==YStoryPikachuFollows &&
           g->has_pikachu && g->oak_parcel_flags==0u &&
           !g->oak_parcel_integrity_error) {
            static const YellowKey dirs[]={YLeft,YUp,YUp};
            static const uint8_t px[]={2,2,2},py[]={6,5,4};
            bool ok=true;
            for(unsigned i=0;i<3;i++) {
                if(g->screen!=YScreenWorld){ok=false;break;}
                uint16_t steps=g->steps;
                yellow_input(g,dirs[i]);
                if(g->screen!=YScreenWorld || g->map!=YMapViridianMart ||
                   g->x!=px[i] || g->y!=py[i] ||
                   (uint16_t)(g->steps-steps)!=1u){ok=false;break;}
            }
            if(ok)open_dialog(g,mart_clerk_parcel_text,3,YPendingGrantOakParcel);
        }
    }
}
/* Pinned Viridian Forest trainers facing LEFT/RIGHT (not ordinary NPCs). */
static bool forest_trainer_sight(const YellowGame* g) {
    static const struct {uint8_t x,y,facing,range;} trainers[]={
        {30,33,YLeft,4},{30,19,YLeft,4},{2,18,YLeft,1},
        {13,17,YRight,4}
    };
    if(g->map!=YMapViridianForest || g->screen!=YScreenWorld ||
       g->story!=YStoryPikachuFollows || !g->has_pikachu)return false;
    for(unsigned i=0;i<sizeof(trainers)/sizeof(trainers[0]);i++) {
        int tx=trainers[i].x,ty=trainers[i].y;
        int vx=trainers[i].facing==YLeft?-1:1;
        int distance=(g->x-tx)*vx;
        if(g->y!=ty || distance<1 || distance>trainers[i].range)continue;
        bool clear=true;
        for(int j=1;j<distance;j++) {
            int x=tx+j*vx;
            if(!base_passable(YMapViridianForest,x,ty) || npc_at(g,x,ty)) {
                clear=false;break;
            }
        }
        if(clear) {
            if((g->forest_defeated_mask&(1u<<i)) ||
               (g->forest_seen_mask&(1u<<i)))continue;
            return true;
        }
    }
    return false;
}

/* Source header Route4 east CeruleanCity -4, Cerulean west Route4 +4.
 * y_Cerulean=y_Route4+8 on original 16x16 step grid.
 * World edge transitions require BOTH original collision-open endpoints;
 * interior Cerulean warps remain disabled until their destinations exist.
 */
static bool yellow_route4_cerulean_connection(uint8_t map,int x,int y,
                                              uint8_t* next,uint8_t* nx,uint8_t* ny){
  if(!next||!nx||!ny)return false;
  if(map==YMapRoute4&&x==90&&y>=0&&y<18 &&
     yellow_route4_passable(89,y) && yellow_cerulean_city_passable(0,y+8)){
      *next=YMapCeruleanCity;*nx=0;*ny=(uint8_t)(y+8);return true;
  }
  if(map==YMapCeruleanCity&&x==-1&&y>=8&&y<26 &&
     yellow_cerulean_city_passable(0,y) && yellow_route4_passable(89,y-8)){
      *next=YMapRoute4;*nx=89;*ny=(uint8_t)(y-8);return true;
  }
  return false;
}
/* Source headers: Cerulean City east Route9, +4;
 * Route9 west Cerulean, -4 (Gen1 block offset).
 * On 16px steps, y_Route9=y_Cerulean-8.
 * Pass both source-verified collision endpoints; all other edges fail closed.
 */
static bool yellow_cerulean_route9_connection(uint8_t map,int x,int y,
                                             uint8_t* to,uint8_t* nx,uint8_t* ny){
 if(!to||!nx||!ny)return false;
 if(map==YMapCeruleanCity&&x==40&&y>=8&&y<26&&
    yellow_cerulean_city_passable(39,y)&&
    yellow_route9_passable(0,y-8)){
   *to=YMapRoute9;*nx=0;*ny=(uint8_t)(y-8);return true;
 }
 if(map==YMapRoute9&&x==-1&&y>=0&&y<18&&
    yellow_route9_passable(0,y)&&
    yellow_cerulean_city_passable(39,y+8)){
   *to=YMapCeruleanCity;*nx=39;*ny=(uint8_t)(y+8);return true;
 }
 return false;
}
/* Pinned Route9 east Route10 west, offset 0. Source Overworld edge.
 * Original native walk cells: Route9 x=59 and Route10 x=0, shared y=8,9.
 * All other border cells remain blocked by both source collision masks. */
static bool yellow_route9_route10_connection(uint8_t map,int x,int y,
                                             uint8_t* to,uint8_t* nx,uint8_t* ny){
 if(!to||!nx||!ny)return false;
 if(y<0||y>=18||!yellow_route9_passable(59,y)||!yellow_route10_passable(0,y))return false;
 if(map==YMapRoute9&&x==60){*to=YMapRoute10;*nx=0;*ny=(uint8_t)y;return true;}
 if(map==YMapRoute10&&x==-1){*to=YMapRoute9;*nx=59;*ny=(uint8_t)y;return true;}
 return false;
}

static bool yellow_route22_viridian_edge(int y){
 if(y<0||y>=36)return false;
 YellowGame state={0};
 state.map=YMapViridian;
 return yellow_walkable(&state,0,y);
}
/* Original Gen1 Viridian City west Route22 +4, Route22 east Viridian -4.
 * y_Viridian=y_Route22+8. Both original collision endpoints required.
 */
static bool yellow_viridian_route22_connection(uint8_t map,int x,int y,
                                                uint8_t* to,uint8_t* nx,uint8_t* ny){
 if(!to||!nx||!ny)return false;
 if(map==YMapViridian&&x==-1&&y>=8&&y<26&&
    yellow_route22_viridian_edge(y)&&yellow_route22_passable(39,y-8)){
   *to=YMapRoute22;*nx=39;*ny=(uint8_t)(y-8);return true;
 }
 if(map==YMapRoute22&&x==40&&y>=0&&y<18&&
    yellow_route22_passable(39,y)&&yellow_route22_viridian_edge(y+8)){
   *to=YMapViridian;*nx=0;*ny=(uint8_t)(y+8);return true;
 }
 return false;
}
/* Exact eight original Overworld LedgeTiles records from pinned
 * pret/pokeyellow/data/tilesets/ledge_tiles.asm. Direction must
 * already match the player's facing before this keypress.
 * Never synthesize UP or an arbitrary collision override.
 */
static bool yellow_route9_ledge_rule(YellowKey key,uint8_t standing,uint8_t front) {
    if(key==YDown)
        return (standing==0x2c && front==0x37) ||
               (standing==0x39 && (front==0x36 || front==0x37));
    if(key==YLeft)
        return (standing==0x2c || standing==0x39) && front==0x27;
    if(key==YRight)
        return (standing==0x2c && (front==0x0d || front==0x1d)) ||
               (standing==0x39 && front==0x0d);
    return false;
}

static void do_step(YellowGame* g,YellowKey key) {
    int x=g->x,y=g->y;
    uint8_t previous_facing=g->facing;
    g->facing=(uint8_t)key;
    if(key==YUp)--y;
    else if(key==YDown)++y;
    else if(key==YLeft)--x;
    else if(key==YRight)++x;

    /* The original tile atlas uses 8px coordinates; g->x/y are 16px
     * walkable world steps. Match the same lower-left tile sampling as
     * yellow_is_grass and the independent pinned Route9 graph oracle.
     * A ledge is a 2-cell hop, not 1+2=3. Normal collision, follower
     * handling and later movement effects remain in the shared path.
     */
    bool route9_ledge_hop=false;
    if(g->map==YMapRoute9 && previous_facing==(uint8_t)key &&
       (key==YDown || key==YLeft || key==YRight)) {
        const uint8_t standing=yellow_route9_tile_id((int)g->x*2,(int)g->y*2+1);
        const uint8_t front=yellow_route9_tile_id(x*2,y*2+1);
        if(yellow_route9_ledge_rule(key,standing,front)) {
            const int landing_x=(int)g->x+2*(x-(int)g->x);
            const int landing_y=(int)g->y+2*(y-(int)g->y);
            if(!yellow_walkable(g,landing_x,landing_y))return;
            x=landing_x;y=landing_y;route9_ledge_hop=true;
        }
    }

    /* Original City Mart door: keep historical yellow_original_warp_at
     * graph immutable. New reachable door is a separate native gameplay
     * transition; protected legacy graph intentionally excluded it.
     * ViridianCity warp2 (29,19) -> Mart warp1, Mart LAST_MAP -> City.
     */
    if(g->story==YStoryPikachuFollows){
        if(g->map==YMapViridian && (key==YDown||key==YUp) && x==29 && y==19){
            set_map(g,YMapViridianMart,3,6);++g->steps;return;
        }
        if(g->map==YMapViridianMart && key==YDown &&
           (x==3||x==4) && y==7){
            set_map(g,YMapViridian,29,20);++g->steps;return;
        }
    }
    /* Original source Cerulean City warp6 (25,25) -> Cerulean Mart warp1;
     * both Mart door tiles (3,7),(4,7) LAST_MAP -> City warp6.
     * Source maps Viridian Mart and Cerulean Mart are byte-identical:
     * one immutable shared 4x4 Mart tile/collision atlas is correct.
     * Does NOT enable shop inventory, NPCs, or invented door destinations.
     */
    if(g->story==YStoryPikachuFollows){
        if(g->map==YMapCeruleanCity && (key==YUp||key==YDown) &&
           x==25 && y==25){
            set_map(g,YMapCeruleanMart,3,6);++g->steps;return;
        }
        if(g->map==YMapCeruleanMart && key==YDown &&
           (x==3||x==4) && y==7){
            set_map(g,YMapCeruleanCity,25,26);++g->steps;return;
        }
    }
    if(g->map==YMapPallet&&y==0&&(x==10||x==11)&&g->story==YStoryBeforeOak) {
        open_dialog(g,oak_warning,3,YPendingBeginCapture);
        return;
    }
    /* REAL original Route3 north / Route4 south; no cave jump.
       These guarded corridors were derived from all source block
       collisions at both edges. */
    if((key==YUp||key==YDown) && g->story==YStoryPikachuFollows) {
        uint8_t next,tx,ty;
        if(yellow_route4_vertical_connection(g->map,x,y,&next,&tx,&ty)) {
            set_map(g,next,tx,ty);
            ++g->steps;
            return;
        }
    }
    /* Pinned original data/maps/headers connection offsets, including
       Route1 <-> Pallet/Viridian and Viridian <-> Route2. The old narrow
       exit corridors and story gates are retained byte-for-byte in the
       deterministic imported adapter. New map edges are NOT enabled. */
    if(key==YDown || (key==YUp && g->story==YStoryPikachuFollows)) {
        uint8_t next_map=0,next_x=0,next_y=0;
        if(yellow_source_connection(g->map,key==YUp?0u:1u,x,y,
                                    &next_map,&next_x,&next_y)) {
            set_map(g,next_map,next_x,next_y);
            g->steps++;
            return;
        }
    }
    /* The opposite header offsets are source-backed in both directions.
       Do not require a special warp: actual directional input crosses. */
    if((key==YRight||key==YLeft) && g->story==YStoryPikachuFollows) {
        uint8_t to,tx,ty;
        if(yellow_route3_horizontal_connection(g->map,x,y,&to,&tx,&ty)) {
            set_map(g,to,tx,ty);
            ++g->steps;
            return;
        }
    }
    /* Original Viridian <-> Route22 boundary, no speculative Route23 exit. */
    if((key==YRight||key==YLeft)&&g->story==YStoryPikachuFollows){
        uint8_t to,tx,ty;
        if(yellow_viridian_route22_connection(g->map,x,y,&to,&tx,&ty)){
            set_map(g,to,tx,ty);++g->steps;return;
        }
    }
    /* Connected original Cerulean <-> Route9, no fabricated route10 exit. */
    if((key==YRight||key==YLeft)&&g->story==YStoryPikachuFollows){
        uint8_t to,tx,ty;
        if(yellow_cerulean_route9_connection(g->map,x,y,&to,&tx,&ty)){
            set_map(g,to,tx,ty);++g->steps;return;
        }
    }
    /* Pinned Route9 east <-> Route10 west, original offset zero. */
    if((key==YRight||key==YLeft)&&g->story==YStoryPikachuFollows){
        uint8_t to,tx,ty;
        if(yellow_route9_route10_connection(g->map,x,y,&to,&tx,&ty)){
            set_map(g,to,tx,ty);++g->steps;return;
        }
    }
    /* New exact original Route4 <-> Cerulean boundary; no phantom doors. */
    if((key==YRight||key==YLeft)&&g->story==YStoryPikachuFollows){
        uint8_t to,tx,ty;
        if(yellow_route4_cerulean_connection(g->map,x,y,&to,&tx,&ty)){
            set_map(g,to,tx,ty);++g->steps;return;
        }
    }
    /* Gen1 OaksLabPlayerDontGoAwayScript prevents leaving before the
       starter choice. The original pushes the player back; this bounded
       adaptation keeps them on the previous tile and opens Oak's warning.
       No new story/save flags or speculative items are introduced. */
    if(g->map==YMapOaksLab && g->story==YStoryChooseStarter &&
       key==YDown && y==11 && (x==4||x==5)) {
        open_dialog(g,oak_dont_leave,1,YPendingNone);
        return;
    }
    /* First reversal toward a follower just faces it, allowing OK to talk.
       Repeating the same D-pad direction exchanges places instead of
       permanently trapping the player in a one-tile-wide passage. */
    if(g->story==YStoryPikachuFollows && g->follower_valid &&
       g->follower_map==g->map && x==g->follower_x && y==g->follower_y &&
       previous_facing!=(uint8_t)key) return;
    if(!yellow_walkable(g,x,y))return;
    uint8_t map=g->map,dx,dy;
    if(destination(g->map,x,y,&map,&dx,&dy)) {
        set_map(g,map,dx,dy);
    } else {
        if(g->story==YStoryPikachuFollows&&g->has_pikachu) {
            g->follower_map=g->map;g->follower_x=(uint8_t)g->x;
            g->follower_y=(uint8_t)g->y;g->follower_valid=1;
        }
        g->x=(int8_t)x;g->y=(int8_t)y;
    }
    g->steps+=(uint32_t)(route9_ledge_hop?2u:1u);
    /* Actual movement-triggered trainer line-of-sight. Battles come later. */
    /* Source-backed Route3 fighter sees a moving player on unobstructed
       cardinal lanes, not through terrain or other trainer objects. */
    int route3_sighting=route3_trainer_sight(g);
    if(route3_sighting>=0 && first_healthy_party(g)!=255) {
        g->route3_pending_trainer=(uint8_t)(route3_sighting+1);
        open_dialog(g,route3_challenge,1,YPendingBeginRoute3TrainerBattle);
        return;
    }
    if(forest_trainer_sight(g)) {
        /* Supported forest trainers: exact facing and sight range. */
        static const uint8_t tx[4]={30,30,2,13},ty[4]={33,19,18,17};
        static const uint8_t ranges[4]={4,4,1,4};
        static const int8_t directions[4]={-1,-1,-1,1};
        for(uint8_t i=0;i<4;i++) {
            int distance=(g->x-(int)tx[i])*directions[i];
            if(g->y==ty[i]&&distance>=1&&distance<=ranges[i])
                g->forest_seen_mask|=(uint8_t)(1u<<i);
        }
        dialog_id(g,TalkForestTrainer);return;
    }
    /* Cave encounters use original 10/256 rate and original ten
       weighted slots. Unsupported seven slots fail closed; never
       substitute/reweight Zubat or Clefairy into different species. */
    if((g->map==YMapRoute1||g->map==YMapViridianForest||
        yellow_mtmoon1f_is_active(g->map)) && g->has_pikachu &&
       first_healthy_party(g)!=255 &&
       (yellow_mtmoon1f_is_active(g->map) ||
        yellow_is_grass(g->map,g->x,g->y)) &&
       (next_rng(g)&255u)<(yellow_mtmoon1f_is_active(g->map)?
                             YELLOW_MTMOON_WILD_RATE:25u)) {
        uint8_t roll=(uint8_t)(next_rng(g)&255u);
        int slot=0, sum=0;
        while(slot<9) {
            sum+=encounter_weights[slot];
            if(roll<sum)break;
            ++slot;
        }
        uint8_t kind,level;
        if(yellow_wild_slot(g,g->map,(uint8_t)slot,&kind,&level))
            yellow_begin_battle(g,kind,level);
    }
}
bool yellow_is_grass(uint8_t map,int x,int y) {
    return (map==YMapRoute1 && yellow_tile_id(map,x*2,y*2+1)==0x52) ||
           (map==YMapViridianForest && yellow_tile_id(map,x*2,y*2+1)==0x20);
}
static void interact(YellowGame* g) {
    int x=g->x,y=g->y;
    if(g->facing==YUp)--y;
    else if(g->facing==YDown)++y;
    else if(g->facing==YLeft)--x;
    else if(g->facing==YRight)++x;

    if(g->story==YStoryPikachuFollows&&g->follower_valid&&
       g->follower_map==g->map&&x==g->follower_x&&y==g->follower_y) {
        if(g->affection<255)g->affection++;
        open_dialog(g,g->affection%3?pika_happy:pika_normal,1,YPendingNone);
        return;
    }
    /* Original Viridian Mart clerk at (0,5), facing RIGHT. Require
     * natural movement to (2,5), then face LEFT across counter (1,5).
     * The first OK only offers the item; a second OK commits purchase.
     * Never grant anything before EVENT_OAK_GOT_PARCEL. */
    if(g->map==YMapViridianMart && g->story==YStoryPikachuFollows &&
       g->has_pikachu && x==1 && y==5 && g->x==2 && g->y==5 &&
       g->facing==YLeft) {
        if(g->oak_parcel_integrity_error ||
           !(g->oak_parcel_flags&YOakParcelDelivered))
            open_dialog(g,mart_antidote_quest,1,YPendingNone);
        else open_dialog(g,mart_antidote_offer,1,YPendingBuyAntidote);
        return;
    }
    /* The Gen1 nurse talks across a one-step Pokecenter counter. */
    if(g->map==YMapViridianCenter&&x==3&&y==2&&g->facing==YUp) {
        if(g->native_nurse_choice_extension) {
            g->selected=0;g->screen=YScreenNurseChoice;return;
        }
        yellow_heal_pikachu(g);
        if(!g->last_blackout_integrity_error)g->last_blackout_original_map=1u;
        dialog_id(g,TalkViridianCenter);
        return;
    }
    if(g->map==YMapRoute3 && g->story==YStoryPikachuFollows) {
        int t=route3_trainer_at(x,y);
        if(t>=0) {
            const YRoute3Trainer* data=route3_trainer((unsigned)t);
            if(g->route3_defeated_mask&(1u<<data->original_index))
                open_dialog(g,route3_after,1,YPendingNone);
            else if(g->has_pikachu && first_healthy_party(g)!=255) {
                g->route3_pending_trainer=(uint8_t)(t+1);
                open_dialog(g,route3_challenge,1,YPendingBeginRoute3TrainerBattle);
            }
            return;
        }
    }
    if(g->map==YMapPewterGym && g->story==YStoryPikachuFollows) {
        uint8_t trainer=255;
        if(x==3&&y==6) trainer=0;
        else if(x==4&&y==1) trainer=1;
        if(trainer<2) {
            if(trainer==0 && (g->pewter_flags&YPewterFlagGymTrainer0))
                open_dialog(g,pewter_jr_after,1,YPendingNone);
            else if(trainer==1 && (g->pewter_flags&YPewterFlagBrock))
                open_dialog(g,pewter_brock_after,1,
                    (g->pewter_flags&YPewterFlagTM34)?YPendingNone:YPendingGrantPewterTM34);
            else if(g->has_pikachu && first_healthy_party(g)!=255)
                open_dialog(g,trainer==0?pewter_jr_challenge:pewter_brock_challenge,
                    1,trainer==0?YPendingBeginPewterJrBattle:YPendingBeginPewterBrockBattle);
            return;
        }
    }
    for(uint8_t i=0;i<yellow_npc_count;i++) {
        const YellowNpc* n=&yellow_npcs[i];
        if(n->x==x&&n->y==y&&yellow_npc_visible(g,n)) {
            uint8_t item=forest_item_bit(n);
            if(item) {
                uint8_t* count=(item==4u)?&g->balls:&g->potions;
                if(*count>=99u) {
                    open_dialog(g,forest_bag_full,1,YPendingNone);
                } else {
                    ++*count;
                    g->forest_pickups|=item;
                    open_dialog(g,item==4u?forest_ball_found:forest_potion_found,
                                1,YPendingNone);
                }
                return;
            }
            if(g->map==YMapViridianForest) {
                int trainer=forest_trainer_at(x,y);
                if(trainer==4 && g->has_pikachu && g->story==YStoryPikachuFollows) {
                    if(g->forest_defeated_mask&16u)
                        open_dialog(g,forest_lass_after,1,YPendingNone);
                    else
                        open_dialog(g,forest_lass_challenge,1,
                                    YPendingBeginForestLassBattle);
                    return;
                }
                /* A defeated Youngster #3 speaks his original post-battle
                   text instead of the generic TRAINER BATTLE TODO stub. */
                if(trainer==1 && (g->forest_defeated_mask&2u)) {
                    open_dialog(g,forest_youngster3_after,2,YPendingNone);
                    return;
                }
                if(trainer>=0 && g->has_pikachu &&
                   !(g->forest_defeated_mask&(1u<<trainer))) {
                    if(g->forest_seen_mask&(1u<<trainer)) {
                        forest_start_battle(g,(uint8_t)trainer);
                        return;
                    }
                    g->forest_seen_mask|=(uint8_t)(1u<<trainer);
                }
            }
            if(n->type==YActorNurse) {
                if(g->native_nurse_choice_extension) {
                    g->selected=0;g->screen=YScreenNurseChoice;return;
                }
                yellow_heal_pikachu(g);
                if(g->map==YMapViridianCenter && !g->last_blackout_integrity_error)
                    g->last_blackout_original_map=1u;
            }
            /* Original OaksLabOak1Text checks the parcel in Bag and
               consumes it when speaking to Oak. Never use the unrelated
               Oak at y=10 or invent a remote delivery event. */
            if(n->map==YMapOaksLab && n->type==YActorOak && n->y==2 &&
               g->story==YStoryPikachuFollows && g->has_pikachu &&
               !g->oak_parcel_integrity_error &&
               g->oak_parcel_flags==YOakParcelGot) {
                open_dialog(g,oak_parcel_delivery_text,2,YPendingDeliverOakParcel);
                return;
            }
            if(n->map==YMapOaksLab&&n->type==YActorPokeBall) {
                if(g->story==YStoryChooseStarter) {
                    open_dialog(g,starter_text,3,YPendingStartBattle);
                } else open_dialog(g,empty_ball,1,YPendingNone);
                return;
            }
            if(n->map==YMapViridianForest&&n->x==30&&n->y==33) {
                open_dialog(g,forest_youngster2_challenge,2,YPendingNone);
                return;
            }
            if(n->map==YMapOaksLab&&g->story==YStoryBeforeOak&&n->type==YActorOak)
                open_dialog(g,early_lab,1,YPendingNone);
            else dialog_id(g,n->dialog_id);
            return;
        }
    }
    if(g->map==YMapViridianForest) {
        /* Background sign events, in tile coordinates, from the pinned map.
           They never consume items, start phantom trainer battles or set flags. */
        static const struct {uint8_t x,y;const char* const* lines;uint8_t pages;} signs[]={
            {24,40,forest_tips1,2},{16,32,forest_antidote_sign,2},
            {26,17,forest_tips2,2},{4,24,forest_tips3,2},
            {18,45,forest_tips4,2},{2,1,forest_leave_sign,1}
        };
        for(unsigned i=0;i<sizeof(signs)/sizeof(signs[0]);++i) {
            if(x==signs[i].x&&y==signs[i].y) {
                open_dialog(g,signs[i].lines,signs[i].pages,YPendingNone);
                return;
            }
        }
    }
    /* Unlike the old local Route2 placeholder item dialog, the original
       source sign records can now trigger genuine, sourced map text. Do not
       turn the nearby unported cave or Mart warps into fake destinations. */
    if(g->map==YMapRoute2) {
        YellowEventIR event;
        if(yellow_ir_at_position(YMapRoute2,x,y,YEIR_BG,&event)) {
            if(x==5&&y==65) {
                open_dialog(g,source_route2_sign,2,YPendingNone);return;
            }
            if(x==11&&y==11) {
                open_dialog(g,source_route2_cave_sign,1,YPendingNone);return;
            }
        }
    }
    if(g->map==YMapPallet) {
        if(x==13&&y==13){dialog_id(g,TalkLabSign);return;}
        if(x==7&&y==9){dialog_id(g,TalkTownSign);return;}
        if(x==3&&y==5){dialog_id(g,TalkHomeSign);return;}
        if(x==11&&y==5){dialog_id(g,TalkRivalSign);return;}
    }
    if(g->map==YMapRedsHouse&&x==3&&y==1)dialog_id(g,TalkTV);
    if(g->map==YMapRoute1&&x==9&&y==27)dialog_id(g,TalkRouteSign);
    if(g->map==YMapViridian) {
        if(x==17&&y==17){dialog_id(g,TalkViridianSign);return;}
        if(x==24&&y==25){dialog_id(g,TalkViridianSign);return;}
    }
}
/* Opponent move selection: temporary simple Gen1-inspired policy.
   Complete Yellow trainer AI and four move slot RNG are not ported yet. */
static uint8_t choose_enemy_move(YellowGame* g) {
    uint32_t choice=next_rng(g);
    if(g->foe_kind==YK_PIDGEY) {
        if(g->foe_level>=12u && (choice&3u)==0u)return YM_QUICK_ATTACK;
        return g->foe_level>=5u && (choice&3u)==1u?YM_SAND_ATTACK:YM_GUST;
    }
    if(g->foe_kind==YK_CATERPIE)
        return (choice&1u)?YM_STRING_SHOT:YM_TACKLE;
    if(g->foe_kind==YK_METAPOD)return YM_HARDEN;
    if(g->foe_kind==YK_PIDGEOTTO)
        return (choice&3u)==1u?YM_SAND_ATTACK:YM_GUST;
    if(g->foe_kind==YK_NIDORAN_F)
        return (choice&3u)==0u?YM_GROWL:YM_TACKLE;
    if(g->foe_kind==YK_NIDORAN_M)
        return (choice&3u)==0u?YM_LEER:YM_TACKLE;
    if(g->foe_kind==YK_CLEFAIRY)return (choice&3u)==0u?YM_GROWL:YM_POUND;
    if(g->foe_kind==YK_ZUBAT)
        return g->foe_level>=10u && (choice&3u)==0u?YM_SUPERSONIC:YM_LEECH_LIFE;
    if(g->foe_kind==YK_GEODUDE)return YM_TACKLE;
    /* Yellow Brock Onix Lv12: Tackle / Screech (base_stats/onix.asm). */
    if(g->foe_kind==YK_ONIX)return (choice&1u)?YM_SCREECH:YM_TACKLE;
    if(g->foe_kind==YK_SANDSHREW)
        return g->foe_level>=10u && (choice&3u)==0u?
               YM_SAND_ATTACK:YM_SCRATCH;
    if(g->foe_kind==YK_RATTATA) {
        if(g->foe_level>=14u && (choice&7u)==2u)return YM_HYPER_FANG;
        if(g->foe_level>=7u && (choice&3u)==0u)return YM_QUICK_ATTACK;
        return (choice&7u)==1u?YM_TAIL_WHIP:YM_TACKLE;
    }
    /* For species without a dedicated AI policy, use their actual learned
       moves rather than fabricating Tackle/Tail Whip (e.g. Diglett). */
    uint8_t moves[4];
    y_build_moveset(g->foe_kind,g->foe_level,moves);
    uint8_t count=0;
    while(count<4u && moves[count]!=YM_NONE)count++;
    if(count)return moves[choice%count];
    return YM_TACKLE; /* legacy unsupported-species fallback */
}
/* Shared action pipeline for either combatant.  Field ordering:
 * choose move -> (player PP) -> full paralysis -> accuracy ->
 * critical/damage -> primary effect -> on-hit secondary effect ->
 * faint or forced replacement.  This is a partial Gen1 rule engine;
 * status self-buffs and exact PRNG consumption still need source parity.
 */
static void shift_stage(int8_t* stage,int8_t by) {
    int v=(int)*stage+(int)by;
    if(v>6)v=6;
    if(v< -6)v=-6;
    *stage=(int8_t)v;
}
/* Gen1 SleepEffect: repeat BattleRandom & SLP_MASK until nonzero.
 * The port's xorshift PRNG is not Game Boy BattleRandom bit-for-bit.
 */
static uint8_t roll_sleep_status(YellowGame* g) {
    uint8_t n;
    do { n=(uint8_t)next_rng(g)&7u; } while(n==0u);
    return (uint8_t)(YStatusSleep1+n-1u);
}
/* Gen1 status move type checks from pinned pret/pokeyellow:
 * engine/battle/effects.asm PoisonEffect excludes either POISON type;
 * engine/battle/move_effects/paralyze.asm ParalyzeEffect_ excludes either
 * GROUND type only when the attacking move is ELECTRIC (Thunder Wave).
 * These tests happen before MoveHitTest, so no accuracy RNG is consumed. */
static bool status_move_type_immune(const YellowGame* g,bool player,uint8_t move) {
    if(move!=YM_POISONPOWDER && move!=YM_TOXIC && move!=YM_THUNDER_WAVE)return false;
    uint8_t target=player?g->foe_kind:yellow_party_species(g,g->active_party);
    const YSpeciesInfo* info=y_species_info(target);
    if(!info)return true; /* invalid target is not statusable */
    if(move==YM_POISONPOWDER || move==YM_TOXIC)
        return info->type1==YT_POISON || info->type2==YT_POISON;
    if(move==YM_THUNDER_WAVE)
        return info->type1==YT_GROUND || info->type2==YT_GROUND;
    return false; /* Stun Spore is Grass, so Ground does not block it. */
}
static void apply_move_effect(YellowGame* g,bool player,uint8_t move,
                              uint8_t damage,bool critical) {
    int8_t* target_attack=player?&g->foe_attack_stage:&g->pika_attack_stage;
    int8_t* target_defense=player?&g->foe_defense_stage:&g->pika_defense_stage;
    int8_t* target_accuracy=player?&g->foe_accuracy_stage:&g->player_accuracy_stage;
    int8_t* source_evasion=player?&g->player_evasion_stage:&g->foe_evasion_stage;
    int8_t* target_speed=player?&g->foe_speed_stage:&g->pika_speed_stage;
    int8_t* source_defense=player?&g->pika_defense_stage:&g->foe_defense_stage;
    switch(move) {
    case YM_GROWL:shift_stage(target_attack,-1);return;
    case YM_TAIL_WHIP:case YM_LEER:shift_stage(target_defense,-1);return;
    case YM_SCREECH:shift_stage(target_defense,-2);return;
    case YM_SAND_ATTACK:shift_stage(target_accuracy,-1);return;
    case YM_STRING_SHOT:shift_stage(target_speed,-1);return;
    case YM_HARDEN:shift_stage(source_defense,+1);return;
    case YM_DOUBLE_TEAM:shift_stage(source_evasion,+1);return;
    case YM_SUPERSONIC:
        if(player) {
            if(g->foe_confusion_turns==0)
                g->foe_confusion_turns=(uint8_t)(2u+(next_rng(g)&3u));
        } else if(g->party_confusion_turns[g->active_party]==0) {
            g->party_confusion_turns[g->active_party]=(uint8_t)(2u+(next_rng(g)&3u));
        }
        return;
    case YM_THUNDER_WAVE:
        if(player) {
            if(g->foe_status==YStatusNone)g->foe_status=YStatusParalyzed;
        } else if(yellow_party_status(g,g->active_party)==YStatusNone) {
            if(g->active_party==0)g->pika_status=YStatusParalyzed;
            else g->caught[g->active_party-1u].status=YStatusParalyzed;
        }
        return;
    case YM_TOXIC: {
        /* Pinned pret/pokeyellow engine/battle/effects.asm PoisonEffect:
         * status already occupied or either target type Poison => no effect;
         * move #92 TOXIC alone sets BADLY_POISONED. Accuracy was tested
         * once by battle_action_pipeline before this primary effect. */
        uint8_t target_kind=player?g->foe_kind:
            yellow_party_species(g,g->active_party);
        const YSpeciesInfo* target=y_species_info(target_kind);
        if(!target || target->type1==YT_POISON || target->type2==YT_POISON)
            return;
        if(player) {
            if(g->foe_status==YStatusNone)g->foe_status=YStatusToxic1;
        } else if(yellow_party_status(g,g->active_party)==YStatusNone) {
            set_party_status(g,g->active_party,YStatusToxic1);
        }
        return;
    }
    case YM_POISONPOWDER:
        if(player) {
            if(g->foe_status==YStatusNone)g->foe_status=YStatusPoisoned;
        } else if(yellow_party_status(g,g->active_party)==YStatusNone) {
            if(g->active_party==0)g->pika_status=YStatusPoisoned;
            else g->caught[g->active_party-1u].status=YStatusPoisoned;
        }
        return;
    case YM_SLEEP_POWDER:
        if(player) {
            if(g->foe_status==YStatusNone)
                g->foe_status=roll_sleep_status(g);
        } else if(yellow_party_status(g,g->active_party)==YStatusNone) {
            set_party_status(g,g->active_party,roll_sleep_status(g));
        }
        return;
    case YM_STUN_SPORE:
        /* Single accuracy roll handled by battle_action_pipeline(). */
        if(player) {
            /* Check for existing status */
            if(g->foe_status==YStatusNone) {
                g->foe_status=YStatusParalyzed;
            }
        } else {
            /* Check for existing status */
            if(yellow_party_status(g,g->active_party)==YStatusNone) {
                if(g->active_party==0)g->pika_status=YStatusParalyzed;
                else g->caught[g->active_party-1u].status=YStatusParalyzed;
            }
        }
        return;
    default:break;
    }
    if(!damage)return; /* no effects after an immunity/zero damage */
    if(critical)g->last_critical=1;
    if(player) {
        if(move==YM_LEECH_LIFE) {
            uint8_t applied=damage>g->foe_hp?g->foe_hp:damage;
            uint8_t heal=(uint8_t)(applied/2u);
            uint8_t hp=yellow_party_hp(g,g->active_party);
            uint8_t max=yellow_party_max_hp(g,g->active_party);
            if(heal>max-hp)heal=(uint8_t)(max-hp);
            set_party_hp(g,g->active_party,(uint8_t)(hp+heal));
        }
        if(g->foe_hp<=damage) {
            finalize_battle(g,true);
            return;
        }
        g->foe_hp-=damage;
        if((move==YM_THUNDERSHOCK || move==YM_THUNDERBOLT) &&
           y_thundershock_paralyzes((uint8_t)next_rng(g),g->foe_status))
            g->foe_status=YStatusParalyzed;
        /* pret/pokeyellow effects.asm CheckDefrost: FIRE burn-side-effect
           moves (Ember, unlike Fire Spin) always clear freeze upon a hit.
           The status is cleared even when the burn chance would fail. */
        if(move==YM_EMBER) {
            if(g->foe_status==YStatusFrozen)g->foe_status=YStatusNone;
            else if(g->foe_status==YStatusNone && (uint8_t)next_rng(g)<26u)
                g->foe_status=YStatusBurned;
        }
        if(move==YM_HYPER_FANG && (uint8_t)next_rng(g)<26u)
            g->foe_flinched=1;
    } else {
        uint8_t hp=yellow_party_hp(g,g->active_party);
        if(move==YM_LEECH_LIFE) {
            uint8_t heal=(uint8_t)((damage>hp?hp:damage)/2u);
            uint8_t free_hp=(uint8_t)(g->foe_max_hp-g->foe_hp);
            if(heal>free_hp)heal=free_hp;
            g->foe_hp=(uint8_t)(g->foe_hp+heal);
        }
        if(hp<=damage) {
            set_party_hp(g,g->active_party,0);
            uint8_t next=first_healthy_party(g);
            if(next==255) {
                finalize_battle(g,false);
                return;
            }
            g->screen=YScreenParty;
            g->party_battle=1;g->force_switch=1;g->selected=next;
        } else {
            set_party_hp(g,g->active_party,(uint8_t)(hp-damage));
            if(move==YM_EMBER) {
                uint8_t target_status=yellow_party_status(g,g->active_party);
                if(target_status==YStatusFrozen)
                    set_party_status(g,g->active_party,YStatusNone);
                else if(target_status==YStatusNone && (uint8_t)next_rng(g)<26u)
                    set_party_status(g,g->active_party,YStatusBurned);
            }
        }
    }
}
static void battle_action_pipeline(YellowGame* g,bool player,uint8_t move,
                                   bool paralysis_check) {
    g->screen=YScreenRivalBattle;
    uint8_t active=g->active_party;
    uint8_t status=player?yellow_party_status(g,active):g->foe_status;
    if(y_status_is_sleep(status)) {
        uint8_t next=y_sleep_after_action(status);
        if(player)set_party_status(g,active,next);
        else g->foe_status=next;
        return;
    }
    if(status==YStatusFrozen)return;
    if(player) {
        /* The pending move is spent only after the battler's action starts. */
        (*pp_location(g,active,g->selected))--;
    } else g->last_enemy_move=move;
    if(paralysis_check &&
       (player?yellow_party_status(g,active):g->foe_status)==YStatusParalyzed &&
       y_full_paralysis((uint8_t)next_rng(g))) {
        g->screen=YScreenRivalBattle;return;
    }
    uint8_t* confused=player?&g->party_confusion_turns[active]:&g->foe_confusion_turns;
    if(*confused) {
        --*confused;
        if((next_rng(g)&1u)==0) {
            uint8_t source=player?yellow_party_species(g,active):g->foe_kind;
            uint8_t level=player?yellow_party_level(g,active):g->foe_level;
            YBattleStats own=y_calc_stats(source,level);
            unsigned def=own.defense?own.defense:1;
            unsigned damage=((2u*level/5u+2u)*40u*own.attack/def/50u)+2u;
            if(!damage)damage=1;
            if(player) {
                uint8_t hp=yellow_party_hp(g,active);
                set_party_hp(g,active,hp>damage?(uint8_t)(hp-damage):0);
                if(hp<=damage) {
                    uint8_t next=first_healthy_party(g);
                    if(next==255)finalize_battle(g,false);
                    else {g->screen=YScreenParty;g->party_battle=1;
                          g->force_switch=1;g->selected=next;}
                }
            } else {
                if(g->foe_hp<=damage)finalize_battle(g,true);
                else g->foe_hp=(uint8_t)(g->foe_hp-damage);
            }
            return;
        }
    }
    /* SleepEffect checks occupied status before MoveHitTest. Accuracy is
     * rolled only for an eligible target; duration only on a successful hit.
     * Non-damaging Sleep Powder must not draw critical/damage RNG bytes.
     */
    if(move==YM_SLEEP_POWDER) {
        uint8_t target_status=player?g->foe_status:yellow_party_status(g,active);
        if(target_status!=YStatusNone)return;
        int8_t acc=player?g->player_accuracy_stage:g->foe_accuracy_stage;
        int8_t evade=player?g->foe_evasion_stage:g->player_evasion_stage;
        if(!y_move_hits(move,acc,evade,(uint8_t)next_rng(g)))return;
        apply_move_effect(g,player,move,0,false);
        return;
    }
    /* Gen1 PoisonEffect/ParalyzeEffect_: reject occupied status first,
     * then type immunity, then exactly one MoveHitTest.  No critical/damage
     * RNG is drawn by a non-damaging powder/wave.  Sleep has its own path. */
    if(move==YM_TOXIC) {
        uint8_t target_status=player?g->foe_status:yellow_party_status(g,active);
        if(target_status!=YStatusNone || status_move_type_immune(g,player,move))return;
        int8_t acc=player?g->player_accuracy_stage:g->foe_accuracy_stage;
        int8_t evade=player?g->foe_evasion_stage:g->player_evasion_stage;
        if(!y_move_hits(move,acc,evade,(uint8_t)next_rng(g)))return;
        apply_move_effect(g,player,move,0,false);
        return;
    }
    if(move==YM_POISONPOWDER || move==YM_THUNDER_WAVE || move==YM_STUN_SPORE) {
        uint8_t target_status=player?g->foe_status:yellow_party_status(g,active);
        if(target_status!=YStatusNone || status_move_type_immune(g,player,move))return;
        int8_t acc=player?g->player_accuracy_stage:g->foe_accuracy_stage;
        int8_t evade=player?g->foe_evasion_stage:g->player_evasion_stage;
        if(!y_move_hits(move,acc,evade,(uint8_t)next_rng(g)))return;
        apply_move_effect(g,player,move,0,false);
        return;
    }
    int8_t accuracy=player?g->player_accuracy_stage:g->foe_accuracy_stage;
    int8_t evasion=player?g->foe_evasion_stage:g->player_evasion_stage;
    if(!y_move_hits(move,accuracy,evasion,(uint8_t)next_rng(g))) {
        g->screen=YScreenRivalBattle;return;
    }
    const uint8_t source=player?yellow_party_species(g,active):g->foe_kind;
    const uint8_t source_level=player?yellow_party_level(g,active):g->foe_level;
    const uint8_t target=player?g->foe_kind:yellow_party_species(g,active);
    const uint8_t target_level=player?g->foe_level:yellow_party_level(g,active);
    const int8_t atk_stage=player?g->pika_attack_stage:g->foe_attack_stage;
    const int8_t def_stage=player?g->foe_defense_stage:g->pika_defense_stage;
    /* Maintain existing deterministic test RNG sequence: status moves draw
       critical/damage bytes too.  This will be reconciled with original PRNG. */
    bool critical=y_roll_critical(source,move,(uint8_t)next_rng(g));
    YDamage d=y_calc_damage_status_crit(source,source_level,target,target_level,move,
                                 atk_stage,def_stage,0,(uint8_t)next_rng(g),
                                 critical,player?yellow_party_status(g,active):
                                                g->foe_status);
    apply_move_effect(g,player,move,d.damage,critical);
}
/* Wrappers kept for the previous test suite and non-attack turn actions. */
static void execute_enemy_action(YellowGame* g,uint8_t move,bool check_paralysis) {
    battle_action_pipeline(g,false,move,check_paralysis);
}
static bool apply_residual_after_action(YellowGame* g,bool player) {
    uint8_t status=player?yellow_party_status(g,g->active_party):g->foe_status;
    uint8_t hp=player?yellow_party_hp(g,g->active_party):g->foe_hp;
    uint8_t max_hp=player?yellow_party_max_hp(g,g->active_party):g->foe_max_hp;
    uint8_t damage=y_residual_damage(max_hp,status);
    if(!damage || !hp)return g->screen==YScreenRivalBattle;
    if(y_status_is_toxic(status)) {
        uint8_t updated=y_toxic_after_tick(status);
        if(player)set_party_status(g,g->active_party,updated);
        else g->foe_status=updated;
    }
    if(!player) {
        if(hp<=damage) {
            finalize_battle(g,true);
            return false;
        }
        g->foe_hp=(uint8_t)(hp-damage);
        return true;
    }
    if(hp<=damage) {
        set_party_hp(g,g->active_party,0);
        uint8_t next=first_healthy_party(g);
        if(next==255) {
            finalize_battle(g,false);
            return false;
        }
        g->screen=YScreenParty;
        g->party_battle=1;g->force_switch=1;g->selected=next;
        return false;
    }
    set_party_hp(g,g->active_party,(uint8_t)(hp-damage));
    return true;
}
static void enemy_turn(YellowGame* g) {
    /* The shared pipeline owns the paralysis check.  Keeping a separate
       pre-check here changes RNG ordering and duplicates status logic. */
    execute_enemy_action(g,choose_enemy_move(g),true);
    if(g->screen==YScreenRivalBattle)(void)apply_residual_after_action(g,false);
}
static void execute_player_action(YellowGame* g,uint8_t move) {
    battle_action_pipeline(g,true,move,true);
}
static void resolve_attack_turn(YellowGame* g,uint8_t player_move) {
    uint8_t enemy_move=choose_enemy_move(g);
    g->last_enemy_move=enemy_move;g->last_critical=0;
    g->foe_flinched=0;
    uint16_t ps=y_effective_speed(yellow_party_species(g,g->active_party),
                    yellow_party_level(g,g->active_party),
                    yellow_party_status(g,g->active_party));
    uint16_t es=y_effective_speed(g->foe_kind,g->foe_level,g->foe_status);
    /* STRING SHOT affects turn order; these stages are not persisted. */
    if(g->pika_speed_stage>0)ps=(uint16_t)(ps*(2+g->pika_speed_stage)/2);
    else if(g->pika_speed_stage<0)ps=(uint16_t)(ps*2/(2-g->pika_speed_stage));
    if(g->foe_speed_stage>0)es=(uint16_t)(es*(2+g->foe_speed_stage)/2);
    else if(g->foe_speed_stage<0)es=(uint16_t)(es*2/(2-g->foe_speed_stage));
    bool player_first=y_player_moves_first(player_move,enemy_move,ps,es,
                                          (uint8_t)next_rng(g));
    g->last_player_first=(uint8_t)player_first;
    g->screen=YScreenRivalBattle; /* leave action selector before executing */
    if(!player_first) {
        execute_enemy_action(g,enemy_move,true);
        if(g->screen!=YScreenRivalBattle)return;
        if(!apply_residual_after_action(g,false))return;
        /* Enemy KO/forced replacement interrupts pending player's action.
           The cancelled move doesn't spend any PP. */
    }
    execute_player_action(g,player_move);
    if(g->screen!=YScreenRivalBattle)return;
    if(!apply_residual_after_action(g,true))return;
    if(player_first) {
        if(!g->foe_flinched)execute_enemy_action(g,enemy_move,true);
        if(g->screen==YScreenRivalBattle)
            (void)apply_residual_after_action(g,false);
    }
}
static bool yellow_pc_box_valid(const YellowPCBox* box);
static bool yellow_pc_mon_valid(const YCapturedMon* mon);
static void battle_move(YellowGame* g) {
    uint8_t move_count=yellow_active_move_count(g);
    if(g->selected==move_count) {
        /* Poké Balls only work against wild Pokémon and if storage has room.
           NPC party/box transfer and trainer theft do not exist here. */
        if(g->forest_active_trainer<5 || g->pewter_active_trainer!=0 || g->route3_active_trainer!=0 || g->foe_kind==YK_EEVEE || !g->balls ||
           (yellow_party_size(g)>=YELLOW_PARTY_MAX &&
            (g->pc.count>=YELLOW_PC_CAPACITY || !yellow_pc_box_valid(&g->pc))))return;
        g->balls--;
        uint8_t r1=(uint8_t)next_rng(g);
        uint8_t r2=(uint8_t)next_rng(g);
        if(y_gen1_pokeball_capture_status(y_species_catch_rate(g->foe_kind),
                                  g->foe_max_hp,g->foe_hp,r1,r2,g->foe_status)) {
            bool to_pc=yellow_party_size(g)>=YELLOW_PARTY_MAX;
            YCapturedMon* mon=to_pc?&g->pc.slots[g->pc.count]:
                &g->caught[g->caught_count];
            memset(mon,0,sizeof(*mon));
            mon->species=g->foe_kind;
            mon->level=g->foe_level;
            mon->hp=g->foe_hp;
            mon->experience=y_exp_threshold(mon->species,mon->level);
            mon->status=y_status_is_toxic(g->foe_status)?
                YStatusPoisoned:g->foe_status;
            y_build_moveset(mon->species,mon->level,mon->moves);
            for(unsigned i=0;i<YELLOW_MOVE_SLOTS;i++)
                mon->pp[i]=y_max_pp(mon->moves[i]);
            /* Capture is committed only after a valid destination was selected. */
            if(to_pc)g->pc.count++;else g->caught_count++;
            (void)yellow_dex_mark_caught(&g->dex,yellow_dex_number(mon->species));
            g->foe_hp=0;
            open_dialog(g,caught_text,1,YPendingEndWildBattle);
        } else enemy_turn(g);
        return;
    }
    if(g->selected==move_count+2u) {
        if(yellow_party_size(g)>1){
            g->party_battle=1;g->force_switch=0;
            g->selected=g->active_party;g->screen=YScreenParty;
        }
        return;
    }
    if(g->selected==move_count+1u) {
        /* Rival trainer battles cannot be escaped. */
        if(g->forest_active_trainer<5 || g->pewter_active_trainer!=0 || g->route3_active_trainer!=0) {g->screen=YScreenRivalBattle;return;}
        if(g->foe_kind!=YK_EEVEE&&(next_rng(g)&3u)!=0u) {
            g->foe_hp=0;
            open_dialog(g,wild_flee,1,YPendingEndWildBattle);
        } else if(g->foe_kind!=YK_EEVEE)enemy_turn(g);
        else g->screen=YScreenRivalBattle;
        return;
    }
    if(g->selected>=move_count || g->selected>=YELLOW_MOVE_SLOTS ||
       !yellow_party_pp(g,g->active_party,g->selected))return;
    uint8_t move=yellow_party_move(g,g->active_party,g->selected);
    resolve_attack_turn(g,move);
}
/* pret/pokeyellow engine/events/poison.asm: ApplyOutOfBattlePoisonDamage.
 * Damage is one HP per poisoned living party member every four successful
 * world steps, including source-backed map transitions and ledge traversal.
 * Poison cannot advance on blocked movement, menus, dialogue or battle turns.
 * Toxic battle stage is never incremented by overworld poison.
 * Gen1 blackout/whiteout recovery routing is NOT implemented here: the
 * source-backed dialogue and volatile blackout signal do not imply a heal,
 * destination, or a finished whiteout implementation.
 */
static void yellow_overworld_poison_on_steps(YellowGame* g,uint32_t before) {
    /* YellowGame.steps is uint16_t; preserve the Gen1 4-step cadence
     * when a native save counter wraps from 65535 to 0. Every accepted
     * do_step is exactly 1 step, or 2 for an original Route9 ledge hop.
     */
    uint16_t delta=(uint16_t)(g->steps-(uint16_t)before);
    if(delta==0u || delta>2u)return;
    uint32_t ticks=((before&3u)+(uint32_t)delta)/4u;
    while(ticks--) {
        for(uint8_t i=0;i<yellow_party_size(g);i++) {
            uint8_t status=yellow_party_status(g,i);
            if(status!=YStatusPoisoned && !y_status_is_toxic(status))continue;
            uint8_t hp=yellow_party_hp(g,i);
            if(!hp)continue;
            set_party_hp(g,i,(uint8_t)(hp-1u));
            if(hp==1u)set_party_status(g,i,YStatusNone);
        }
        /* Original poison.asm tests AnyPartyAlive after applying damage and
         * emits TEXT_BLACKED_OUT with wOutOfBattleBlackout. Do not fake the
         * unported whiteout/last heal center, money loss, or teleport.
         * A pending flag prevents duplicate dialogues before recovery exists.
         */
        if(g->has_pikachu && !g->overworld_blackout_pending &&
           first_healthy_party(g)==255u) {
            g->overworld_blackout_pending=1u;
            open_dialog(g,poison_blackout_text,1u,YPendingNone);
        }
    }
}

void yellow_input(YellowGame* g,YellowKey key) {
    if(!g->running)return;
    switch(g->screen) {
    case YScreenWorld:
        if(key<=YRight) {
            uint32_t before=g->steps;
            do_step(g,key);
            yellow_overworld_poison_on_steps(g,before);
        }
        else if(key==YOk)interact(g);
        else if(key==YBack){g->screen=YScreenMenu;g->selected=0;}
        break;
    case YScreenNurseChoice:
        /* Gen1 pokecenter.asm: SetLastBlackoutMap only after Yes. No and
         * Back cannot heal or mutate the original last-map anchor. */
        if(key==YUp || key==YLeft)g->selected=0;
        else if(key==YDown || key==YRight)g->selected=1;
        else if(key==YBack)g->screen=YScreenWorld;
        else if(key==YOk) {
            if(g->selected==0) {
                yellow_heal_pikachu(g);
                if(g->map==YMapViridianCenter && !g->last_blackout_integrity_error)
                    g->last_blackout_original_map=1u;
                dialog_id(g,TalkViridianCenter);
            } else g->screen=YScreenWorld;
        }
        break;
    case YScreenDialog:
        /* Unlike legacy dialogues, an explicit shop confirmation must
         * never treat BACK as acceptance (or charge the player). */
        if(key==YBack && g->pending==YPendingBuyAntidote) {
            g->pending=YPendingNone;g->screen=YScreenWorld;
            g->dialog_page=0;g->dialog_count=0;break;
        }
        if(key==YOk||key==YBack) {
            if(++g->dialog_page>=g->dialog_count)resolve_pending(g);
        }
        break;
    case YScreenOakEncounter:
        if(key==YOk)open_dialog(g,capture_text,3,YPendingEnterLab);
        break;
    case YScreenRivalBattle:
        if(key==YOk){g->selected=0;g->screen=YScreenFightMenu;}
        break;
    case YScreenFightMenu:
        if(key==YUp&&g->selected)g->selected--;
        else if(key==YDown&&g->selected<yellow_active_move_count(g)+2u)g->selected++;
        else if(key==YBack)g->screen=YScreenRivalBattle;
        else if(key==YOk)battle_move(g);
        break;
    case YScreenMenu:
        if(key==YRight && g->has_pikachu && g->map==YMapViridianCenter) {
            g->screen=YScreenPokedex;g->selected=1;
            break;
        }
        if(key==YUp&&g->selected)g->selected--;
        else if(key==YDown && (g->selected<4 ||
                 (g->selected==4 && g->map==YMapViridianCenter)))g->selected++;
        else if(key==YBack)g->screen=YScreenWorld;
        else if(key==YOk) {
            if(g->selected==0){g->screen=YScreenParty;g->selected=0;}
            else if(g->selected==1){g->screen=YScreenBag;g->selected=0;g->bag_notice=YBagNoNotice;g->bag_last_heal=0;}
            else if(g->selected==2){g->request_save=true;g->message_saved=false;}
            else if(g->selected==3)g->running=false;
            else if(g->selected==4){g->screen=YScreenLinkOffline;g->link_result=0;}
            else if(g->selected==5 && g->map==YMapViridianCenter) {
                g->screen=YScreenPC;g->pc_mode=0;g->selected=0;
            }

        }
        break;
    case YScreenPokedex:
        if(key==YBack) {
            g->screen=YScreenMenu;g->selected=0;
        } else if(key==YUp && g->selected>1)g->selected--;
        else if(key==YDown && g->selected<YELLOW_DEX_SPECIES)g->selected++;
        else if(key==YLeft) {
            g->selected=g->selected>10?g->selected-10u:1;
        } else if(key==YRight) {
            g->selected=g->selected<YELLOW_DEX_SPECIES-10u?
                g->selected+10u:YELLOW_DEX_SPECIES;
        }
        break;
    case YScreenPC:
        if(g->map!=YMapViridianCenter) {
            g->screen=YScreenMenu;g->pc_mode=0;g->selected=5;
            break;
        }
        if(g->pc_mode==0) {
            if(key==YUp && g->selected)g->selected--;
            else if(key==YDown && g->selected<2)g->selected++;
            else if(key==YBack || (key==YOk && g->selected==2)){
                g->screen=YScreenMenu;g->selected=5;g->pc_mode=0;
            } else if(key==YOk) {
                g->pc_mode=(uint8_t)(g->selected+1u);
                g->selected=0;
            }
        } else if(g->pc_mode==1 || g->pc_mode==2) {
            uint8_t options=g->pc_mode==1?g->caught_count:g->pc.count;
            if(key==YBack){g->pc_mode=0;g->selected=0;}
            else if(key==YUp && g->selected)g->selected--;
            else if(key==YDown && options && g->selected+1u<options)g->selected++;
            else if(key==YOk && options && g->selected<options) {
                bool done=g->pc_mode==1?
                    yellow_pc_deposit(g,(uint8_t)(g->selected+1u)):
                    yellow_pc_withdraw(g,g->selected);
                if(done && g->selected>0)g->selected--;
            }
        } else {g->pc_mode=0;g->selected=0;}
        break;
    case YScreenParty:
        if(key==YUp&&g->selected)g->selected--;
        else if(key==YDown&&g->selected+1u<yellow_party_size(g))g->selected++;
        else if(key==YOk&&g->party_battle&&yellow_can_switch(g,g->selected)) {
            uint8_t forced=g->force_switch;
            if(y_status_is_toxic(yellow_party_status(g,g->active_party)))
                set_party_status(g,g->active_party,YStatusPoisoned);
            g->active_party=g->selected;
            g->participants_mask|=(uint8_t)(1u<<g->active_party);
            g->party_battle=0;g->force_switch=0;
            reset_player_stages(g);
            g->selected=0;
            if(forced)g->screen=YScreenRivalBattle;
            else enemy_turn(g); /* voluntary switch consumes the turn */
        } else if(key==YOk&&!g->party_battle&&g->selected<yellow_party_size(g)&&
                  yellow_party_hp(g,g->selected)>0) {
            g->active_party=g->selected; /* overworld default active monster */
        } else if(key==YBack&&!g->force_switch) {
            uint8_t battle=g->party_battle;
            g->party_battle=0;
            g->screen=battle?YScreenRivalBattle:YScreenMenu;
            g->selected=0;
        }
        break;
    case YScreenBag:
        if(key==YBack){
            g->screen=YScreenMenu;g->selected=1;
            g->bag_notice=YBagNoNotice;g->bag_last_heal=0;
        } else if(key==YUp || key==YDown) {
            if(key==YUp && g->selected)g->selected--;
            if(key==YDown && g->selected<1)g->selected++;
            g->bag_notice=YBagNoNotice;g->bag_last_heal=0;
        } else if(key==YOk && g->selected==0) {
            /* Report *why* a Potion works or fails. The same real party
               healing and stock rules remain authoritative. */
            uint8_t index=g->active_party;
            g->bag_last_heal=0;
            if(!g->has_pikachu || index>=yellow_party_size(g)) {
                g->bag_notice=YBagNoParty;
            } else if(!g->potions) {
                g->bag_notice=YBagNoPotions;
            } else {
                uint8_t hp=yellow_party_hp(g,index);
                uint8_t maximum=yellow_party_max_hp(g,index);
                if(!hp)g->bag_notice=YBagFainted;
                else if(hp>=maximum)g->bag_notice=YBagFullHP;
                else {
                    uint8_t amount=(uint8_t)((maximum-hp)>20?20:(maximum-hp));
                    set_party_hp(g,index,(uint8_t)(hp+amount));
                    --g->potions;
                    g->bag_last_heal=amount;
                    g->bag_notice=YBagHealed;
                }
            }
        }
        break;
    case YScreenLinkOffline:
        if(key==YBack) {
            g->screen=YScreenMenu;g->selected=4;
        } else if(key==YOk) {
            g->link_result=2;
            if(!g->has_pikachu ||
               g->active_party<yellow_party_size(g)) {
                YellowLinkSnapshot sent={
                    .sequence=(uint16_t)(g->link_sequence+1u),
                    .kind=g->has_pikachu?1u:0u,
                    .species=g->has_pikachu?yellow_party_species(g,g->active_party):0,
                    .level=g->has_pikachu?yellow_party_level(g,g->active_party):0,
                    .hp=g->has_pikachu?yellow_party_hp(g,g->active_party):0,
                    .status=g->has_pikachu?yellow_party_status(g,g->active_party):0
                };
                uint8_t packet[YELLOW_LINK_PACKET_SIZE];
                YellowLinkSnapshot received={0};
                if(yellow_link_encode(&sent,packet) &&
                   yellow_link_decode(packet,sizeof(packet),&received) &&
                   sent.sequence==received.sequence &&
                   sent.kind==received.kind &&
                   sent.species==received.species &&
                   sent.level==received.level &&
                   sent.hp==received.hp &&
                   sent.status==received.status) {
                    g->link_sequence=sent.sequence;
                    g->link_result=1;
                }
            }
        }
        break;
    case YScreenMoveForget:
        if(g->learn_index>=g->learn_count) {
            resolve_pending(g);
            break;
        }
        if(key==YUp&&g->selected)g->selected--;
        else if(key==YDown&&g->selected<YELLOW_MOVE_SLOTS)g->selected++;
        else if(key==YOk||key==YBack) {
            const uint8_t n=g->learn_index;
            uint8_t party=g->learn_party[n];
            uint8_t move=g->learn_moves[n];
            /* 0..3 replace selected slot; 4 or BACK skip learning. */
            if(key==YOk&&g->selected<YELLOW_MOVE_SLOTS &&
               party<yellow_party_size(g) && move!=YM_NONE) {
                uint8_t* moves=party==0?g->pika_moves:g->caught[party-1].moves;
                uint8_t* pp=party==0?g->pika_pp:g->caught[party-1].pp;
                bool known=false;
                for(uint8_t j=0;j<YELLOW_MOVE_SLOTS;j++)
                    if(moves[j]==move)known=true;
                if(!known) {
                    moves[g->selected]=move;
                    pp[g->selected]=y_max_pp(move);
                }
            }
            g->learn_index++;
            if(g->learn_index==g->learn_count)resolve_pending(g);
            else g->selected=0;
        }
        break;
    }
}
static bool yellow_valid_moves(uint8_t species,uint8_t level,
                               const uint8_t moves[4],const uint8_t pp[4]) {
    bool seen_empty=false;
    bool any=false;
    for(uint8_t i=0;i<YELLOW_MOVE_SLOTS;i++) {
        uint8_t id=moves[i];
        if(id==YM_NONE) {
            seen_empty=true;
            if(pp[i]!=0)return false;
            continue;
        }
        if(seen_empty || !y_move_available(species,level,id) ||
           pp[i]>y_max_pp(id))return false;
        for(uint8_t j=0;j<i;j++)if(moves[j]==id)return false;
        any=true;
    }
    return any;
}
#include "yellow_pc_box.c" /* source-bound box, v13 unchanged */
static uint32_t checksum(const YellowSave* s) {
    /* Explicit serialization-independent checksum, no uninitialized padding. */
    uint32_t c=0xA54C537Bu;
    c^=s->magic^((uint32_t)s->version<<16);
    c^=(uint32_t)s->map<<24 ^ (uint32_t)s->x<<16 ^ (uint32_t)s->y<<8 ^ s->facing;
    c^=(uint32_t)s->hp<<24 ^ (uint32_t)s->potions<<16 ^ (uint32_t)s->story<<8 ^ s->has_pikachu;
    c^=(uint32_t)s->affection<<24 ^ (uint32_t)s->foe_hp<<16 ^ s->steps;
    c^=(uint32_t)s->foe_kind<<24 ^ (uint32_t)s->foe_level<<16 ^
       (uint32_t)s->foe_max_hp<<8 ^ s->pika_level;
    c^=(uint32_t)s->follower_map<<24 ^ (uint32_t)s->follower_x<<16 ^
       (uint32_t)s->follower_y<<8 ^ s->follower_valid;
    c^=s->experience;
    for(unsigned i=0;i<YELLOW_MOVE_SLOTS;i++) {
        c=(c*16777619u)^s->pika_pp[i];
        c=(c*16777619u)^s->pika_moves[i];
    }
    c=(c*16777619u)^s->balls;
    c=(c*16777619u)^s->caught_count;
    c=(c*16777619u)^s->active_party;
    c=(c*16777619u)^s->pika_status;
    for(unsigned i=0;i<YELLOW_CAPTURE_SLOTS;i++) {
        const YCapturedMon* m=&s->caught[i];
        c=(c*16777619u)^m->species;
        c=(c*16777619u)^m->level;
        c=(c*16777619u)^m->hp;
        c=(c*16777619u)^m->status;
        for(unsigned j=0;j<YELLOW_MOVE_SLOTS;j++) {
            c=(c*16777619u)^m->pp[j];
            c=(c*16777619u)^m->moves[j];
        }
        c=(c*16777619u)^m->experience;
    }
    return c^s->rng;
}
/* Sidecar is tied to the actual v13 parent snapshot checksum, preventing
   cross-save defeat flags and preserving all legacy v13 bytes verbatim. */
static uint32_t forest_save_checksum(const YellowTrainerSave* s) {
    uint32_t c=2166136261u;
    c=(c^s->magic)*16777619u;
    c=(c^s->version)*16777619u;
    c=(c^s->defeated_mask)*16777619u;
    c=(c^s->parent_checksum)*16777619u;
    return c^0xFE143031u;
}
void yellow_trainer_save_create(const YellowGame* g,const YellowSave* parent,
                               YellowTrainerSave* out) {
    memset(out,0,sizeof(*out));
    out->magic=FOREST_TRAINER_SAVE_MAGIC;
    out->version=1;
    out->defeated_mask=g->forest_defeated_mask&31u;
    out->parent_checksum=parent->checksum;
    out->checksum=forest_save_checksum(out);
}
bool yellow_trainer_save_load(YellowGame* g,const YellowSave* parent,
                              const YellowTrainerSave* s) {
    if(s->magic!=FOREST_TRAINER_SAVE_MAGIC || s->version!=1 ||
       s->reserved!=0 || (s->defeated_mask&~31u)!=0 ||
       s->parent_checksum!=parent->checksum ||
       s->checksum!=forest_save_checksum(s))return false;
    g->forest_defeated_mask=s->defeated_mask;
    return true;
}

void yellow_save_create(const YellowGame* g,YellowSave* s) {
    memset(s,0,sizeof(*s));
    s->magic=YELLOW_SAVE_MAGIC;s->version=13;
    /* Native v13 format knows only the original 11 maps. On the new
       pre-badge City scene, store a valid Route2 gate checkpoint until the
       versioned exact-city position sidecar is implemented. Never write an
       unreadable native save with map==11; preserve old bytes/layout. */
    if(g->map==YMapPewterCity || g->map==YMapPewterGym ||
       g->map==YMapRoute3 || g->map==YMapRoute4 || g->map==YMapCeruleanCity || g->map==YMapRoute9 || g->map==YMapRoute10 || g->map==YMapRoute22 || g->map==YMapViridianMart || g->map==YMapCeruleanMart ||
       g->map==YELLOW_B1F_MAP_ID || g->map==YELLOW_B2F_MAP_ID ||
       yellow_mtmoon1f_is_active(g->map)) {
        s->map=YMapRoute2;s->x=9;s->y=0;
    } else {
        s->map=g->map;s->x=(uint8_t)g->x;s->y=(uint8_t)g->y;
    }
    s->facing=g->facing;s->hp=g->hp;s->potions=g->potions;
    s->story=(uint8_t)g->story;s->has_pikachu=g->has_pikachu;s->affection=g->affection;
    s->foe_hp=g->foe_hp;s->foe_kind=g->foe_kind;
    s->foe_level=g->foe_level;s->foe_max_hp=g->foe_max_hp;s->pika_level=g->pika_level;
    s->follower_map=g->follower_map;
    s->follower_x=g->follower_x;s->follower_y=g->follower_y;
    s->follower_valid=(g->map==YMapPewterCity||g->map==YMapPewterGym||g->map==YMapRoute3||g->map==YMapRoute4||g->map==YMapCeruleanCity||g->map==YMapRoute9||g->map==YMapRoute10||g->map==YMapRoute22||g->map==YMapViridianMart||g->map==YMapCeruleanMart||g->map==YELLOW_B1F_MAP_ID||g->map==YELLOW_B2F_MAP_ID||yellow_mtmoon1f_is_active(g->map))?0:g->follower_valid;
    if(g->map==YMapPewterCity||g->map==YMapPewterGym||g->map==YMapRoute3||g->map==YMapRoute4||g->map==YMapCeruleanCity||g->map==YMapRoute9||g->map==YMapRoute10||g->map==YMapRoute22||g->map==YMapViridianMart||g->map==YMapCeruleanMart||g->map==YELLOW_B1F_MAP_ID||g->map==YELLOW_B2F_MAP_ID||yellow_mtmoon1f_is_active(g->map)){s->follower_map=YMapRoute2;s->follower_x=9;s->follower_y=0;}
    s->steps=g->steps;s->rng=g->rng;
    s->experience=g->experience;
    for(unsigned i=0;i<YELLOW_MOVE_SLOTS;i++) {
        s->pika_pp[i]=g->pika_pp[i];
        s->pika_moves[i]=g->pika_moves[i];
    }
    s->balls=g->balls;
    s->caught_count=g->caught_count;
    s->active_party=g->active_party;
    s->pika_status=g->pika_status;
    for(unsigned i=0;i<g->caught_count&&i<YELLOW_CAPTURE_SLOTS;i++)
        s->caught[i]=g->caught[i];
    s->checksum=checksum(s);
}
static bool yellow_save_load_with_pickups(YellowGame* g,const YellowSave* s,uint8_t pickups) {
    if(s->magic!=YELLOW_SAVE_MAGIC||s->version!=13||
       s->map>=YELLOW_MAP_COUNT||s->x>=yellow_map_w(s->map)||
       s->y>=yellow_map_h(s->map)||s->facing>YRight||
       s->pika_level<5||s->pika_level>100||
       s->hp>y_calc_stats(YK_PIKACHU,s->pika_level).hp||
       (s->foe_kind==YK_PIKACHU||s->foe_kind==13||s->foe_kind>YK_CLEFAIRY)||s->foe_level>100||
       s->experience<y_exp_threshold(YK_PIKACHU,s->pika_level)||
       (s->pika_level<100&&s->experience>=y_exp_threshold(YK_PIKACHU,(uint8_t)(s->pika_level+1)))||
       s->balls>99||
       s->caught_count>YELLOW_CAPTURE_SLOTS||
       s->active_party> s->caught_count ||
       !y_status_valid(s->pika_status)||
       (s->active_party!=0&&!s->has_pikachu)||
       (!s->has_pikachu&&s->caught_count)||
       s->story>YStoryPikachuFollows||
       (s->story==YStoryPikachuFollows&&!s->has_pikachu)||
       (s->has_pikachu&&s->story<YStoryRivalBattle)||
       (s->follower_valid&&(s->follower_map>=YELLOW_MAP_COUNT||
           s->follower_x>=yellow_map_w(s->follower_map)||
           s->follower_y>=yellow_map_h(s->follower_map)))||
       s->checksum!=checksum(s))return false;
    if(!yellow_valid_moves(YK_PIKACHU,s->pika_level,
                           s->pika_moves,s->pika_pp))return false;
    for(unsigned i=0;i<s->caught_count;i++) {
        const YCapturedMon* m=&s->caught[i];
        if(!y_status_valid(m->status) ||
           (m->species!=YK_PIDGEY&&m->species!=YK_RATTATA&&
            m->species!=YK_CATERPIE&&m->species!=YK_METAPOD&&
            m->species!=YK_PIDGEOTTO&&m->species!=YK_GEODUDE&&
            m->species!=YK_SANDSHREW&&m->species!=YK_ZUBAT&&
            m->species!=YK_CLEFAIRY)||
           m->level<2||m->level>100||
           m->hp>y_calc_stats(m->species,m->level).hp||
           m->experience<y_exp_threshold(m->species,m->level)||
           (m->level<100&&m->experience>=y_exp_threshold(m->species,(uint8_t)(m->level+1)))||
           !yellow_valid_moves(m->species,m->level,m->moves,m->pp))return false;
    }
    YellowGame tmp={0};tmp.map=s->map;tmp.story=(YellowStory)s->story;
    tmp.forest_pickups=pickups;
    if(!yellow_walkable(&tmp,s->x,s->y))return false;

    yellow_init(g);
    g->forest_pickups=pickups;
    g->map=s->map;g->x=(int8_t)s->x;g->y=(int8_t)s->y;g->facing=s->facing;
    g->hp=s->hp;g->potions=s->potions;g->steps=s->steps;
    g->story=(YellowStory)s->story;g->has_pikachu=s->has_pikachu;
    g->affection=s->affection;g->foe_hp=s->foe_hp;g->rng=s->rng;
    g->foe_kind=s->foe_kind;g->foe_level=s->foe_level;
    g->foe_max_hp=s->foe_max_hp;g->pika_level=s->pika_level;
    g->experience=s->experience;
    for(unsigned i=0;i<YELLOW_MOVE_SLOTS;i++) {
        g->pika_pp[i]=s->pika_pp[i];
        g->pika_moves[i]=s->pika_moves[i];
    }
    g->balls=s->balls;
    g->caught_count=s->caught_count;
    for(unsigned i=0;i<g->caught_count;i++)g->caught[i]=s->caught[i];
    /* Recover the minimum true Pokédex knowledge from old version 13 saves
       without inventing species that were never observed. */
    if(g->has_pikachu)
        (void)yellow_dex_mark_caught(&g->dex,yellow_dex_number(YK_PIKACHU));
    for(unsigned i=0;i<g->caught_count;i++)
        (void)yellow_dex_mark_caught(&g->dex,yellow_dex_number(g->caught[i].species));
    g->active_party=s->active_party;
    g->pika_status=s->pika_status;
    if(yellow_party_hp(g,g->active_party)==0) {
        uint8_t next=first_healthy_party(g);
        if(next==255)return false;
        g->active_party=next;
    }
    g->screen=YScreenWorld;g->pending=YPendingNone;
    run_source_map_script(g); /* regenerate volatile script state from source IR */
    set_follower_near(g); /* rebuild follower, do not trust saved stale coordinates */
    return true;
}

bool yellow_save_load(YellowGame* g,const YellowSave* s) {
    return yellow_save_load_with_pickups(g,s,0);
}
static uint32_t forest_item_checksum(const YellowItemSave* s) {
    uint32_t h=2166136261u;
    h=(h^s->magic)*16777619u;
    h=(h^s->version)*16777619u;
    h=(h^s->collected_mask)*16777619u;
    h=(h^s->parent_checksum)*16777619u;
    return h^0x62143817u;
}
void yellow_item_save_create(const YellowGame* g,const YellowSave* parent,
                             YellowItemSave* out) {
    memset(out,0,sizeof(*out));
    out->magic=FOREST_ITEM_SAVE_MAGIC;out->version=1;
    out->collected_mask=g->forest_pickups&7u;
    out->parent_checksum=parent->checksum;
    out->checksum=forest_item_checksum(out);
}
static bool validate_item_save(const YellowSave* parent,const YellowItemSave* sidecar) {
    return parent && sidecar &&
       sidecar->magic==FOREST_ITEM_SAVE_MAGIC &&
       sidecar->version==1 && sidecar->reserved==0 &&
       !(sidecar->collected_mask&~7u) &&
       (!sidecar->collected_mask ||
          (parent->has_pikachu && parent->story==YStoryPikachuFollows)) &&
       sidecar->parent_checksum==parent->checksum &&
       sidecar->checksum==forest_item_checksum(sidecar);
}
bool yellow_item_save_load(YellowGame* g,const YellowSave* parent,
                            const YellowItemSave* sidecar) {
    if(!validate_item_save(parent,sidecar))return false;
    /* Validate the actual position against the collected-object state. */
    YellowGame rebuilt;
    if(!yellow_save_load_with_pickups(&rebuilt,parent,sidecar->collected_mask))
        return false;
    *g=rebuilt;
    return true;
}
bool yellow_save_load_with_items(YellowGame* g,const YellowSave* parent,
                                  const YellowItemSave* sidecar) {
    if(!sidecar)return yellow_save_load(g,parent);
    return yellow_item_save_load(g,parent,sidecar);
}

/* R10, R11, R12 save layouts are identical.  Reject corrupted or
 * mismatched previous versions before re-signing with the current
 * format version.  Never mutate the source file/struct. */
static bool migrate_saved_version(YellowGame* g,const YellowSave* original,
                                  uint16_t expected_version) {
    if(original->magic!=YELLOW_SAVE_MAGIC ||
       original->version!=expected_version ||
       original->checksum!=checksum(original) ||
       original->caught_count>YELLOW_CAPTURE_SLOTS ||
       original->pika_status>YStatusPoisoned)return false;
    for(unsigned i=0;i<original->caught_count;i++)
        if(original->caught[i].status>YStatusPoisoned)return false;
    YellowSave converted=*original;
    converted.version=13;
    converted.checksum=checksum(&converted);
    return yellow_save_load(g,&converted);
}
bool yellow_save_migrate_r10(YellowGame* g,const YellowSave* original) {
    return migrate_saved_version(g,original,10);
}
bool yellow_save_migrate_r11(YellowGame* g,const YellowSave* original) {
    return migrate_saved_version(g,original,11);
}

/* v12 and v13 use identical byte layouts. The new status range is only
 * valid in version 13; reject unexpected old codes before migration. */
bool yellow_save_migrate_r12(YellowGame* g,const YellowSave* old) {
    if(old->version!=12 || old->magic!=YELLOW_SAVE_MAGIC ||
       old->checksum!=checksum(old) || old->pika_status>YStatusPoisoned ||
       old->caught_count>YELLOW_CAPTURE_SLOTS)return false;
    for(unsigned i=0;i<old->caught_count;i++)
        if(old->caught[i].status>YStatusPoisoned)return false;
    return migrate_saved_version(g,old,12);
}

/* R0.14 original event-pair state prerequisite; no grant, inventory or story claims. */
#include "yellow_oak_parcel_sidecar.c"
#include "yellow_oak_parcel_slot.c" /* per-generation SD sidecar, v13 untouched */
