/* Host-only fixture producer: valid v1/v3/v4 parents using real codec. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../yellow_core.h"
#include "../yellow_pewter_progress.h"
#include "../yellow_antidote_inventory_slot.h"
#define YELLOW_ATOMIC_ALLOW_V4 1
#include "../yellow_atomic_snapshot.c"
static int write_blob(const char* path,const void* bytes,size_t n) {
    FILE* f=fopen(path,"wb");
    if(!f)return 1;
    int bad=fwrite(bytes,1,n,f)!=n;
    if(fclose(f))bad=1;
    return bad;
}
int main(int argc,char** argv) {
    if(argc!=5)return 2;
    unsigned version=(unsigned)atoi(argv[2]);
    unsigned with_item=(unsigned)atoi(argv[3]);
    unsigned slot=(unsigned)atoi(argv[4]);
    if((version!=1&&version!=3&&version!=4) || with_item>1 || slot>1)return 2;
    YellowGame g;
    yellow_init(&g);
    g.running=true;
    g.screen=YScreenWorld;
    g.story=YStoryPikachuFollows;
    g.has_pikachu=1;
    g.map=YMapRoute1;
    g.x=10;g.y=10;
    g.money_bcd[0]=0x12;g.money_bcd[1]=0x34;g.money_bcd[2]=0x56;
    g.antidotes=2;
    YellowAtomicSnapshot s={0};
    if(!yellow_atomic_make(&g,41u,&s))return 3;
    s.version=version;
    s.checksum=yellow_atomic_checksum(&s);
    if(!yellow_atomic_unpack(&s,NULL))return 4;
    char path[1024];
    snprintf(path,sizeof(path),"%s/snapshot_gen_%c_v01.dat",argv[1],slot?'b':'a');
    if(write_blob(path,&s,sizeof(s)))return 5;
    if(with_item) {
        YellowAntidoteInventorySlot item={{0}};
        if(!yellow_antidote_inventory_create(2u,s.parent.checksum,s.generation,&item))return 6;
        snprintf(path,sizeof(path),"%s/antidote_%c.v1",argv[1],slot?'b':'a');
        if(write_blob(path,&item,sizeof(item)))return 7;
    }
    puts("LEGACY_FIXTURE_PARENT_AND_OPTIONAL_STOCK_PASS");
    return 0;
}
