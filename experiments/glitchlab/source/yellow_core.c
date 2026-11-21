#include "yellow_core.h"
#include "yellow_assets.h"
#include <string.h>

/*
 * Native game logic; original Gen 1 maps/tiles/overworld actors originate
 * from pret/pokeyellow. This is a limited adaptation of the opening sequence,
 * NOT a byte-identical implementation of Pokemon Yellow's battle formula,
 * movement scripts, or complete event engine.
 */

enum {
    TalkOak,TalkGirl,TalkFisher,TalkMom,TalkDaisy,TalkRival,
    TalkLabOak,TalkLabGirl,TalkScientist,TalkTownSign,TalkLabSign,
    TalkHomeSign,TalkRivalSign,TalkMap,TalkTV,TalkExit,TalkRouteYoungster,TalkRouteSign,TalkViridianResident,TalkViridianSign,TalkViridianCenter,TalkReception
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
static const char* const caught_text[]={"GOTCHA! WILD","POKEMON CAUGHT!"};
static const char* const proto_kit[]={"PIKACHU WILL","FOLLOW YOU!",
    "DEV KIT: YOU GET","5 POKE BALLS.",
    "ORIGINAL PARCEL","QUEST NOT YET PORTED."};
static const char* const wild_victory[]={"WILD POKEMON","FAINTED!"};
static const char* const wild_flee[]={"GOT AWAY","SAFELY!"};
static const char* const wild_defeat[]={"PIKACHU FAINTED!","BACK TO MOM!"};
static const uint8_t encounter_weights[10]={51,51,39,25,25,25,13,13,11,3};
static const uint8_t route1_levels[10]={3,4,2,3,2,3,5,4,6,7};
static const uint8_t route1_kinds[10]={1,1,2,2,1,1,1,2,1,1};

static const char* const empty_ball[]={"EMPTY POKE BALL","RIVAL TOOK EEVEE"};

typedef struct {const char* const* lines;uint8_t pages;} DialogDef;
static const DialogDef dialogs[]={
    {oak_regular,1},{girl,3},{fisher,3},{mom,1},{daisy,1},
    {rival,1},{lab_oak,1},{lab_girl,1},{scientist,1},
    {town_sign,2},{lab_sign,1},{home_sign,1},{rival_sign,1},
    {townmap,1},{tv,1},{exit_text,1},
    {route_youngster,1},{route_sign,1},{viridian_resident,1},
    {viridian_sign,1},{viridian_center,2},{reception,1}
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
int yellow_map_w(uint8_t map) {
    return map<YELLOW_MAP_COUNT?yellow_levels[map].w*2:0;
}
int yellow_map_h(uint8_t map) {
    return map<YELLOW_MAP_COUNT?yellow_levels[map].h*2:0;
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
bool yellow_npc_visible(const YellowGame* g,const YellowNpc* n) {
    if(g->map!=n->map)return false;
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
    if(map>=YELLOW_MAP_COUNT||x<0||y<0||x>=yellow_map_w(map)||y>=yellow_map_h(map))
        return false;
    /* Gen1 collision checks the bottom-left 8x8 tile of each 16x16 step. */
    uint8_t tile=yellow_tile_id(map,x*2,y*2+1);
    return tile<96&&yellow_levels[map].collision[tile]!=0;
}
static bool destination(uint8_t map,int x,int y,uint8_t* target,uint8_t* dx,uint8_t* dy) {
    if(map==YMapPallet) {
        if(x==5&&y==5){*target=YMapRedsHouse;*dx=2;*dy=6;return true;}
        if(x==13&&y==5){*target=YMapBluesHouse;*dx=2;*dy=6;return true;}
        if(x==12&&y==11){*target=YMapOaksLab;*dx=4;*dy=10;return true;}
    } else if(map==YMapRedsHouse) {
        if((x==2||x==3)&&y==7){*target=YMapPallet;*dx=5;*dy=6;return true;}
    } else if(map==YMapBluesHouse) {
        if((x==2||x==3)&&y==7){*target=YMapPallet;*dx=13;*dy=6;return true;}
    } else if(map==YMapOaksLab) {
        if((x==4||x==5)&&y==11){*target=YMapPallet;*dx=12;*dy=12;return true;}
    } else if(map==YMapViridian) {
        if(x==23&&y==25){*target=YMapViridianCenter;*dx=3;*dy=6;return true;}
    } else if(map==YMapViridianCenter) {
        if((x==3||x==4)&&y==7){*target=YMapViridian;*dx=23;*dy=26;return true;}
    }
    return false;
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
static void set_map(YellowGame* g,uint8_t map,uint8_t x,uint8_t y) {
    g->map=map;g->x=(int8_t)x;g->y=(int8_t)y;
    set_follower_near(g);
}
void yellow_init(YellowGame* g) {
    memset(g,0,sizeof(*g));g->map=YMapRedsHouse;
    g->x=3;g->y=6;g->facing=YUp;g->hp=YELLOW_MAX_HP;
    g->rng=0x5733CC11u;g->pika_level=5;
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
    if(kind>YK_RATTATA||!level) return;
    uint8_t first=first_healthy_party(g);
    if(first==255)return;
    if(yellow_party_hp(g,g->active_party)==0 ||
       g->active_party>=yellow_party_size(g))g->active_party=first;
    g->foe_kind=kind;g->foe_level=level;
    g->foe_max_hp=y_calc_stats(kind,level).hp;
    g->foe_hp=g->foe_max_hp;
    g->foe_attack_stage=0;g->foe_defense_stage=0;
    g->player_accuracy_stage=0;g->foe_accuracy_stage=0;
    g->player_evasion_stage=0;g->foe_evasion_stage=0;
    g->foe_flinched=0;
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
    return species<4 && level?y_calc_stats(species,level).hp:0u;
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
        else {g->caught[i-1u].experience=exp;g->caught[i-1u].level=level;}
    }
}
static void finalize_battle(YellowGame* g,bool won) {
    if(won)grant_exp(g);
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
        yellow_begin_battle(g,YK_EEVEE,5);
        open_dialog(g,challenge_text,2,YPendingBeginRivalBattle);
        break;
    case YPendingBeginRivalBattle:
        g->screen=YScreenRivalBattle;
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
}
static void do_step(YellowGame* g,YellowKey key) {
    int x=g->x,y=g->y;
    uint8_t previous_facing=g->facing;
    g->facing=(uint8_t)key;
    if(key==YUp)--y;
    else if(key==YDown)++y;
    else if(key==YLeft)--x;
    else if(key==YRight)++x;

    if(g->map==YMapPallet&&y==0&&(x==10||x==11)&&g->story==YStoryBeforeOak) {
        open_dialog(g,oak_warning,3,YPendingBeginCapture);
        return;
    }
    /* Outdoor map connections use original width/height and north -5 offset.
       Route 1 (20-step wide) aligns to Viridian City at +10 steps. */
    if(g->story==YStoryPikachuFollows&&key==YUp) {
        if(g->map==YMapPallet&&y<0&&(x==10||x==11)) {
            set_map(g,YMapRoute1,(uint8_t)x,35);g->steps++;return;
        }
        if(g->map==YMapRoute1&&y<0&&(x==10||x==11)) {
            set_map(g,YMapViridian,(uint8_t)(x+10),35);g->steps++;return;
        }
    }
    if(key==YDown) {
        if(g->map==YMapRoute1&&y>=36&&(x==10||x==11)) {
            set_map(g,YMapPallet,(uint8_t)x,0);g->steps++;return;
        }
        if(g->map==YMapViridian&&y>=36&&(x==20||x==21)) {
            set_map(g,YMapRoute1,(uint8_t)(x-10),0);g->steps++;return;
        }
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
    ++g->steps;
    if(g->map==YMapRoute1&&g->has_pikachu&&
       first_healthy_party(g)!=255 &&
       yellow_is_grass(g->map,g->x,g->y) &&
       (next_rng(g)&255u)<25u) {
        uint8_t roll=(uint8_t)(next_rng(g)&255u);
        int slot=0, sum=0;
        while(slot<9) {
            sum+=encounter_weights[slot];
            if(roll<sum)break;
            ++slot;
        }
        yellow_begin_battle(g,route1_kinds[slot],route1_levels[slot]);
    }
}
bool yellow_is_grass(uint8_t map,int x,int y) {
    return map==YMapRoute1 && yellow_tile_id(map,x*2,y*2+1)==0x52;
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
    /* The Gen1 nurse talks across a one-step Pokecenter counter. */
    if(g->map==YMapViridianCenter&&x==3&&y==2&&g->facing==YUp) {
        yellow_heal_pikachu(g);
        dialog_id(g,TalkViridianCenter);
        return;
    }
    for(uint8_t i=0;i<yellow_npc_count;i++) {
        const YellowNpc* n=&yellow_npcs[i];
        if(n->x==x&&n->y==y&&yellow_npc_visible(g,n)) {
            if(n->type==YActorNurse)yellow_heal_pikachu(g);
            if(n->map==YMapOaksLab&&n->type==YActorPokeBall) {
                if(g->story==YStoryChooseStarter) {
                    open_dialog(g,starter_text,3,YPendingStartBattle);
                } else open_dialog(g,empty_ball,1,YPendingNone);
                return;
            }
            if(n->map==YMapOaksLab&&g->story==YStoryBeforeOak&&n->type==YActorOak)
                open_dialog(g,early_lab,1,YPendingNone);
            else dialog_id(g,n->dialog_id);
            return;
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
    if(g->foe_kind==YK_RATTATA) {
        if(g->foe_level>=14u && (choice&7u)==2u)return YM_HYPER_FANG;
        if(g->foe_level>=7u && (choice&3u)==0u)return YM_QUICK_ATTACK;
        return (choice&7u)==1u?YM_TAIL_WHIP:YM_TACKLE;
    }
    return (choice&7u)==0u?YM_TAIL_WHIP:YM_TACKLE;
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
static void apply_move_effect(YellowGame* g,bool player,uint8_t move,
                              uint8_t damage,bool critical) {
    int8_t* target_attack=player?&g->foe_attack_stage:&g->pika_attack_stage;
    int8_t* target_defense=player?&g->foe_defense_stage:&g->pika_defense_stage;
    int8_t* target_accuracy=player?&g->foe_accuracy_stage:&g->player_accuracy_stage;
    int8_t* source_evasion=player?&g->player_evasion_stage:&g->foe_evasion_stage;
    switch(move) {
    case YM_GROWL:shift_stage(target_attack,-1);return;
    case YM_TAIL_WHIP:shift_stage(target_defense,-1);return;
    case YM_SAND_ATTACK:shift_stage(target_accuracy,-1);return;
    case YM_DOUBLE_TEAM:shift_stage(source_evasion,+1);return;
    case YM_THUNDER_WAVE:
        if(player) {
            if(g->foe_status==YStatusNone)g->foe_status=YStatusParalyzed;
        } else if(yellow_party_status(g,g->active_party)==YStatusNone) {
            if(g->active_party==0)g->pika_status=YStatusParalyzed;
            else g->caught[g->active_party-1u].status=YStatusParalyzed;
        }
        return;
    default:break;
    }
    if(!damage)return; /* no effects after an immunity/zero damage */
    if(critical)g->last_critical=1;
    if(player) {
        if(g->foe_hp<=damage) {
            finalize_battle(g,true);
            return;
        }
        g->foe_hp-=damage;
        if((move==YM_THUNDERSHOCK || move==YM_THUNDERBOLT) &&
           y_thundershock_paralyzes((uint8_t)next_rng(g),g->foe_status))
            g->foe_status=YStatusParalyzed;
        if(move==YM_HYPER_FANG && (uint8_t)next_rng(g)<26u)
            g->foe_flinched=1;
    } else {
        uint8_t hp=yellow_party_hp(g,g->active_party);
        if(hp<=damage) {
            set_party_hp(g,g->active_party,0);
            uint8_t next=first_healthy_party(g);
            if(next==255) {
                finalize_battle(g,false);
                return;
            }
            g->screen=YScreenParty;
            g->party_battle=1;g->force_switch=1;g->selected=next;
        } else set_party_hp(g,g->active_party,(uint8_t)(hp-damage));
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
    if(g->foe_status==YStatusParalyzed &&
       y_full_paralysis((uint8_t)next_rng(g))) {
        g->screen=YScreenRivalBattle;
        (void)apply_residual_after_action(g,false);
        return;
    }
    execute_enemy_action(g,choose_enemy_move(g),false);
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
static void battle_move(YellowGame* g) {
    uint8_t move_count=yellow_active_move_count(g);
    if(g->selected==move_count) {
        /* Poké Balls only work against wild Pokémon and if storage has room.
           NPC party/box transfer and trainer theft do not exist here. */
        if(g->foe_kind==YK_EEVEE || !g->balls ||
           yellow_party_size(g)>=YELLOW_PARTY_MAX)return;
        g->balls--;
        uint8_t r1=(uint8_t)next_rng(g);
        uint8_t r2=(uint8_t)next_rng(g);
        if(y_gen1_pokeball_capture_status(y_species_catch_rate(g->foe_kind),
                                  g->foe_max_hp,g->foe_hp,r1,r2,g->foe_status)) {
            YCapturedMon* mon=&g->caught[g->caught_count++];
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
void yellow_input(YellowGame* g,YellowKey key) {
    if(!g->running)return;
    switch(g->screen) {
    case YScreenWorld:
        if(key<=YRight)do_step(g,key);
        else if(key==YOk)interact(g);
        else if(key==YBack){g->screen=YScreenMenu;g->selected=0;}
        break;
    case YScreenDialog:
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
        if(key==YUp&&g->selected)g->selected--;
        else if(key==YDown&&g->selected<3)g->selected++;
        else if(key==YBack)g->screen=YScreenWorld;
        else if(key==YOk) {
            if(g->selected==0){g->screen=YScreenParty;g->selected=0;}
            else if(g->selected==1)g->screen=YScreenBag;
            else if(g->selected==2){g->request_save=true;g->message_saved=false;}
            else g->running=false;
        }
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
        if(key==YBack||key==YOk){g->screen=YScreenMenu;g->selected=1;}
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
void yellow_save_create(const YellowGame* g,YellowSave* s) {
    memset(s,0,sizeof(*s));
    s->magic=YELLOW_SAVE_MAGIC;s->version=13;
    s->map=g->map;s->x=(uint8_t)g->x;s->y=(uint8_t)g->y;
    s->facing=g->facing;s->hp=g->hp;s->potions=g->potions;
    s->story=(uint8_t)g->story;s->has_pikachu=g->has_pikachu;s->affection=g->affection;
    s->foe_hp=g->foe_hp;s->foe_kind=g->foe_kind;
    s->foe_level=g->foe_level;s->foe_max_hp=g->foe_max_hp;s->pika_level=g->pika_level;
    s->follower_map=g->follower_map;
    s->follower_x=g->follower_x;s->follower_y=g->follower_y;
    s->follower_valid=g->follower_valid;s->steps=g->steps;s->rng=g->rng;
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
bool yellow_save_load(YellowGame* g,const YellowSave* s) {
    if(s->magic!=YELLOW_SAVE_MAGIC||s->version!=13||
       s->map>=YELLOW_MAP_COUNT||s->x>=yellow_map_w(s->map)||
       s->y>=yellow_map_h(s->map)||s->facing>YRight||
       s->pika_level<5||s->pika_level>100||
       s->hp>y_calc_stats(YK_PIKACHU,s->pika_level).hp||
       s->foe_kind>2||s->foe_level>100||
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
           (m->species!=YK_PIDGEY&&m->species!=YK_RATTATA)||
           m->level<2||m->level>100||
           m->hp>y_calc_stats(m->species,m->level).hp||
           m->experience<y_exp_threshold(m->species,m->level)||
           (m->level<100&&m->experience>=y_exp_threshold(m->species,(uint8_t)(m->level+1)))||
           !yellow_valid_moves(m->species,m->level,m->moves,m->pp))return false;
    }
    YellowGame tmp={0};tmp.map=s->map;tmp.story=(YellowStory)s->story;
    if(!yellow_walkable(&tmp,s->x,s->y))return false;

    yellow_init(g);
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
    g->active_party=s->active_party;
    g->pika_status=s->pika_status;
    if(yellow_party_hp(g,g->active_party)==0) {
        uint8_t next=first_healthy_party(g);
        if(next==255)return false;
        g->active_party=next;
    }
    g->screen=YScreenWorld;g->pending=YPendingNone;
    set_follower_near(g); /* rebuild follower, do not trust saved stale coordinates */
    return true;
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
