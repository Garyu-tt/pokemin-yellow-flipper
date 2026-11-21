#include "yellow_battle.h"
#include "yellow_type_matchups.generated.h"
#include "yellow_source_species.c"
#include "yellow_source_moves.c" /* 165 pinned source move records */ /* verified 151-species source ROM table */

/*
 * Base stats, growth curves, move power/PP/type:
 * pret/pokeyellow data/pokemon/base_stats/<species>.asm,
 * data/pokemon/evos_moves.asm, data/moves/moves.asm.
 * Scope: starter and Route 1 encounters, fixed DV=15, zero StatExp.
 * This remains a partial Gen1 implementation (no StatExp, status breadth, AI).
 */
static const YSpeciesInfo mons[]={
    {"EEVEE",55,55,50,55,65,92,YT_NORMAL,YT_NORMAL,0},
    {"PIDGEY",40,45,40,56,35,55,YT_NORMAL,YT_FLYING,1},
    {"RATTATA",30,56,35,72,25,57,YT_NORMAL,YT_NORMAL,0},
    {"PIKACHU",35,55,30,90,50,82,YT_ELECTRIC,YT_ELECTRIC,0},
    {"CATERPIE",45,30,35,45,20,53,YT_BUG,YT_BUG,0},
    {"METAPOD",50,20,55,30,25,72,YT_BUG,YT_BUG,0},
    {"PIDGEOTTO",63,60,55,71,50,113,YT_NORMAL,YT_FLYING,1},
    /* Base stats from pinned original data/pokemon/base_stats/nidoranf/m.asm. */
    {"NIDORAN F",55,47,52,41,40,59,YT_POISON,YT_POISON,1},
    {"NIDORAN M",46,57,40,50,40,60,YT_POISON,YT_POISON,1},
    /* Pinned original Pewter Gym and Jr. Trainer parties; no placeholders. */
    {"GEODUDE",40,80,100,20,30,86,YT_ROCK,YT_GROUND,1},
    {"ONIX",35,45,160,70,30,108,YT_ROCK,YT_GROUND,0},
    {"DIGLETT",10,55,25,95,45,81,YT_GROUND,YT_GROUND,0},
    {"SANDSHREW",50,75,85,40,30,93,YT_GROUND,YT_GROUND,0},
    {0,0,0,0,0,0,0,0,0,0}, /* id13 remains invalid, no fake legacy species */
    {"ZUBAT",40,45,35,55,40,54,YT_POISON,YT_FLYING,0}, /* #41 */
    {"CLEFAIRY",70,45,48,35,60,68,YT_NORMAL,YT_NORMAL,2}, /* #35 FAST */
};
static const YMoveInfo moves[]={
    {"THUNDERSHOCK",40,100,30,YT_ELECTRIC,1},
    {"GROWL",0,100,40,YT_NORMAL,0},
    {"TAIL WHIP",0,100,30,YT_NORMAL,0},
    {"TACKLE",35,95,35,YT_NORMAL,0},
    {"GUST",40,100,35,YT_NORMAL,0}, /* Gen1 Gust is NORMAL */
    {"QUICK ATTACK",40,100,30,YT_NORMAL,0},
    {"SAND ATTACK",0,100,15,YT_NORMAL,0},
    {"THUNDER WAVE",0,100,20,YT_ELECTRIC,1},
    {"DOUBLE TEAM",0,100,15,YT_NORMAL,0},
    {"SLAM",80,75,20,YT_NORMAL,0},
    {"THUNDERBOLT",95,100,15,YT_ELECTRIC,1},
    {"HYPER FANG",80,90,15,YT_NORMAL,0},
    {"SLEEP POWDER",0,75,15,YT_GRASS,1},
    {"POISONPOWDER",0,75,35,YT_POISON,0},
    {"STUN SPORE",0,75,30,YT_GRASS,1},
    {"EMBER",40,100,25,YT_FIRE,1}, /* pinned Yellow: BURN_SIDE_EFFECT1 */
    {"STRING SHOT",0,95,40,YT_BUG,0},
    {"HARDEN",0,100,30,YT_NORMAL,0},
    {"LEER",0,100,30,YT_NORMAL,0},
    {0,0,0,0,0,0}, /* id 19 is reserved; protected legacy ABI expects NULL */
    {"SCRATCH",40,100,35,YT_NORMAL,0},  /* original move #10 */
    {"SCREECH",0,85,40,YT_NORMAL,0},   /* original #103 DEFENSE_DOWN2 */
    {"LEECH LIFE",20,100,15,YT_BUG,0}, /* original #141 drain half inflicted */
    {"SUPERSONIC",0,55,20,YT_NORMAL,0}, /* original #48 confusion */
    {"POUND",40,100,35,YT_NORMAL,0}, /* original #1 */
    {"TOXIC",0,85,10,YT_POISON,0}, /* original #92, POISON_EFFECT */
};
const YSpeciesInfo* y_species_info(uint8_t kind) {
    return kind!=13 && kind<sizeof(mons)/sizeof(mons[0])?&mons[kind]:0;
}
const YMoveInfo* y_move_info(uint8_t id) {
    return id!=YM_RESERVED_19 && id<sizeof(moves)/sizeof(moves[0])?&moves[id]:0;
}
static uint8_t calc_stat(uint8_t base,uint8_t level,bool hp) {
    if(level<1)level=1;
    /* Gen1 stat: DV=15; floor(sqrt(StatExp))=0 */
    uint32_t n=((2u*base+15u)*level)/100u;
    n+=hp?(uint32_t)level+10u:5u;
    return n>255u?255u:(uint8_t)n;
}
YBattleStats y_calc_stats(uint8_t kind,uint8_t level) {
    const YSpeciesInfo* p=y_species_info(kind);
    YBattleStats s={0};
    if(!p || !level)return s;
    /* Existing nine native battle species resolve through the same pinned
       151-species source table. This does NOT enable 142 unsupported movesets. */
    static const uint8_t native_dex[]={133,16,19,25,10,11,17,29,32,74,95,50,27};
    if(kind==YK_ZUBAT && y_source_calc_stats(41,level,&s))return s;
    if(kind==YK_CLEFAIRY && y_source_calc_stats(35,level,&s))return s;
    if(kind<sizeof(native_dex) &&
       y_source_calc_stats(native_dex[kind],level,&s))return s;
    s.hp=calc_stat(p->hp,level,true);
    s.attack=calc_stat(p->attack,level,false);
    s.defense=calc_stat(p->defense,level,false);
    s.speed=calc_stat(p->speed,level,false);
    s.special=calc_stat(p->special,level,false);
    return s;
}
uint32_t y_exp_threshold(uint8_t kind,uint8_t level) {
    const YSpeciesInfo* p=y_species_info(kind);
    if(!p || level>100)return 0;
    if(level<=1)return 0;
    uint32_t cube=(uint32_t)level*level*level;
    if(kind==YK_CLEFAIRY)return y_source_growth_threshold(35,level);
    if(kind==YK_GEODUDE||kind==YK_ONIX||
       kind==YK_DIGLETT||kind==YK_SANDSHREW) {
       static const uint8_t original_dex[]={74,95,50,27};
       return y_source_growth_threshold(original_dex[kind-YK_GEODUDE],level);
    }
    if(p->growth==0)return cube; /* medium fast */
    /* medium slow: 6/5*n^3-15*n^2+100*n-140, clamp at zero */
    int32_t v=(int32_t)(6*cube/5u)-(int32_t)(15u*level*level)+100*(int32_t)level-140;
    return (uint32_t)(v>0?v:0);
}
uint16_t y_exp_award(uint8_t kind,uint8_t level,bool trainer) {
    const YSpeciesInfo* p=y_species_info(kind);
    if(!p)return 0;
    uint32_t base=(uint32_t)p->exp_yield*level/7u;
    if(trainer)base=base*3u/2u;
    return base>65535u?65535u:(uint16_t)base;
}
uint8_t y_max_pp(uint8_t move) {
    /* Native only permits its original 19 moves. PP is now sourced from
       immutable pret/pokeyellow move table; all other move IDs stay unusable. */
    const YMoveInfo* m=y_move_info(move);
    if(!m)return 0;
    YSourceMove source;
    if(move==YM_SCRATCH)return y_source_move(10,&source)?source.pp:0;
    if(move==YM_SCREECH)return y_source_move(103,&source)?source.pp:0;
    if(move==YM_LEECH_LIFE)return y_source_move(141,&source)?source.pp:0;
    if(move==YM_SUPERSONIC)return y_source_move(48,&source)?source.pp:0;
    if(move==YM_POUND)return y_source_move(1,&source)?source.pp:0;
    if(move==YM_TOXIC)return y_source_move(92,&source)?source.pp:0;
    return y_source_native_move(move,&source)?source.pp:m->pp;
}
/* Source: pret/pokeyellow data/pokemon/evos_moves.asm.
 * Initial move slots from base_stats; top-out replaces oldest entry.
 * This is a deterministic developer policy. The actual Yellow game opens
 * a move-forgetting dialogue when all four slots are occupied. */
/* Supported original Yellow level evolutions only. Do not invent
   unsupported Butterfree, Raticate, Raichu or Pidgeot species. */
uint8_t y_evolution_at(uint8_t species,uint8_t level) {
    if(species==YK_CATERPIE && level>=7)return YK_METAPOD;
    if(species==YK_PIDGEY && level>=18)return YK_PIDGEOTTO;
    return species;
}

uint8_t y_learned_move_at(uint8_t species,uint8_t level) {
    if(species==YK_PIKACHU) {
        switch(level) {
        case 6:return YM_TAIL_WHIP;
        case 8:return YM_THUNDER_WAVE;
        case 11:return YM_QUICK_ATTACK;
        case 15:return YM_DOUBLE_TEAM;
        case 20:return YM_SLAM;
        case 26:return YM_THUNDERBOLT;
        default:return YM_NONE;
        }
    }
    if(species==YK_PIDGEY) {
        if(level==5)return YM_SAND_ATTACK;
        if(level==12)return YM_QUICK_ATTACK;
    }
    if(species==YK_RATTATA) {
        if(level==7)return YM_QUICK_ATTACK;
        if(level==14)return YM_HYPER_FANG;
    }
    if(species==YK_ZUBAT && level==10)return YM_SUPERSONIC;
    if(species==YK_SANDSHREW && level==10)return YM_SAND_ATTACK;
    if(species==YK_PIDGEOTTO) {
        if(level==5)return YM_SAND_ATTACK;
        if(level==12)return YM_QUICK_ATTACK;
    }
    return YM_NONE;
}
static void moveset_add(uint8_t moves[4],uint8_t move) {
    if(move==YM_NONE)return;
    for(uint8_t i=0;i<4;i++) {
        if(moves[i]==move)return;
        if(moves[i]==YM_NONE) {moves[i]=move;return;}
    }
    for(uint8_t i=0;i<3;i++)moves[i]=moves[i+1];
    moves[3]=move;
}
void y_build_moveset(uint8_t species,uint8_t level,uint8_t out_moves[4]) {
    for(uint8_t i=0;i<4;i++)out_moves[i]=YM_NONE;
    if(species==YK_PIKACHU) {
        moveset_add(out_moves,YM_THUNDERSHOCK);
        moveset_add(out_moves,YM_GROWL);
    } else if(species==YK_EEVEE) {
        moveset_add(out_moves,YM_TACKLE);
        moveset_add(out_moves,YM_TAIL_WHIP);
    } else if(species==YK_PIDGEY) {
        moveset_add(out_moves,YM_GUST);
    } else if(species==YK_RATTATA) {
        moveset_add(out_moves,YM_TACKLE);
        moveset_add(out_moves,YM_TAIL_WHIP);
    } else if(species==YK_CATERPIE) {
        moveset_add(out_moves,YM_TACKLE);
        moveset_add(out_moves,YM_STRING_SHOT);
    } else if(species==YK_METAPOD) {
        moveset_add(out_moves,YM_HARDEN);
    } else if(species==YK_PIDGEOTTO) {
        moveset_add(out_moves,YM_GUST);
        moveset_add(out_moves,YM_SAND_ATTACK);
    } else if(species==YK_NIDORAN_F) {
        moveset_add(out_moves,YM_GROWL);
        moveset_add(out_moves,YM_TACKLE);
    } else if(species==YK_NIDORAN_M) {
        moveset_add(out_moves,YM_LEER);
        moveset_add(out_moves,YM_TACKLE);
    } else if(species==YK_GEODUDE) {
        moveset_add(out_moves,YM_TACKLE); /* Lv10, DEFENSE_CURL only Lv11 */
    } else if(species==YK_ONIX) {
        moveset_add(out_moves,YM_TACKLE);
        moveset_add(out_moves,YM_SCREECH); /* source base_stats/onix.asm */
    } else if(species==YK_CLEFAIRY) {
        moveset_add(out_moves,YM_POUND);
        moveset_add(out_moves,YM_GROWL);
    } else if(species==YK_ZUBAT) {
        moveset_add(out_moves,YM_LEECH_LIFE);
    } else if(species==YK_DIGLETT || species==YK_SANDSHREW) {
        moveset_add(out_moves,YM_SCRATCH); /* source base_stats */
    } else return;
    for(uint8_t n=2;n<=level;n++) {
        uint8_t learned=y_learned_move_at(species,n);
        if(learned!=YM_NONE)moveset_add(out_moves,learned);
        if(n==100)break;
    }
}
bool y_move_available(uint8_t species,uint8_t level,uint8_t move) {
    if(move==YM_NONE)return true;
    if(!y_move_info(move))return false;
    if(species==YK_PIKACHU) {
        if(move==YM_THUNDERSHOCK||move==YM_GROWL)return true;
    } else if(species==YK_EEVEE) {
        if(move==YM_TACKLE||move==YM_TAIL_WHIP)return true;
    } else if(species==YK_PIDGEY) {
        if(move==YM_GUST)return true;
    } else if(species==YK_RATTATA) {
        if(move==YM_TACKLE||move==YM_TAIL_WHIP)return true;
    } else if(species==YK_CATERPIE) {
        if(move==YM_TACKLE||move==YM_STRING_SHOT)return true;
    } else if(species==YK_METAPOD) {
        if(move==YM_HARDEN||move==YM_TACKLE||move==YM_STRING_SHOT)return true;
    } else if(species==YK_PIDGEOTTO) {
        if(move==YM_GUST||move==YM_SAND_ATTACK)return true;
    } else if(species==YK_NIDORAN_F) {
        if(move==YM_GROWL||move==YM_TACKLE)return true;
    } else if(species==YK_NIDORAN_M) {
        if(move==YM_LEER||move==YM_TACKLE)return true;
    } else if(species==YK_GEODUDE) {
        if(move==YM_TACKLE)return true;
    } else if(species==YK_ONIX) {
        if(move==YM_TACKLE||move==YM_SCREECH)return true;
    } else if(species==YK_CLEFAIRY) {
        if(move==YM_POUND||move==YM_GROWL)return true;
    } else if(species==YK_ZUBAT) {
        if(move==YM_LEECH_LIFE)return true;
    } else if(species==YK_DIGLETT||species==YK_SANDSHREW) {
        if(move==YM_SCRATCH)return true;
        if(species==YK_SANDSHREW && level>=10 &&
           move==YM_SAND_ATTACK)return true;
    }
    for(uint8_t i=2;i<=level;i++) {
        if(y_learned_move_at(species,i)==move)return true;
        if(i==100)break;
    }
    return false;
}
uint8_t y_player_move(uint8_t level,uint8_t slot) {
    uint8_t moves[4];
    y_build_moveset(YK_PIKACHU,level,moves);
    return slot<4?moves[slot]:YM_NONE;
}
uint8_t y_player_move_count(uint8_t level) {
    uint8_t moves[4],count=0;
    y_build_moveset(YK_PIKACHU,level,moves);
    for(uint8_t i=0;i<4;i++)if(moves[i]!=YM_NONE)count++;
    return count;
}
static uint16_t stage_factor(uint16_t stat,int8_t stage) {
    /* Gen1 stages: stat * (2+stage)/2 or stat * 2/(2-stage) */
    if(stage>6)stage=6;
    if(stage< -6)stage=-6;
    if(stage>=0)return (uint16_t)(stat*(2+stage)/2);
    return (uint16_t)(stat*2/(2-stage));
}
/* Exact sparse chart from Yellow. Multiplier is %: 0,25,50,100,200,400.
   Ordering of integer truncation in damage is still an approximation. */
uint16_t y_type_effectiveness(uint8_t type,uint8_t target1,uint8_t target2) {
    uint16_t first=10,second=10;
    for(unsigned i=0;i<YELLOW_TYPE_MATCHUP_COUNT;i++) {
        const YTypeMatchup* item=&yellow_type_matchups[i];
        if(item->attack!=type)continue;
        if(item->defend==target1)first=item->multiplier_tenths;
        if(item->defend==target2 && target2!=target1)
            second=item->multiplier_tenths;
    }
    return first*second;
}
uint8_t y_move_priority(uint8_t move) {
    /* Current move catalog: Quick Attack +1, all others 0.
       Gen1 Counter has -1 but is not ported yet. */
    return move==YM_QUICK_ATTACK?1u:0u;
}
bool y_player_moves_first(uint8_t player_move,uint8_t enemy_move,
                          uint16_t player_speed,uint16_t enemy_speed,uint8_t tie_roll) {
    uint8_t player_priority=y_move_priority(player_move);
    uint8_t enemy_priority=y_move_priority(enemy_move);
    if(player_priority!=enemy_priority)return player_priority>enemy_priority;
    if(player_speed!=enemy_speed)return player_speed>enemy_speed;
    /* pret/pokeyellow engine/battle/core.asm .speedEqual:
       BattleRandom < 50 percent + 1 (128): 50% tie-break. */
    return tie_roll<128u;
}
uint8_t y_critical_threshold(uint8_t species,uint8_t move) {
    const YSpeciesInfo* info=y_species_info(species);
    const YMoveInfo* m=y_move_info(move);
    if(!info||!m||!m->power)return 0;
    /* Regular moves only: original Yellow computes base Speed / 2.
       High crit move table exists upstream but none is in our active pool. */
    return info->speed/2u;
}
bool y_roll_critical(uint8_t species,uint8_t move,uint8_t roll) {
    uint8_t threshold=y_critical_threshold(species,move);
    /* CriticalHitTest rotates the RNG byte left three times. */
    uint8_t rotated=(uint8_t)((roll<<3)|(roll>>5));
    return threshold && rotated<threshold;
}
bool y_status_is_sleep(uint8_t status) {
    return status>=YStatusSleep1 && status<=YStatusSleep7;
}
bool y_status_is_toxic(uint8_t status) {
    return status>=YStatusToxic1 && status<=YStatusToxic15;
}
bool y_status_valid(uint8_t status) {
    return status<=YStatusToxic15;
}
uint8_t y_sleep_after_action(uint8_t status) {
    /* Gen1 decrements SLP_MASK and cancels the turn even when waking. */
    if(!y_status_is_sleep(status))return status;
    return status==YStatusSleep1?YStatusNone:(uint8_t)(status-1u);
}
uint8_t y_toxic_after_tick(uint8_t status) {
    if(!y_status_is_toxic(status))return status;
    return status<YStatusToxic15?(uint8_t)(status+1u):status;
}
uint8_t y_residual_damage(uint8_t max_hp,uint8_t status) {
    if(status!=YStatusBurned && status!=YStatusPoisoned &&
       !y_status_is_toxic(status))return 0;
    uint16_t base=(uint16_t)max_hp/16u;
    if(!base)base=1;
    if(y_status_is_toxic(status))base*=(status-YStatusToxic1+1u);
    if(base>255u)base=255u;
    return (uint8_t)base;
}
YDamage y_calc_damage_status_crit(uint8_t source,uint8_t source_level,
                           uint8_t target,uint8_t target_level,
                           uint8_t move,int8_t atk_stage,int8_t def_stage,
                           uint8_t hit_roll,uint8_t rand_roll,bool critical,
                           uint8_t attacker_status) {
    YDamage out={0,0};
    const YSpeciesInfo* a=y_species_info(source);
    const YSpeciesInfo* b=y_species_info(target);
    const YMoveInfo* m=y_move_info(move);
    if(!a||!b||!m||!source_level||!target_level)return out;
    if(!y_move_hits(move,0,0,hit_roll))return out;
    out.hit=1;
    if(!m->power)return out;
    YBattleStats attacker=y_calc_stats(source,source_level);
    YBattleStats defender=y_calc_stats(target,target_level);
    uint16_t atk=m->special?attacker.special:
        (critical?attacker.attack:stage_factor(attacker.attack,atk_stage));
    uint16_t def=m->special?defender.special:
        (critical?defender.defense:stage_factor(defender.defense,def_stage));
    if(!m->special && !critical && attacker_status==YStatusBurned) {
        /* Gen1 GetDamageVarsForPlayerAttack/.physicalAttackCritCheck uses
         * unmodified party Attack for critical hits; burn halves only the
         * live battle Attack used for ordinary physical damage. */
        atk/=2;
        if(!atk)atk=1;
    }
    if(!def)def=1;
    uint32_t level=(uint32_t)source_level*(critical?2u:1u);
    uint32_t damage=((((2u*level/5u+2u)*m->power*atk)/def)/50u)+2u;
    if(m->type==a->type1||m->type==a->type2)damage=damage*3u/2u;
    uint16_t effectiveness=y_type_effectiveness(m->type,b->type1,b->type2);
    if(!effectiveness){out.damage=0;return out;} /* Gen1 immunity */
    damage=damage*effectiveness/100u;
    damage=damage*(217u+(rand_roll%39u))/255u;
    if(!damage)damage=1;
    out.damage=damage>255u?255u:(uint8_t)damage;
    return out;
}
YDamage y_calc_damage_crit(uint8_t source,uint8_t source_level,
                           uint8_t target,uint8_t target_level,
                           uint8_t move,int8_t atk_stage,int8_t def_stage,
                           uint8_t hit_roll,uint8_t rand_roll,bool critical) {
    return y_calc_damage_status_crit(source,source_level,target,target_level,
                                    move,atk_stage,def_stage,hit_roll,rand_roll,
                                    critical,YStatusNone);
}
YDamage y_calc_damage(uint8_t source,uint8_t source_level,
                      uint8_t target,uint8_t target_level,
                      uint8_t move,int8_t atk_stage,int8_t def_stage,
                      uint8_t hit_roll,uint8_t rand_roll) {
    return y_calc_damage_crit(source,source_level,target,target_level,move,
                              atk_stage,def_stage,hit_roll,rand_roll,false);
}

/* Based on pret/pokeyellow engine/items/item_effects.asm ItemUseBall.
   Basic Poke Ball, no statuses and no Great/Ultra Ball adjustments.
   Original two RNG rolls and integer truncation. */
uint8_t y_species_catch_rate(uint8_t species) {
    switch(species) {
    case YK_PIKACHU:return 190;
    case YK_EEVEE:return 45;
    case YK_PIDGEY:case YK_RATTATA:case YK_CATERPIE:return 255;
    case YK_METAPOD:case YK_PIDGEOTTO:return 120;
    case YK_NIDORAN_F:case YK_NIDORAN_M:return 235;
    case YK_GEODUDE:return 255;
    case YK_ONIX:return 45;
    case YK_DIGLETT:case YK_SANDSHREW:case YK_ZUBAT:return 255;
    case YK_CLEFAIRY:return 150;
    default:return 0;
    }
}
bool y_gen1_pokeball_capture(uint8_t rate,uint8_t max_hp,
                             uint8_t hp,uint8_t r1,uint8_t r2) {
    if(!rate||!max_hp||!hp||hp>max_hp)return false;
    if(r1>rate)return false;
    uint32_t divisor=hp/4u;
    if(!divisor)divisor=1;
    uint32_t x=((uint32_t)max_hp*255u/12u)/divisor;
    return x>255u || r2<=x;
}


uint8_t y_wild_move(uint8_t kind,uint8_t level,uint8_t slot) {
    uint8_t moves[4];
    y_build_moveset(kind,level,moves);
    return slot<4?moves[slot]:YM_NONE;
}

/* Gen1 staged accuracy / evasion ratios (data/battle/stat_modifiers.asm).
   Converts percentage moves to 0..255 thresholds; 255 roll always misses. */
static const uint16_t hit_ratio_num[13]={
    25,28,33,40,50,66,100,150,200,250,300,350,400
};
static int8_t capped_stage(int8_t stage) {
    if(stage< -6)return -6;
    if(stage>6)return 6;
    return stage;
}
uint8_t y_move_accuracy_threshold(uint8_t id,int8_t accuracy_stage,int8_t evasion_stage) {
    const YMoveInfo* move=y_move_info(id);
    if(!move)return 0;
    uint32_t chance=(uint32_t)move->accuracy*255u/100u;
    int a=capped_stage(accuracy_stage)+6;
    int e=6-capped_stage(evasion_stage);
    chance=chance*hit_ratio_num[a]/100u;
    chance=chance*hit_ratio_num[e]/100u;
    if(chance<1u)chance=1u;
    if(chance>255u)chance=255u;
    return (uint8_t)chance;
}
bool y_move_hits(uint8_t move,int8_t accuracy_stage,int8_t evasion_stage,uint8_t roll) {
    return roll<y_move_accuracy_threshold(move,accuracy_stage,evasion_stage);
}
bool y_thundershock_paralyzes(uint8_t roll,uint8_t target_status) {
    /* PARALYZE_SIDE_EFFECT1: 10 percent + 1 = 26/256 */
    return target_status==YStatusNone && roll<26u;
}
bool y_full_paralysis(uint8_t roll) {
    return roll<63u; /* Gen1 25 percent macro = floor(25*255/100) = 63 */
}
uint16_t y_effective_speed(uint8_t species,uint8_t level,uint8_t status) {
    uint16_t s=y_calc_stats(species,level).speed;
    if(status==YStatusParalyzed)s/=4u;
    return s;
}

bool y_gen1_pokeball_capture_status(uint8_t rate,uint8_t max_hp,
    uint8_t hp,uint8_t r1,uint8_t r2,uint8_t status) {
    if(!rate || !max_hp || !hp || hp>max_hp)return false;
    if(status==YStatusParalyzed || status==YStatusBurned ||
       status==YStatusPoisoned || y_status_is_toxic(status)) {
        /* Gen1 ItemUseBall: Burn/Paralysis/Poison subtract 12 from Rand1. */
        if(r1<12u)return true;
        r1=(uint8_t)(r1-12u);
    }
    if(status==YStatusFrozen || y_status_is_sleep(status)) {
        /* Gen1 Sleep/Freeze: Status=25, distinct from 12 status classes. */
        if(r1<25u)return true;
        r1=(uint8_t)(r1-25u);
    }
    return y_gen1_pokeball_capture(rate,max_hp,hp,r1,r2);
}
