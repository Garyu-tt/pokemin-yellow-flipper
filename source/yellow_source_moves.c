#include "yellow_source_moves.h"
/* Source pret/pokeyellow data/moves/moves.asm, pinned git revision.
   Most effect ids have no native execution implementation yet. */
static const YSourceMove source_moves[165]={
 {1,40,0,100,35,0}, /* POUND */
 {2,50,0,100,25,0}, /* KARATE_CHOP */
 {3,15,0,85,10,29}, /* DOUBLESLAP */
 {4,18,0,85,15,29}, /* COMET_PUNCH */
 {5,80,0,85,20,0}, /* MEGA_PUNCH */
 {6,40,0,100,20,16}, /* PAY_DAY */
 {7,75,20,100,15,4}, /* FIRE_PUNCH */
 {8,75,25,100,15,5}, /* ICE_PUNCH */
 {9,75,23,100,15,6}, /* THUNDERPUNCH */
 {10,40,0,100,35,0}, /* SCRATCH */
 {11,55,0,100,30,0}, /* VICEGRIP */
 {12,1,0,30,5,38}, /* GUILLOTINE */
 {13,80,0,75,10,39}, /* RAZOR_WIND */
 {14,0,0,100,30,50}, /* SWORDS_DANCE */
 {15,50,0,95,30,0}, /* CUT */
 {16,40,0,100,35,0}, /* GUST */
 {17,35,2,100,35,0}, /* WING_ATTACK */
 {18,0,0,85,20,28}, /* WHIRLWIND */
 {19,70,2,95,15,43}, /* FLY */
 {20,15,0,75,20,42}, /* BIND */
 {21,80,0,75,20,0}, /* SLAM */
 {22,35,22,100,10,0}, /* VINE_WHIP */
 {23,65,0,100,20,37}, /* STOMP */
 {24,30,1,100,30,44}, /* DOUBLE_KICK */
 {25,120,0,75,5,0}, /* MEGA_KICK */
 {26,70,1,95,25,45}, /* JUMP_KICK */
 {27,60,1,85,15,37}, /* ROLLING_KICK */
 {28,0,0,100,15,22}, /* SAND_ATTACK */
 {29,70,0,100,15,37}, /* HEADBUTT */
 {30,65,0,100,25,0}, /* HORN_ATTACK */
 {31,15,0,85,20,29}, /* FURY_ATTACK */
 {32,1,0,30,5,38}, /* HORN_DRILL */
 {33,35,0,95,35,0}, /* TACKLE */
 {34,85,0,100,15,36}, /* BODY_SLAM */
 {35,15,0,85,20,42}, /* WRAP */
 {36,90,0,85,20,48}, /* TAKE_DOWN */
 {37,90,0,100,20,27}, /* THRASH */
 {38,100,0,100,15,48}, /* DOUBLE_EDGE */
 {39,0,0,100,30,19}, /* TAIL_WHIP */
 {40,15,3,100,35,2}, /* POISON_STING */
 {41,25,7,100,20,77}, /* TWINEEDLE */
 {42,14,7,85,20,29}, /* PIN_MISSILE */
 {43,0,0,100,30,19}, /* LEER */
 {44,60,0,100,25,31}, /* BITE */
 {45,0,0,100,40,18}, /* GROWL */
 {46,0,0,100,20,28}, /* ROAR */
 {47,0,0,55,15,32}, /* SING */
 {48,0,0,55,20,49}, /* SUPERSONIC */
 {49,1,0,90,20,41}, /* SONICBOOM */
 {50,0,0,55,20,86}, /* DISABLE */
 {51,40,3,100,30,69}, /* ACID */
 {52,40,20,100,25,4}, /* EMBER */
 {53,95,20,100,15,4}, /* FLAMETHROWER */
 {54,0,25,100,30,46}, /* MIST */
 {55,40,21,100,25,0}, /* WATER_GUN */
 {56,120,21,80,5,0}, /* HYDRO_PUMP */
 {57,95,21,100,15,0}, /* SURF */
 {58,95,25,100,10,5}, /* ICE_BEAM */
 {59,120,25,90,5,5}, /* BLIZZARD */
 {60,65,24,100,20,76}, /* PSYBEAM */
 {61,65,21,100,20,70}, /* BUBBLEBEAM */
 {62,65,25,100,20,68}, /* AURORA_BEAM */
 {63,150,0,90,5,80}, /* HYPER_BEAM */
 {64,35,2,100,35,0}, /* PECK */
 {65,80,2,100,20,0}, /* DRILL_PECK */
 {66,80,1,80,25,48}, /* SUBMISSION */
 {67,50,1,90,20,37}, /* LOW_KICK */
 {68,1,1,100,20,0}, /* COUNTER */
 {69,1,1,100,20,41}, /* SEISMIC_TOSS */
 {70,80,0,100,15,0}, /* STRENGTH */
 {71,20,22,100,20,3}, /* ABSORB */
 {72,40,22,100,10,3}, /* MEGA_DRAIN */
 {73,0,22,90,10,84}, /* LEECH_SEED */
 {74,0,0,100,40,13}, /* GROWTH */
 {75,55,22,95,25,0}, /* RAZOR_LEAF */
 {76,120,22,100,10,39}, /* SOLARBEAM */
 {77,0,3,75,35,66}, /* POISONPOWDER */
 {78,0,22,75,30,67}, /* STUN_SPORE */
 {79,0,22,75,15,32}, /* SLEEP_POWDER */
 {80,70,22,100,20,27}, /* PETAL_DANCE */
 {81,0,7,95,40,20}, /* STRING_SHOT */
 {82,1,26,100,10,41}, /* DRAGON_RAGE */
 {83,15,20,70,15,42}, /* FIRE_SPIN */
 {84,40,23,100,30,6}, /* THUNDERSHOCK */
 {85,95,23,100,15,6}, /* THUNDERBOLT */
 {86,0,23,100,20,67}, /* THUNDER_WAVE */
 {87,120,23,70,10,6}, /* THUNDER */
 {88,50,5,65,15,0}, /* ROCK_THROW */
 {89,100,4,100,10,0}, /* EARTHQUAKE */
 {90,1,4,30,5,38}, /* FISSURE */
 {91,100,4,100,10,39}, /* DIG */
 {92,0,3,85,10,66}, /* TOXIC */
 {93,50,24,100,25,76}, /* CONFUSION */
 {94,90,24,100,10,71}, /* PSYCHIC_M */
 {95,0,24,60,20,32}, /* HYPNOSIS */
 {96,0,24,100,40,10}, /* MEDITATE */
 {97,0,24,100,30,52}, /* AGILITY */
 {98,40,0,100,30,0}, /* QUICK_ATTACK */
 {99,20,0,100,20,81}, /* RAGE */
 {100,0,24,100,20,28}, /* TELEPORT */
 {101,0,8,100,15,41}, /* NIGHT_SHADE */
 {102,0,0,100,10,82}, /* MIMIC */
 {103,0,0,85,40,59}, /* SCREECH */
 {104,0,0,100,15,15}, /* DOUBLE_TEAM */
 {105,0,0,100,20,56}, /* RECOVER */
 {106,0,0,100,30,11}, /* HARDEN */
 {107,0,0,100,20,15}, /* MINIMIZE */
 {108,0,0,100,20,22}, /* SMOKESCREEN */
 {109,0,8,100,10,49}, /* CONFUSE_RAY */
 {110,0,21,100,40,11}, /* WITHDRAW */
 {111,0,0,100,40,11}, /* DEFENSE_CURL */
 {112,0,24,100,30,51}, /* BARRIER */
 {113,0,24,100,30,64}, /* LIGHT_SCREEN */
 {114,0,25,100,30,25}, /* HAZE */
 {115,0,24,100,20,65}, /* REFLECT */
 {116,0,0,100,30,47}, /* FOCUS_ENERGY */
 {117,0,0,100,10,26}, /* BIDE */
 {118,0,0,100,10,83}, /* METRONOME */
 {119,0,2,100,20,9}, /* MIRROR_MOVE */
 {120,130,0,100,5,7}, /* SELFDESTRUCT */
 {121,100,0,75,10,0}, /* EGG_BOMB */
 {122,20,8,100,30,36}, /* LICK */
 {123,20,3,70,20,33}, /* SMOG */
 {124,65,3,100,20,33}, /* SLUDGE */
 {125,65,4,85,20,31}, /* BONE_CLUB */
 {126,120,20,85,5,34}, /* FIRE_BLAST */
 {127,80,21,100,15,0}, /* WATERFALL */
 {128,35,21,75,10,42}, /* CLAMP */
 {129,60,0,100,20,17}, /* SWIFT */
 {130,100,0,100,15,39}, /* SKULL_BASH */
 {131,20,0,100,15,29}, /* SPIKE_CANNON */
 {132,10,0,100,35,70}, /* CONSTRICT */
 {133,0,24,100,20,53}, /* AMNESIA */
 {134,0,24,80,15,22}, /* KINESIS */
 {135,0,0,100,10,56}, /* SOFTBOILED */
 {136,85,1,90,20,45}, /* HI_JUMP_KICK */
 {137,0,0,75,30,67}, /* GLARE */
 {138,100,24,100,15,8}, /* DREAM_EATER */
 {139,0,3,55,40,66}, /* POISON_GAS */
 {140,15,0,85,20,29}, /* BARRAGE */
 {141,20,7,100,15,3}, /* LEECH_LIFE */
 {142,0,0,75,10,32}, /* LOVELY_KISS */
 {143,140,2,90,5,39}, /* SKY_ATTACK */
 {144,0,0,100,10,57}, /* TRANSFORM */
 {145,20,21,100,30,70}, /* BUBBLE */
 {146,70,0,100,10,0}, /* DIZZY_PUNCH */
 {147,0,22,100,15,32}, /* SPORE */
 {148,0,0,70,20,22}, /* FLASH */
 {149,1,24,80,15,41}, /* PSYWAVE */
 {150,0,0,100,40,85}, /* SPLASH */
 {151,0,3,100,40,51}, /* ACID_ARMOR */
 {152,90,21,85,10,0}, /* CRABHAMMER */
 {153,170,0,100,5,7}, /* EXPLOSION */
 {154,18,0,80,15,29}, /* FURY_SWIPES */
 {155,50,4,90,10,44}, /* BONEMERANG */
 {156,0,24,100,10,56}, /* REST */
 {157,75,5,90,10,0}, /* ROCK_SLIDE */
 {158,80,0,90,15,31}, /* HYPER_FANG */
 {159,0,0,100,30,10}, /* SHARPEN */
 {160,0,0,100,30,24}, /* CONVERSION */
 {161,80,0,100,10,0}, /* TRI_ATTACK */
 {162,1,0,90,10,40}, /* SUPER_FANG */
 {163,70,0,100,20,0}, /* SLASH */
 {164,0,0,100,10,79}, /* SUBSTITUTE */
 {165,50,0,100,10,48}, /* STRUGGLE */
};
static const uint8_t native_move_ids[19]={84,45,39,33,16,98,28,86,104,21,85,158,79,77,78,52,81,106,43};
bool y_source_move(uint8_t original_id,YSourceMove* out) {
 if(!out||original_id<1||original_id>165)return false;
 *out=source_moves[original_id-1];return out->id==original_id;
}
bool y_source_native_move(uint8_t native_index,YSourceMove* out) {
 if(native_index>=sizeof(native_move_ids))return false;
 return y_source_move(native_move_ids[native_index],out);
}
uint8_t y_source_supported_native_moves(void){return sizeof(native_move_ids);}
bool y_source_native_index(uint8_t original_id,uint8_t* native_index) {
 if(!native_index || original_id==0 || original_id>165)return false;
 for(uint8_t i=0;i<sizeof(native_move_ids);i++) {
  if(native_move_ids[i]==original_id){*native_index=i;return true;}
 }
 return false;
}
