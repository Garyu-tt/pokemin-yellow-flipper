#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "yellow_core.h"
#include "../yellow_assets.c"
#include "../yellow_battle.c"
#include "../yellow_core.c"
#include "../pydp_reader.c"
#include "../yellow_bulk_data.c"

static int read_at(void*ctx,uint32_t pos,uint8_t*dst,size_t len) {
 FILE *f=(FILE*)ctx;
 /* Exactly the bounded I/O contract of the real Flipper adapter. */
 if(!len || len>128 || fseek(f,(long)pos,SEEK_SET))return 0;
 return fread(dst,1,len,f)==len;
}
int main(void) {
 FILE *f=fopen("../../../bulk-pokeyellow-r01/pokeyellow_data.pydb","rb");
 assert(f);
 PydpPack pack;assert(pydp_open(&pack,read_at,f));
 assert(pydp_verify(&pack));
 YellowBulkWild imported;assert(yellow_bulk_prepare(&pack,&imported));
 assert(imported.route1.rate==25 && imported.forest.rate==25);
 assert(imported.route1.kind[0]==YK_PIDGEY && imported.route1.level[0]==3);
 assert(imported.route1.kind[9]==YK_PIDGEY && imported.route1.level[9]==7);
 assert(imported.forest.kind[0]==YK_CATERPIE && imported.forest.level[0]==3);
 assert(imported.forest.kind[9]==YK_PIDGEOTTO && imported.forest.level[9]==9);

 YellowGame g;yellow_init(&g);
 uint8_t species=255,level=0;
 assert(yellow_wild_slot(&g,YMapRoute1,0,&species,&level));
 assert(species==YK_PIDGEY && level==3);
 /* Missing pack keeps the previously accepted encounter arrays. */
 assert(yellow_attach_bulk_wild(&g,NULL));
 assert(yellow_wild_slot(&g,YMapViridianForest,9,&species,&level));
 assert(species==YK_PIDGEOTTO && level==9);

 assert(yellow_attach_bulk_wild(&g,&imported));
 assert(g.imported_wild==&imported);
 /* Exercise the exact lookup function used by actual wild battles. */
 imported.route1.kind[0]=YK_RATTATA;imported.route1.level[0]=6;
 assert(yellow_wild_slot(&g,YMapRoute1,0,&species,&level));
 assert(species==YK_RATTATA && level==6);
 imported.forest.kind[9]=YK_CATERPIE;imported.forest.level[9]=4;
 assert(yellow_wild_slot(&g,YMapViridianForest,9,&species,&level));
 assert(species==YK_CATERPIE && level==4);
 assert(!yellow_wild_slot(&g,YMapPallet,0,&species,&level));
 assert(!yellow_wild_slot(&g,YMapRoute1,10,&species,&level));

 /* Reject unsupported IDs, absurd levels, invalid encounter rates. */
 YellowBulkWild bad=imported;bad.forest.kind[2]=255;
 assert(!yellow_attach_bulk_wild(&g,&bad) && g.imported_wild==NULL);
 bad=imported;bad.route1.level[1]=101;
 assert(!yellow_attach_bulk_wild(&g,&bad));
 bad=imported;bad.route1.rate=0;
 assert(!yellow_attach_bulk_wild(&g,&bad));
 assert(yellow_wild_slot(&g,YMapRoute1,0,&species,&level));
 assert(species==YK_PIDGEY && level==3);

 YellowSave parent;yellow_save_create(&g,&parent);
 YellowGame restored;assert(yellow_save_load(&restored,&parent));
 assert(restored.imported_wild==NULL && parent.version==13);
 assert(yellow_attach_bulk_wild(&restored,&imported));
 assert(yellow_wild_slot(&restored,YMapRoute1,0,&species,&level));
 assert(species==YK_RATTATA && level==6);
 fclose(f);
 puts("PACK_STREAM_WILD_SELECTION_FALLBACK_SAVE_R13_PASS");
 return 0;
}
