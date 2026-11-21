#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
#include "../pydp_reader.c"
#include "../yellow_bulk_data.c"

static int read_at(void* context,uint32_t offset,uint8_t* buffer,size_t size) {
 FILE* f=(FILE*)context;
 if(!size || size>128 || fseek(f,(long)offset,SEEK_SET))return 0;
 return fread(buffer,1,size,f)==size;
}
static void ready(YellowGame* g) {
 yellow_init(g);g->story=YStoryPikachuFollows;g->screen=YScreenWorld;
 g->map=YMapViridianForest;g->x=1;g->y=41;g->facing=YRight;
 g->has_pikachu=1;g->hp=yellow_player_max_hp(g);g->follower_valid=0;
}
int main(void) {
 FILE* file=fopen("../../../bulk-pokeyellow-r01/pokeyellow_data.pydb","rb");
 assert(file);
 PydpPack source;assert(pydp_open(&source,read_at,file));assert(pydp_verify(&source));
 YellowBulkLass original;
 assert(yellow_bulk_prepare_lass(&source,&original));
 assert(original.count==2 && original.kind[0]==YK_NIDORAN_F);
 assert(original.kind[1]==YK_NIDORAN_M);
 assert(original.level[0]==6 && original.level[1]==6);
 assert(y_species_info(YK_NIDORAN_F)->hp==55 &&
        y_species_info(YK_NIDORAN_F)->exp_yield==59);
 assert(y_species_info(YK_NIDORAN_M)->attack==57 &&
        y_species_info(YK_NIDORAN_M)->exp_yield==60);
 uint8_t moves[4];y_build_moveset(YK_NIDORAN_F,6,moves);
 assert(moves[0]==YM_GROWL && moves[1]==YM_TACKLE);
 y_build_moveset(YK_NIDORAN_M,6,moves);
 assert(moves[0]==YM_LEER && moves[1]==YM_TACKLE);
 assert(y_species_catch_rate(YK_NIDORAN_M)==235);
 YellowGame g;ready(&g);
 /* Lass STAY/NONE: unlike Bug Catchers, no line-of-sight attack. */
 g.x=4;g.y=41;
 assert(!forest_trainer_sight(&g));
 g.x=1;g.y=41;
 assert(yellow_attach_bulk_lass(&g,&original));
 assert(g.imported_lass==&original);
 yellow_input(&g,YOk);
 assert(g.screen==YScreenDialog &&
        g.pending==YPendingBeginForestLassBattle);
 yellow_input(&g,YOk);
 assert(g.screen==YScreenRivalBattle);
 assert(g.forest_active_trainer==4 && g.foe_kind==YK_NIDORAN_F &&
        g.foe_level==6 && g.forest_defeated_mask==0);
 /* Authentic two-mon trainer, victory only after second faint. */
 finalize_battle(&g,true);
 assert(g.screen==YScreenRivalBattle &&
        g.forest_active_trainer==4 && g.forest_party_index==1);
 assert(g.foe_kind==YK_NIDORAN_M && g.foe_level==6);
 assert((g.forest_defeated_mask&16)==0);
 finalize_battle(&g,true);
 assert(g.forest_active_trainer==255 && g.screen==YScreenDialog &&
        (g.forest_defeated_mask&16));
 yellow_input(&g,YOk);
 assert(g.screen==YScreenWorld);
 /* Save v13 remains byte-compatible and the trainer sidecar now uses bit 4. */
 YellowSave parent;yellow_save_create(&g,&parent);
 YellowTrainerSave saved;yellow_trainer_save_create(&g,&parent,&saved);
 assert(parent.version==13 && saved.version==1 && saved.defeated_mask==16);
 YellowGame reload;assert(yellow_save_load(&reload,&parent));
 assert(reload.imported_lass==NULL);
 assert(yellow_trainer_save_load(&reload,&parent,&saved));
 assert(yellow_attach_bulk_lass(&reload,&original));
 reload.map=YMapViridianForest;reload.x=1;reload.y=41;
 reload.facing=YRight;reload.screen=YScreenWorld;
 yellow_input(&reload,YOk);
 assert(reload.screen==YScreenDialog &&
        reload.dialog_lines==forest_lass_after);
 yellow_input(&reload,YOk);
 assert(reload.screen==YScreenWorld && reload.forest_active_trainer==255);
 YellowTrainerSave forged=saved;forged.defeated_mask=32;
 assert(!yellow_trainer_save_load(&reload,&parent,&forged));
 /* Old 4-Bug-Catcher sidecar from v021 still loads unmodified. */
 YellowGame old;ready(&old);old.forest_defeated_mask=15;
 YellowSave old_parent;yellow_save_create(&old,&old_parent);
 YellowTrainerSave old_sidecar;
 yellow_trainer_save_create(&old,&old_parent,&old_sidecar);
 assert(old_sidecar.defeated_mask==15);
 YellowGame old_reloaded;assert(yellow_save_load(&old_reloaded,&old_parent));
 assert(yellow_trainer_save_load(&old_reloaded,&old_parent,&old_sidecar));
 assert(old_reloaded.forest_defeated_mask==15);
 /* Missing/invalid imported party must never permit invented species. */
 assert(yellow_attach_bulk_lass(&reload,NULL));
 assert(reload.imported_lass==NULL);
 YellowBulkLass tampered=original;tampered.level[0]=99;
 assert(!yellow_attach_bulk_lass(&reload,&tampered) &&
        reload.imported_lass==NULL);
 fclose(file);
 puts("FOREST_LASS_ORIGINAL_PYDP_TEAM_DIALOG_BATTLE_SAVE_PASS");
 return 0;
}
