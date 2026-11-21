#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"

/* Independent behavior regression: reproduce Oak's scripted lab-door guard.
   No modification of protected historical tests. */
static void setup(YellowGame* g,YellowStory story,uint8_t pikachu) {
    yellow_init(g);
    g->map=YMapOaksLab;
    g->x=4;g->y=10;
    g->story=story;g->has_pikachu=pikachu;
    g->screen=YScreenWorld;g->pending=YPendingNone;
    g->follower_valid=0;
    g->facing=YDown;
}
static void dismiss(YellowGame* g) {
    assert(g->screen==YScreenDialog);
    for(unsigned i=0,n=g->dialog_count;i<n;i++)yellow_input(g,YOk);
}
static void block_before_starter(void) {
    YellowGame g;setup(&g,YStoryChooseStarter,0);
    const uint16_t steps=g.steps;
    for(unsigned tries=0;tries<3;tries++) {
        yellow_input(&g,YDown);
        assert(g.map==YMapOaksLab && g.x==4 && g.y==10);
        assert(g.story==YStoryChooseStarter && g.has_pikachu==0);
        assert(g.steps==steps && g.pending==YPendingNone);
        assert(g.screen==YScreenDialog && g.dialog_count==1);
        assert(g.dialog_lines[0] && g.dialog_lines[1]);
        dismiss(&g);
        assert(g.screen==YScreenWorld && g.story==YStoryChooseStarter);
    }
    /* Dialog interaction cannot bypass the door guard by pressing Back. */
    yellow_input(&g,YDown);
    assert(g.screen==YScreenDialog);
    yellow_input(&g,YBack);
    assert(g.screen==YScreenWorld && g.map==YMapOaksLab);
    assert(g.x==4 && g.y==10 && g.steps==steps);
    YellowSave save;yellow_save_create(&g,&save);
    YellowGame restored;assert(yellow_save_load(&restored,&save));
    assert(restored.story==YStoryChooseStarter);
    assert(restored.map==YMapOaksLab && restored.x==4 && restored.y==10);
    yellow_input(&restored,YDown);
    assert(restored.screen==YScreenDialog);
    assert(restored.map==YMapOaksLab && restored.steps==steps);
    puts("R14_OAK_EXIT_BLOCK_AND_RELOAD_PASS");
}
static void allow_exit_before_and_after(void) {
    YellowGame g;
    setup(&g,YStoryBeforeOak,0);
    yellow_input(&g,YDown);
    assert(g.screen==YScreenWorld && g.map==YMapPallet);
    assert(g.x==12 && g.y==12 && g.steps==1);
    setup(&g,YStoryPikachuFollows,1);
    yellow_input(&g,YDown);
    assert(g.screen==YScreenWorld && g.map==YMapPallet);
    assert(g.x==12 && g.y==12 && g.steps==1);
    puts("R14_OAK_EXIT_NO_FALSE_BLOCK_PASS");
}
int main(void) {
    block_before_starter();
    allow_exit_before_and_after();
    puts("R14_OAK_EXIT_ALL_PASS");
    return 0;
}
