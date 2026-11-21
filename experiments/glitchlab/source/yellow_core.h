#ifndef YELLOW_CORE_H
#define YELLOW_CORE_H
#include <stdbool.h>
#include <stdint.h>
#include "yellow_battle.h"

#define YELLOW_MAP_COUNT 7
#define YELLOW_SAVE_MAGIC 0x59464C50u
#define YELLOW_MAX_HP 19u
#define YELLOW_PARTY_MAX 6
#define YELLOW_CAPTURE_SLOTS 5
#define YELLOW_MOVE_SLOTS 4
#define YELLOW_LEARN_QUEUE_CAPACITY 64
#define YELLOW_RIVAL_MAX_HP 21u

typedef enum {
    YScreenWorld, YScreenDialog, YScreenMenu, YScreenParty, YScreenBag,
    YScreenOakEncounter, YScreenRivalBattle, YScreenFightMenu,
    YScreenMoveForget
} YellowScreen;
typedef enum {YUp,YDown,YLeft,YRight,YOk,YBack} YellowKey;
typedef enum {YMapPallet,YMapRedsHouse,YMapBluesHouse,YMapOaksLab,YMapRoute1,YMapViridian,YMapViridianCenter} YellowMap;
typedef enum {
    YActorRed,YActorOak,YActorGirl,YActorFisher,
    YActorBlue,YActorPikachu,YActorMom,YActorDaisy,
    YActorPokedex,YActorPokeBall,YActorScientist,YActorYoungster,YActorGambler,YActorNurse,YActorGentleman,YActorCooltrainer,YActorReceptionist,YActorChansey
} YellowActorType;
typedef enum {
    YStoryBeforeOak, YStoryOakEncounter,
    YStoryChooseStarter, YStoryRivalBattle, YStoryPikachuFollows
} YellowStory;
typedef enum {
    YPendingNone, YPendingBeginCapture, YPendingEnterLab,
    YPendingStartBattle, YPendingBeginRivalBattle, YPendingEndBattle, YPendingEndWildBattle
} YellowPending;
typedef struct {
    uint8_t map,x,y,type,dialog_id;
} YellowNpc;

/* New versioned save; older v0.2 save file remains untouched. */
typedef struct {
    uint8_t species,level,hp,pp[YELLOW_MOVE_SLOTS];
    uint8_t moves[YELLOW_MOVE_SLOTS];
    uint8_t status;
    uint32_t experience;
} YCapturedMon;
typedef struct {
    uint32_t magic;
    uint16_t version;
    uint8_t map,x,y,facing,hp,potions;
    uint8_t story,has_pikachu,affection,foe_hp;
    uint8_t foe_kind,foe_level,foe_max_hp,pika_level;
    uint8_t follower_map,follower_x,follower_y,follower_valid;
    uint16_t steps;
    uint8_t pika_pp[YELLOW_MOVE_SLOTS];
    uint8_t pika_moves[YELLOW_MOVE_SLOTS];
    uint8_t balls,caught_count;
    YCapturedMon caught[YELLOW_CAPTURE_SLOTS];
    uint8_t active_party,pika_status;
    uint32_t experience;
    uint32_t rng;
    uint32_t checksum;
} YellowSave;

typedef struct {
    int8_t x,y;
    uint8_t map,facing,hp,potions;
    uint16_t steps;
    uint8_t selected;
    uint8_t dialog_page,dialog_count;
    const char* const* dialog_lines;
    YellowScreen screen;
    bool running,request_save,message_saved;
    YellowStory story;
    YellowPending pending;
    uint8_t has_pikachu,affection,foe_hp,foe_attack_debuff;
    uint8_t foe_kind,foe_level,foe_max_hp,pika_level;
    uint8_t pika_pp[YELLOW_MOVE_SLOTS];
    uint8_t pika_moves[YELLOW_MOVE_SLOTS];
    uint8_t balls,caught_count;
    YCapturedMon caught[YELLOW_CAPTURE_SLOTS];
    uint8_t active_party,pika_status;
    uint32_t experience;
    int8_t pika_attack_stage,pika_defense_stage;
    int8_t foe_attack_stage,foe_defense_stage;
    int8_t player_accuracy_stage,foe_accuracy_stage;
    int8_t player_evasion_stage,foe_evasion_stage;
    uint8_t foe_flinched;
    uint8_t foe_status;
    uint16_t last_exp_award;
    uint8_t last_critical,last_enemy_move,last_player_first;
    uint8_t participants_mask,party_battle,force_switch;
    uint8_t learn_count,learn_index;
    uint8_t learn_party[YELLOW_LEARN_QUEUE_CAPACITY];
    uint8_t learn_moves[YELLOW_LEARN_QUEUE_CAPACITY];
    uint8_t follower_map,follower_x,follower_y,follower_valid;
    uint32_t rng;
} YellowGame;
extern const YellowNpc yellow_npcs[];
extern const uint8_t yellow_npc_count;

uint8_t yellow_player_max_hp(const YellowGame* g);
void yellow_heal_pikachu(YellowGame* g);
void yellow_begin_battle(YellowGame* g,uint8_t kind,uint8_t level);
uint8_t yellow_party_size(const YellowGame* g);
uint8_t yellow_party_species(const YellowGame* g,uint8_t index);
uint8_t yellow_party_level(const YellowGame* g,uint8_t index);
uint8_t yellow_party_hp(const YellowGame* g,uint8_t index);
uint8_t yellow_party_max_hp(const YellowGame* g,uint8_t index);
uint32_t yellow_party_exp(const YellowGame* g,uint8_t index);
uint8_t yellow_party_move(const YellowGame* g,uint8_t index,uint8_t slot);
uint8_t yellow_party_pp(const YellowGame* g,uint8_t index,uint8_t slot);
uint8_t yellow_party_status(const YellowGame* g,uint8_t index);
uint8_t yellow_active_move_count(const YellowGame* g);
bool yellow_can_switch(const YellowGame* g,uint8_t index);
void yellow_init(YellowGame* g);
void yellow_input(YellowGame* g,YellowKey key);
bool yellow_walkable(const YellowGame* g,int x,int y);
uint8_t yellow_tile_id(uint8_t map,int tile_x,int tile_y);
int yellow_map_w(uint8_t map);
int yellow_map_h(uint8_t map);
bool yellow_npc_visible(const YellowGame* g,const YellowNpc* npc);
bool yellow_is_grass(uint8_t map,int x,int y);
void yellow_save_create(const YellowGame* g,YellowSave* save);
bool yellow_save_load(YellowGame* g,const YellowSave* save);
bool yellow_save_migrate_r10(YellowGame* g,const YellowSave* old_save);
bool yellow_save_migrate_r11(YellowGame* g,const YellowSave* old_save);
bool yellow_save_migrate_r12(YellowGame* g,const YellowSave* old_save);
#endif
