#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_antidote_cure.c"
#include "../yellow_core.c"
#include "../yellow_antidote_game.c"
#include "../yellow_pewter_progress.c"
#include "../yellow_atomic_snapshot.c"
#include "../yellow_exact_position_v1.c"
int main(void) {
    YellowGame g;yellow_init(&g);
    g.has_pikachu=1;g.hp=10;g.pika_status=YStatusPoisoned;
    uint8_t stock=2;
    assert(yellow_game_apply_antidote(&g,0,&stock));
    assert(g.pika_status==YStatusNone && stock==1 && g.hp==10);
    assert(!yellow_game_apply_antidote(&g,0,&stock) && stock==1);
    g.pika_status=YStatusToxic15;
    assert(yellow_game_apply_antidote(&g,0,&stock));
    assert(g.pika_status==YStatusNone && stock==0);
    g.pika_status=YStatusToxic1;
    assert(!yellow_game_apply_antidote(&g,0,&stock));
    assert(g.pika_status==YStatusToxic1 && stock==0);
    stock=3;g.hp=0;
    assert(!yellow_game_apply_antidote(&g,0,&stock));
    assert(g.pika_status==YStatusToxic1 && stock==3);
    g.hp=10;
    assert(!yellow_game_apply_antidote(&g,1,&stock));
    assert(!yellow_game_apply_antidote(NULL,0,&stock));
    assert(!yellow_game_apply_antidote(&g,0,NULL));
    assert(g.pika_status==YStatusToxic1 && stock==3);
    g.caught_count=1;g.caught[0].species=YK_PIDGEY;
    g.caught[0].level=5;g.caught[0].hp=3;g.caught[0].status=YStatusPoisoned;
    assert(yellow_game_apply_antidote(&g,1,&stock));
    assert(g.caught[0].status==YStatusNone && g.pika_status==YStatusToxic1);
    assert(g.caught[0].hp==3 && stock==2);
    g.caught[0].status=YStatusFrozen;
    assert(!yellow_game_apply_antidote(&g,1,&stock));
    assert(g.caught[0].status==YStatusFrozen && stock==2);
    puts("R012C3_ANTIDOTE_PARTY_PIKACHU_CAUGHT_TOXIC_NO_EFFECT_ATOMIC_PASS_PARTIAL");
    return 0;
}
