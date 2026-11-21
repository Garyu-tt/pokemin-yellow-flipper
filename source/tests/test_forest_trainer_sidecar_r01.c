#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
int main(void){
 YellowGame g;yellow_init(&g);
 g.story=YStoryPikachuFollows;g.map=YMapViridianForest;g.screen=YScreenWorld;
 g.has_pikachu=1;g.x=29;g.y=33;g.forest_defeated_mask=3;
 YellowSave parent;yellow_save_create(&g,&parent);
 assert(parent.version==13);
 YellowTrainerSave side;yellow_trainer_save_create(&g,&parent,&side);
 assert(side.magic==FOREST_TRAINER_SAVE_MAGIC);
 YellowGame loaded;assert(yellow_save_load(&loaded,&parent));
 assert(loaded.forest_defeated_mask==0);
 assert(yellow_trainer_save_load(&loaded,&parent,&side));
 assert(loaded.forest_defeated_mask==3);
 YellowTrainerSave tampered=side;tampered.defeated_mask=7;
 assert(!yellow_trainer_save_load(&loaded,&parent,&tampered));
 tampered=side;tampered.parent_checksum++;
 assert(!yellow_trainer_save_load(&loaded,&parent,&tampered));
 tampered=side;tampered.version=2;
 assert(!yellow_trainer_save_load(&loaded,&parent,&tampered));
 tampered=side;tampered.reserved=1;
 assert(!yellow_trainer_save_load(&loaded,&parent,&tampered));
 YellowSave other=parent;other.checksum++;
 assert(!yellow_trainer_save_load(&loaded,&other,&side));
 puts("FOREST_TRAINER_SIDECAR_CHECKSUM_LEGACY_SAVE_PASS");
 return 0;
}
