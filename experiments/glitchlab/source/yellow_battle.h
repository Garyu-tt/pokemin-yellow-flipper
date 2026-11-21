#ifndef YELLOW_BATTLE_H
#define YELLOW_BATTLE_H
#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef enum {YK_EEVEE=0,YK_PIDGEY=1,YK_RATTATA=2,YK_PIKACHU=3} YSpecies;
typedef enum {
    YM_THUNDERSHOCK=0,YM_GROWL,YM_TAIL_WHIP,YM_TACKLE,
    YM_GUST,YM_QUICK_ATTACK,YM_SAND_ATTACK,
    YM_THUNDER_WAVE,YM_DOUBLE_TEAM,YM_SLAM,YM_THUNDERBOLT,
    YM_HYPER_FANG,YM_NONE=255
} YMoveId;
/* Raw Generation I type IDs from constants/type_constants.asm. */
typedef enum {
 YT_NORMAL=0,YT_FIGHTING=1,YT_FLYING=2,YT_POISON=3,YT_GROUND=4,
 YT_ROCK=5,YT_BIRD=6,YT_BUG=7,YT_GHOST=8,
 YT_FIRE=20,YT_WATER=21,YT_GRASS=22,YT_ELECTRIC=23,
 YT_PSYCHIC=24,YT_ICE=25,YT_DRAGON=26
} YType;
/* Internal small enum, NOT the original Gen1 status bitfield. */
typedef enum {
    YStatusNone=0,YStatusParalyzed=1,
    YStatusBurned=2,YStatusPoisoned=3,YStatusFrozen=4,
    /* Encoded active counters occupy the same persistent status byte. */
    YStatusSleep1=5,YStatusSleep2,YStatusSleep3,YStatusSleep4,
    YStatusSleep5,YStatusSleep6,YStatusSleep7,
    YStatusToxic1=12,YStatusToxic2,YStatusToxic3,YStatusToxic4,
    YStatusToxic5,YStatusToxic6,YStatusToxic7,YStatusToxic8,
    YStatusToxic9,YStatusToxic10,YStatusToxic11,YStatusToxic12,
    YStatusToxic13,YStatusToxic14,YStatusToxic15
} YStatus;
typedef struct {
    const char* name;
    uint8_t hp,attack,defense,speed,special,exp_yield;
    uint8_t type1,type2;
    uint8_t growth; /* 0=medium-fast, 1=medium-slow */
} YSpeciesInfo;
typedef struct {
    const char* name;
    uint8_t power,accuracy,pp,type,special;
} YMoveInfo;
typedef struct {
    uint8_t hp,attack,defense,speed,special;
} YBattleStats;
typedef struct {
    uint8_t damage,hit;
} YDamage;

const YSpeciesInfo* y_species_info(uint8_t species);
const YMoveInfo* y_move_info(uint8_t move);
YBattleStats y_calc_stats(uint8_t species,uint8_t level);
uint32_t y_exp_threshold(uint8_t species,uint8_t level);
uint16_t y_exp_award(uint8_t species,uint8_t level,bool trainer);
uint8_t y_max_pp(uint8_t move);
uint8_t y_species_catch_rate(uint8_t species);
bool y_gen1_pokeball_capture(uint8_t catch_rate,uint8_t max_hp,
                             uint8_t current_hp,uint8_t rand1,uint8_t rand2);
bool y_gen1_pokeball_capture_status(uint8_t catch_rate,uint8_t max_hp,
    uint8_t current_hp,uint8_t rand1,uint8_t rand2,uint8_t status);
uint8_t y_move_accuracy_threshold(uint8_t move,int8_t accuracy_stage,int8_t evasion_stage);
bool y_move_hits(uint8_t move,int8_t accuracy_stage,int8_t evasion_stage,uint8_t rng_roll);
bool y_thundershock_paralyzes(uint8_t rng_roll,uint8_t target_status);
bool y_full_paralysis(uint8_t rng_roll);
uint16_t y_effective_speed(uint8_t species,uint8_t level,uint8_t status);
uint8_t y_learned_move_at(uint8_t species,uint8_t level);
void y_build_moveset(uint8_t species,uint8_t level,uint8_t out_moves[4]);
bool y_move_available(uint8_t species,uint8_t level,uint8_t move);
uint8_t y_player_move(uint8_t level,uint8_t slot);
uint8_t y_wild_move(uint8_t species,uint8_t level,uint8_t slot);
uint8_t y_player_move_count(uint8_t level);
uint16_t y_type_effectiveness(uint8_t attack_type,uint8_t defender_type1,
                               uint8_t defender_type2);
uint8_t y_move_priority(uint8_t move);
bool y_player_moves_first(uint8_t player_move,uint8_t enemy_move,
                          uint16_t player_speed,uint16_t enemy_speed,uint8_t tie_roll);
uint8_t y_critical_threshold(uint8_t species,uint8_t move);
bool y_roll_critical(uint8_t species,uint8_t move,uint8_t rng_roll);
bool y_status_is_sleep(uint8_t status);
bool y_status_is_toxic(uint8_t status);
bool y_status_valid(uint8_t status);
uint8_t y_sleep_after_action(uint8_t status);
uint8_t y_toxic_after_tick(uint8_t status);
uint8_t y_residual_damage(uint8_t max_hp,uint8_t status);
YDamage y_calc_damage_status_crit(uint8_t source,uint8_t source_level,
                                  uint8_t target,uint8_t target_level,
                                  uint8_t move,int8_t attack_stage,int8_t defense_stage,
                                  uint8_t hit_roll,uint8_t rand_roll,bool critical,
                                  uint8_t attacker_status);
YDamage y_calc_damage_crit(uint8_t source,uint8_t source_level,
                           uint8_t target,uint8_t target_level,
                           uint8_t move,int8_t attack_stage,int8_t defense_stage,
                           uint8_t hit_roll,uint8_t rand_roll,bool critical);
YDamage y_calc_damage(uint8_t source,uint8_t source_level,
                      uint8_t target,uint8_t target_level,
                      uint8_t move,int8_t attack_stage,int8_t defense_stage,
                      uint8_t hit_roll,uint8_t rand_roll);

#endif
