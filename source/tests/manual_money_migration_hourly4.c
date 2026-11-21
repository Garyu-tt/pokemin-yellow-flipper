/* Offline, opt-in repair for legacy atomic v1/v3 wallets.
 * Compile on a trusted host. NEVER runs on Flipper or writes the input SD.
 * The old parent is retained byte-for-byte: only missing generation-bound
 * sidecars are produced into a fresh output directory.
 */
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../yellow_core.h"
#include "../yellow_pewter_progress.h"
#include "../yellow_antidote_inventory_slot.h"
#include "../yellow_money_stock_slot.h"
#define YELLOW_ATOMIC_ALLOW_V4 1
#include "../yellow_atomic_snapshot.c"

static bool regular(const char* path) {
    struct stat st;
    return lstat(path,&st)==0 && S_ISREG(st.st_mode);
}
static bool exact_read(const char* path,void* out,size_t size) {
    if(!regular(path))return false;
    FILE* f=fopen(path,"rb");
    if(!f)return false;
    bool ok=fread(out,1,size,f)==size && fgetc(f)==EOF && !ferror(f);
    if(fclose(f))ok=false;
    return ok;
}
static bool new_write(const char* path,const void* data,size_t size) {
    int fd=open(path,O_WRONLY|O_CREAT|O_EXCL|O_NOFOLLOW,0600);
    if(fd<0)return false;
    const uint8_t* b=data;
    size_t done=0;
    bool ok=true;
    while(done<size) {
        ssize_t n=write(fd,b+done,size-done);
        if(n<=0){ok=false;break;}
        done+=(size_t)n;
    }
    if(ok && fsync(fd))ok=false;
    if(close(fd))ok=false;
    if(!ok)unlink(path);
    return ok;
}
static bool parse_uint(const char* s,unsigned limit,unsigned* out) {
    if(!s||!*s)return false;
    unsigned value=0;
    for(const char* p=s;*p;p++) {
        if(*p<'0'||*p>'9')return false;
        unsigned digit=(unsigned)(*p-'0');
        if(value>(limit-digit)/10u)return false;
        value=value*10u+digit;
    }
    *out=value;
    return true;
}
static void bcd_money(unsigned value,uint8_t out[3]) {
    for(int i=2;i>=0;i--) {
        unsigned pair=value%100u;
        out[i]=(uint8_t)(((pair/10u)<<4u)|(pair%10u));
        value/=100u;
    }
}
static int fail(const char* message) {
    fprintf(stderr,"MIGRATION_REFUSED: %s\n",message);
    return 2;
}
int main(int argc,char** argv) {
    const char *snapshot=NULL,*output=NULL,*balance_arg=NULL,*stock_arg=NULL;
    bool ack=false;
    for(int i=1;i<argc;i++) {
        if(!strcmp(argv[i],"--ack-unknown-history"))ack=true;
        else if(i+1<argc && !strcmp(argv[i],"--snapshot"))snapshot=argv[++i];
        else if(i+1<argc && !strcmp(argv[i],"--out-dir"))output=argv[++i];
        else if(i+1<argc && !strcmp(argv[i],"--balance"))balance_arg=argv[++i];
        else if(i+1<argc && !strcmp(argv[i],"--stock"))stock_arg=argv[++i];
        else return fail("unknown/incomplete argument");
    }
    if(!snapshot||!output||!balance_arg||!ack)
        return fail("require --snapshot, --out-dir, --balance and --ack-unknown-history");
    unsigned balance=0,explicit_stock=0;
    if(!parse_uint(balance_arg,999999u,&balance))
        return fail("balance must be explicitly supplied as 0..999999");
    if(stock_arg && !parse_uint(stock_arg,99u,&explicit_stock))
        return fail("stock must be 0..99");
    /* Refuse symlinked input even when it resolves to a valid parent. */
    if(!regular(snapshot))return fail("snapshot must not be a symlink");
    char canonical[PATH_MAX];
    if(!realpath(snapshot,canonical)||!regular(canonical))
        return fail("snapshot must be an existing regular file");
    const char* slash=strrchr(canonical,'/');
    if(!slash)return fail("invalid snapshot path");
    const char* basename=slash+1;
    char slot;
    if(!strcmp(basename,"snapshot_gen_a_v01.dat"))slot='a';
    else if(!strcmp(basename,"snapshot_gen_b_v01.dat"))slot='b';
    else return fail("snapshot filename must identify atomic slot a/b");
    char input_dir[PATH_MAX],antidote_path[PATH_MAX],pair_path[PATH_MAX];
    size_t dirlen=(size_t)(slash-canonical);
    if(!dirlen||dirlen>=sizeof(input_dir))return fail("invalid directory");
    memcpy(input_dir,canonical,dirlen);input_dir[dirlen]=0;
    if(snprintf(antidote_path,sizeof(antidote_path),"%s/antidote_%c.v1",input_dir,slot)
       >=(int)sizeof(antidote_path))return fail("antidote path too long");
    if(snprintf(pair_path,sizeof(pair_path),"%s/money_stock_%c.v1",input_dir,slot)
       >=(int)sizeof(pair_path))return fail("money path too long");
    struct stat existing;
    if(lstat(pair_path,&existing)==0 || errno!=ENOENT)
        return fail("paired wallet already exists (even corrupt); refuse overwrite");
    YellowAtomicSnapshot parent={0};
    if(!exact_read(canonical,&parent,sizeof(parent)) ||
       !yellow_atomic_unpack(&parent,NULL))
        return fail("parent truncated, corrupt, wrong ABI or unsealed");
    if(parent.version!=1u && parent.version!=3u)
        return fail("only atomic v1/v3 without wallet can be migrated");
    YellowAntidoteInventorySlot antidote={{0}};
    uint8_t stock=0;
    bool existing_item=lstat(antidote_path,&existing)==0;
    if(!existing_item && errno!=ENOENT)
        return fail("cannot inspect antidote record");
    if(existing_item) {
        if(stock_arg)return fail("stock override forbidden when inventory record exists");
        if(!exact_read(antidote_path,&antidote,sizeof(antidote)) ||
           !yellow_antidote_inventory_restore(&antidote,
             parent.parent.checksum,parent.generation,&stock))
            return fail("inventory sidecar corrupt/mismatched; never guess stock");
    } else {
        if(parent.version==3u)
            return fail("v3 inventory sidecar missing; stock cannot be inferred");
        if(!stock_arg)
            return fail("v1 stock unknown: require explicit --stock 0..99");
        stock=(uint8_t)explicit_stock;
        if(!yellow_antidote_inventory_create(stock,parent.parent.checksum,
                                               parent.generation,&antidote))
            return fail("cannot construct v1 inventory sidecar");
    }
    uint8_t funds[3]={0};
    bcd_money(balance,funds);
    YellowMoneyStockSlot money={{0}};
    if(!yellow_money_stock_create(funds,stock,parent.parent.checksum,
                                  parent.generation,&money))
        return fail("cannot create bound wallet");
    uint8_t verify_funds[3]={0},verify_stock=255;
    if(!yellow_money_stock_restore(&money,parent.parent.checksum,
           parent.generation,verify_funds,&verify_stock) ||
       memcmp(funds,verify_funds,3u) || verify_stock!=stock)
        return fail("self-verification failed");
    /* Do not touch the original snapshot, input directory or existing files.
       Require a new absolute output directory OUTSIDE the input save tree. */
    if(output[0]!='/')return fail("output path must be absolute");
    char output_copy[PATH_MAX],output_parent[PATH_MAX];
    if(strlen(output)>=sizeof(output_copy))return fail("output path too long");
    strcpy(output_copy,output);
    char* last=strrchr(output_copy,'/');
    if(!last)return fail("invalid output directory");
    if(last==output_copy)last[1]=0;
    else *last=0;
    if(!realpath(output_copy,output_parent))return fail("output parent must exist");
    if(!strcmp(output_parent,input_dir) ||
       (!strncmp(output_parent,input_dir,strlen(input_dir)) &&
        output_parent[strlen(input_dir)]=='/'))
        return fail("output must be outside input save tree");
    if(mkdir(output,0700)!=0)return fail("output directory must not already exist");
    char out_pair[PATH_MAX],out_item[PATH_MAX],out_notes[PATH_MAX];
    if(snprintf(out_pair,sizeof(out_pair),"%s/money_stock_%c.v1",output,slot)
         >=(int)sizeof(out_pair) ||
       snprintf(out_item,sizeof(out_item),"%s/antidote_%c.v1",output,slot)
         >=(int)sizeof(out_item) ||
       snprintf(out_notes,sizeof(out_notes),"%s/MANUAL_INSTALL_ONLY.txt",output)
         >=(int)sizeof(out_notes)) {
        rmdir(output);return fail("output paths too long");
    }
    bool wrote_pair=false,wrote_item=false,wrote_notes=false;
    if(new_write(out_pair,&money,sizeof(money)))wrote_pair=true;
    if(wrote_pair) {
        if(!existing_item)
            wrote_item=new_write(out_item,&antidote,sizeof(antidote));
        else wrote_item=true;
    }
    char notes[1300];
    int len=snprintf(notes,sizeof(notes),
      "OFFLINE MANUAL REVIEW ONLY - NOT A RELEASE OR AUTOMATIC SD REPAIR\n"
      "Snapshot: %s\nVersion: %u\nGeneration: %u\nParent checksum: %08x\n"
      "Owner-asserted historical balance: %u (NOT derivable from snapshot)\n"
      "Antidote stock: %u (%s)\n"
      "Output: money_stock_%c.v1%s\n"
      "BEFORE MANUAL INSTALL: back up entire original save directory, verify\n"
      "snapshot/sidecar SHA-256 and generation; do not overwrite any existing\n"
      "wallet sidecar. Copy only to matching slot after independent acceptance.\n"
      "No Flipper/SD file has been modified by this program.\n",
      canonical,parent.version,parent.generation,parent.parent.checksum,
      balance,(unsigned)stock,existing_item?"validated sidecar":"explicit owner input",
      slot,existing_item?"":" and antidote sidecar");
    if(len>0 && (size_t)len<sizeof(notes) && wrote_pair && wrote_item &&
       new_write(out_notes,notes,(size_t)len))wrote_notes=true;
    if(!wrote_notes) {
        if(wrote_pair)unlink(out_pair);
        if(!existing_item && wrote_item)unlink(out_item);
        rmdir(output);
        return fail("output write/readiness failed; rolled back");
    }
    YellowMoneyStockSlot verify={{0}};
    if(!exact_read(out_pair,&verify,sizeof(verify)) ||
       memcmp(&verify,&money,sizeof(verify))!=0) {
        unlink(out_notes);unlink(out_pair);
        if(!existing_item)unlink(out_item);
        rmdir(output);
        return fail("readback mismatch; rolled back");
    }
    if(!existing_item) {
        YellowAntidoteInventorySlot check={{0}};
        if(!exact_read(out_item,&check,sizeof(check)) ||
           memcmp(&check,&antidote,sizeof(check))!=0) {
            unlink(out_notes);unlink(out_pair);unlink(out_item);rmdir(output);
            return fail("inventory readback mismatch; rolled back");
        }
    }
    printf("OFFLINE_MIGRATION_PREPARED_PASS version=%u generation=%u balance=%u stock=%u\n",
           parent.version,parent.generation,balance,(unsigned)stock);
    return 0;
}
