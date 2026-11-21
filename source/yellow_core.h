#ifndef YELLOW_CORE_H
#define YELLOW_CORE_H
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "yellow_battle.h"
#include "yellow_pokedex_ledger.h"

#define YELLOW_MAP_COUNT 11 /* legacy native map count and v13 save scope */
#define YELLOW_PLAYABLE_MAP_COUNT 12 /* v038 city-only contract, kept stable */
#define YELLOW_GYM_MAP_COUNT 13 /* existing v039 contract */
#define YELLOW_ROUTE3_MAP_COUNT 14 /* source Route3 accepted v048 */
#define YELLOW_ROUTE4_MAP_COUNT 15 /* accepted v050 */
#define YELLOW_MTMOON_MAP_COUNT 16 /* source-backed Mt Moon 1F */
#define YELLOW_B2F_MAP_ID (YELLOW_MTMOON_MAP_COUNT + 1) /* source-backed Mt Moon B2F */
#define YELLOW_CERULEAN_MAP_ID (YELLOW_B2F_MAP_ID + 1u) /* native id 18, source id 3 */
#define YELLOW_ROUTE9_MAP_ID (YELLOW_CERULEAN_MAP_ID + 1u) /* native id 19, source id 20 */
#define YELLOW_ROUTE22_MAP_ID (YELLOW_ROUTE9_MAP_ID + 1u) /* native id 20, source id 33 */
#define YELLOW_MART_MAP_ID (YELLOW_ROUTE22_MAP_ID + 1u) /* native id 21, source id 42 */
#define YELLOW_CERULEAN_MART_ID (YELLOW_MART_MAP_ID+1u) /* native22/source67 */
uint8_t yellow_world_tile_id(uint8_t map,int x,int y);
void yellow_b2f_set_enabled(bool enabled);
#define YELLOW_LINK_PACKET_SIZE 12u
#define YELLOW_SAVE_MAGIC 0x59464C50u
#define FOREST_TRAINER_SAVE_MAGIC 0x46545231u
#define YELLOW_MAX_HP 19u
#define YELLOW_PARTY_MAX 6
#define YELLOW_CAPTURE_SLOTS 5
#define YELLOW_MOVE_SLOTS 4
#define YELLOW_LEARN_QUEUE_CAPACITY 64
#define YELLOW_RIVAL_MAX_HP 21u

typedef enum {
    YScreenWorld, YScreenDialog, YScreenMenu, YScreenParty, YScreenBag,
    YScreenOakEncounter, YScreenRivalBattle, YScreenFightMenu,
    YScreenMoveForget,YScreenLinkOffline,YScreenPC,YScreenPokedex,
    YScreenNurseChoice /* R14 opt-in, no v13 save ABI change */
} YellowScreen;
typedef enum {YUp,YDown,YLeft,YRight,YOk,YBack} YellowKey;
/* Ephemeral Bag feedback: never serialized into YellowSave. */
typedef enum {YBagNoNotice,YBagHealed,YBagFullHP,YBagNoPotions,YBagFainted,YBagNoParty} YBagNotice;
typedef enum {YMapPallet,YMapRedsHouse,YMapBluesHouse,YMapOaksLab,YMapRoute1,YMapViridian,YMapViridianCenter,YMapRoute2,YMapViridianForest,YMapForestSouthGate,YMapForestNorthGate,YMapPewterCity,YMapPewterGym,YMapRoute3,YMapRoute4,YMapMtMoon1F,YMapMtMoonB1F,YMapMtMoonB2F,YMapCeruleanCity,YMapRoute9,YMapRoute22,YMapViridianMart,YMapCeruleanMart,YMapRoute10} YellowMap;
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
    YPendingStartBattle, YPendingBeginRivalBattle, YPendingEndBattle, YPendingEndWildBattle,
    YPendingBeginForestLassBattle,YPendingBeginPewterJrBattle,YPendingBeginPewterBrockBattle,YPendingGrantPewterTM34,
    YPendingBeginRoute3TrainerBattle,YPendingGrantOakParcel,YPendingDeliverOakParcel,
    YPendingBuyAntidote /* explicit confirmation, BACK cancels */
} YellowPending;
typedef struct {
    uint8_t map,x,y,type,dialog_id;
} YellowNpc;
/* Local-only offline trade-preview frame. NOT a USB/Bluetooth transport. */
typedef struct {
    uint16_t sequence;
    uint8_t kind,species,level,hp,status;
} YellowLinkSnapshot;
bool yellow_link_encode(const YellowLinkSnapshot* input,uint8_t output[YELLOW_LINK_PACKET_SIZE]);
bool yellow_link_decode(const uint8_t* packet,size_t length,YellowLinkSnapshot* output);

/* New versioned save; older v0.2 save file remains untouched. */
typedef struct {
    uint8_t species,level,hp,pp[YELLOW_MOVE_SLOTS];
    uint8_t moves[YELLOW_MOVE_SLOTS];
    uint8_t status;
    uint32_t experience;
} YCapturedMon;
#define YELLOW_PC_CAPACITY 20u
#define YELLOW_PC_SLOT_BYTES 16u
#define YELLOW_PC_MAGIC 0x31435059u /* YPC1 */
#define YELLOW_PC_VERSION 1u
typedef struct {
    uint8_t count;
    YCapturedMon slots[YELLOW_PC_CAPACITY];
} YellowPCBox;
/* Fixed, byte-encoded slots, independent of ABI padding of YCapturedMon. */
typedef struct {
    uint32_t magic;
    uint16_t version;
    uint8_t count,reserved;
    uint32_t parent_checksum;
    uint8_t slots[YELLOW_PC_CAPACITY][YELLOW_PC_SLOT_BYTES];
    uint32_t checksum;
} YellowPCSave;
/* Original Route3 trainer header bits 2,4,6. Optional sidecar tied to v13
   checksum; no mutation of original 132-byte native save. */
#define YELLOW_ROUTE3_DEFEATED_BITS ((1u<<2)|(1u<<4)|(1u<<6))
#define YELLOW_ROUTE3_TRAINER_MAGIC 0x33545259u
typedef struct {
    uint32_t magic;
    uint16_t version;
    uint8_t defeated_mask,reserved;
    uint32_t parent_checksum;
    uint32_t checksum;
} YellowRoute3TrainerSave;
bool yellow_route3_trainer_save(uint8_t defeated_mask,uint32_t parent_checksum,
                                YellowRoute3TrainerSave* out);
bool yellow_route3_trainer_restore(uint8_t* mask,uint32_t parent_checksum,
                                   const YellowRoute3TrainerSave* saved);
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

/* Optional versioned sidecar, bound to one verified v13 save checksum.
   Prior save file and layout are intentionally unchanged. */
typedef struct {
    uint32_t magic;
    uint16_t version;
    uint8_t defeated_mask, reserved;
    uint32_t parent_checksum;
    uint32_t checksum;
} YellowTrainerSave;
/* Optional item sidecar: preserves pre-existing v13 and trainer state. */
#define FOREST_ITEM_SAVE_MAGIC 0x46544931u
typedef struct {
    uint32_t magic;
    uint16_t version;
    uint8_t collected_mask,reserved;
    uint32_t parent_checksum;
    uint32_t checksum;
} YellowItemSave;


/* Runtime-only imported encounter data; NOT part of v13 saves. */
typedef struct {
    uint8_t rate;
    uint8_t kind[10], level[10];
} YellowWildSlots;
typedef struct {
    YellowWildSlots route1, forest;
} YellowBulkWild;
/* Original OPP_LASS 19 party, compiled from pinned trainer records. */
typedef struct {
    uint8_t count,kind[2],level[2];
} YellowBulkLass;

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
    int8_t pika_speed_stage,foe_speed_stage; /* volatile battle stages */
    int8_t player_accuracy_stage,foe_accuracy_stage;
    int8_t player_evasion_stage,foe_evasion_stage;
    uint8_t foe_flinched;
    uint8_t foe_confusion_turns,party_confusion_turns[YELLOW_PARTY_MAX]; /* volatile Gen1 2..5 turns */
    bool native_zubat_extension; /* enabled by Flipper runtime, not v13 save */
    bool native_clefairy_extension; /* Lv11 source cave overlay */
    bool native_nurse_choice_extension; /* volatile opt-in; explicit Yes/No before heal */
    bool native_mart_intro_extension; /* original automatic Viridian Mart clerk greeting; volatile */
    uint8_t native_mart_intro_pending; /* volatile script step, not in v13 */
    uint8_t foe_status;
    uint16_t last_exp_award;
    uint8_t last_critical,last_enemy_move,last_player_first;
    uint8_t participants_mask,party_battle,force_switch;
    uint8_t learn_count,learn_index;
    uint8_t learn_party[YELLOW_LEARN_QUEUE_CAPACITY];
    uint8_t learn_moves[YELLOW_LEARN_QUEUE_CAPACITY];
    uint8_t follower_map,follower_x,follower_y,follower_valid;
    uint32_t rng;
    /* Volatile offline loopback state; deliberately not in YellowSave. */
    uint16_t link_sequence;
    uint8_t link_result;
    uint8_t forest_defeated_mask,forest_seen_mask;
    uint8_t forest_pickups; /* three collectible forest map objects */
    uint8_t forest_active_trainer,forest_party_index;
    /* Source-mapped gym state; persisted in separate v1 sidecar, NOT v13. */
    uint8_t pewter_flags;
    YellowDex dex; /* 151 source-numbered seen/caught bits, optional sidecar */
    YellowPCBox pc; /* 20 boxed native-supported Pokémon; separate sidecar */
    uint8_t pc_mode; /* 0 main, 1 deposit list, 2 withdraw list; volatile */
    uint8_t pewter_active_trainer,pewter_party_index; /* 0 = idle, 1 = Jr, 2 = Brock */
    uint8_t route3_defeated_mask; /* original Route3 trainer headers 2,4,6 */
    uint8_t route3_active_trainer,route3_pending_trainer,route3_party_index;
    /* active/pending: 0 idle, 1 Lass1, 2 Lass2, 3 BugCatcher6; volatile */
    const YellowBulkWild* imported_wild; /* opt-in, runtime only */
    const YellowBulkLass* imported_lass; /* optional sourced trainer team */
    uint8_t bag_notice,bag_last_heal; /* Runtime-only UX; excludes YellowSave. */
    uint8_t overworld_blackout_pending; /* Volatile Gen1 out-of-battle blackout signal; NOT v13. */
    /* Authentic Route1 script prefix state; volatile, never in v13 save. */
    uint8_t source_script_state; /* bit 0 auto textbox, bit 1 script returned */
    uint8_t oak_parcel_flags; /* optional SD slot only; NOT YellowSave v13 */
    bool oak_parcel_integrity_error; /* optional SD record exists but invalid: block future grants/saves */
    uint8_t last_blackout_original_map; /* 0=unknown; 1=Viridian City, never native map enum */
    bool last_blackout_integrity_error; /* corrupt optional record blocks unsafe saves */
    /* R20 optional per-generation SD sidecar. No YellowSave v13 ABI change. */
    uint8_t antidotes;
    bool antidote_integrity_error; /* present corrupt sidecar: no item use or resave */
    uint8_t money_bcd[3]; /* original 3000 BCD, paired inventory sidecar */
    bool money_integrity_error; /* corrupt paired sidecar locks purchase and save */
} YellowGame;
bool yellow_pc_deposit(YellowGame* g,uint8_t party_index);
bool yellow_pc_withdraw(YellowGame* g,uint8_t box_index);
bool yellow_pc_save(const YellowPCBox* box,uint32_t parent_checksum,YellowPCSave* out);
bool yellow_pc_restore(YellowPCBox* box,uint32_t parent_checksum,const YellowPCSave* record);
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
/* R18 native party Antidote effect only; caller supplies item stock.
 * No bag UI or inventory save wiring yet. */
bool yellow_game_apply_antidote(YellowGame* g,uint8_t party_index,uint8_t* stock);
/* Optional shop module: legacy core-only tests must link and fail closed. */
bool yellow_game_sell_antidote(YellowGame* g,uint8_t quantity);
bool yellow_game_buy_antidote(YellowGame* g,uint8_t quantity)
#if defined(__GNUC__)
    __attribute__((weak))
#endif
;
uint8_t yellow_active_move_count(const YellowGame* g);
bool yellow_can_switch(const YellowGame* g,uint8_t index);
void yellow_init(YellowGame* g);
void yellow_input(YellowGame* g,YellowKey key);
bool yellow_attach_bulk_lass(YellowGame* g,const YellowBulkLass* imported);
bool yellow_attach_bulk_wild(YellowGame* g,const YellowBulkWild* imported);
bool yellow_wild_slot(const YellowGame* g,uint8_t map,uint8_t slot,
                      uint8_t* kind,uint8_t* level);
bool yellow_walkable(const YellowGame* g,int x,int y);
uint8_t yellow_tile_id(uint8_t map,int tile_x,int tile_y);
int yellow_map_w(uint8_t map);
int yellow_map_h(uint8_t map);
bool yellow_npc_visible(const YellowGame* g,const YellowNpc* npc);
bool yellow_is_grass(uint8_t map,int x,int y);
void yellow_save_create(const YellowGame* g,YellowSave* save);
void yellow_item_save_create(const YellowGame* g,const YellowSave* parent,YellowItemSave* out);
bool yellow_item_save_load(YellowGame* g,const YellowSave* parent,const YellowItemSave* sidecar);
bool yellow_save_load_with_items(YellowGame* g,const YellowSave* parent,const YellowItemSave* sidecar);
void yellow_trainer_save_create(const YellowGame* g,const YellowSave* parent,YellowTrainerSave* out);
bool yellow_trainer_save_load(YellowGame* g,const YellowSave* parent,const YellowTrainerSave* sidecar);
bool yellow_save_load(YellowGame* g,const YellowSave* save);
bool yellow_save_migrate_r10(YellowGame* g,const YellowSave* old_save);
bool yellow_save_migrate_r11(YellowGame* g,const YellowSave* old_save);
bool yellow_save_migrate_r12(YellowGame* g,const YellowSave* old_save);
#endif
