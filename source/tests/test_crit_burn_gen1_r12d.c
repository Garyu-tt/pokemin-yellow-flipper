#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"

/* Pinned pret/pokeyellow engine/battle/core.asm:
 * GetDamageVarsForPlayerAttack .physicalAttackCritCheck selects
 * wPartyMon1Attack (unmodified) on critical, whereas
 * HalveAttackDueToBurn reduces wBattleMonAttack (modified).
 * Therefore a critical physical hit ignores burn's Attack penalty.
 * Both cases must be checked: crit unaffected, noncrit penalized.
 */
static uint8_t physical(uint8_t species,uint8_t foe,bool crit,
                        uint8_t status,int8_t atk_stage,int8_t def_stage) {
    return y_calc_damage_status_crit(species,45,foe,45,
        YM_QUICK_ATTACK,atk_stage,def_stage,0,38,crit,status).damage;
}
static void player_and_enemy_crit_ignore_burn(void) {
    const uint8_t species[][2]={{YK_PIKACHU,YK_RATTATA},
                                 {YK_RATTATA,YK_PIKACHU}};
    for(unsigned i=0;i<2;i++) {
        uint8_t a=species[i][0],b=species[i][1];
        uint8_t ordinary=physical(a,b,false,YStatusNone,0,0);
        uint8_t burned=physical(a,b,false,YStatusBurned,0,0);
        uint8_t crit=physical(a,b,true,YStatusNone,0,0);
        uint8_t crit_burn=physical(a,b,true,YStatusBurned,0,0);
        assert(ordinary>0 && burned>0 && crit>0);
        assert(burned<ordinary); /* negative control: burn still matters */
        assert(crit_burn==crit); /* source parity: critical ignores burn */
        assert(physical(a,b,true,YStatusBurned,-6,+6)==crit);
        assert(physical(a,b,true,YStatusBurned,+6,-6)==crit);
    }
    puts("GEN1_CRIT_BURN_IGNORED_FOR_BOTH_SIDES_PASS");
}
static void special_and_regular_attack_guards(void) {
    uint8_t neutral=y_calc_damage_status_crit(YK_PIKACHU,45,YK_RATTATA,45,
        YM_THUNDERSHOCK,0,0,0,38,true,YStatusNone).damage;
    uint8_t burned=y_calc_damage_status_crit(YK_PIKACHU,45,YK_RATTATA,45,
        YM_THUNDERSHOCK,0,0,0,38,true,YStatusBurned).damage;
    assert(neutral>0 && burned==neutral);
    assert(physical(YK_PIKACHU,YK_RATTATA,false,YStatusBurned,-6,0)<
           physical(YK_PIKACHU,YK_RATTATA,false,YStatusNone,0,0));
    puts("GEN1_NONCRIT_BURN_AND_SPECIAL_GUARDS_PASS");
}
int main(void) {
    player_and_enemy_crit_ignore_burn();
    special_and_regular_attack_guards();
    puts("GEN1_CRITICAL_BURN_REGRESSION_PASS");
    return 0;
}
