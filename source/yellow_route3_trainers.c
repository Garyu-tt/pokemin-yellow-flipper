/* Source-bound subset of original Route3 trainers from pinned
 * data/maps/objects/Route3.asm, scripts/Route3.asm and
 * data/trainers/parties.asm. Exactly three of eight trainer parties use
 * only species currently implemented by the native battle runtime:
 *  original trainer header 2: LASS 1, Pidgey Lv9, Pidgey Lv9
 *  original trainer header 4: LASS 2, Rattata Lv10, NidoranM Lv10
 *  original trainer header 6: BUG_CATCHER 6, Caterpie Lv11, Metapod Lv11.
 * Unsupported parties and all remaining scripts are NOT fabricated. */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
typedef struct {
    uint8_t original_index,x,y,facing,range,count;
    uint8_t species[2],levels[2];
} YRoute3Trainer;
static const YRoute3Trainer route3_supported[3]={
    {2,16,9,YLeft,2,2,{YK_PIDGEY,YK_PIDGEY},{9,9}},
    {4,23,4,YLeft,4,2,{YK_RATTATA,YK_NIDORAN_M},{10,10}},
    {6,24,6,YRight,3,2,{YK_CATERPIE,YK_METAPOD},{11,11}}
};
static const YRoute3Trainer* route3_trainer(unsigned index) {
    return index<3?&route3_supported[index]:NULL;
}
static int route3_trainer_at(int x,int y) {
    for(int i=0;i<3;i++)
        if(route3_supported[i].x==x && route3_supported[i].y==y)return i;
    return -1;
}
static uint32_t route3_trainer_checksum(const YellowRoute3TrainerSave* s) {
    uint32_t hash=2166136261u;
    uint32_t fields[]={s->magic,(uint32_t)s->version,
                       s->defeated_mask,s->reserved,s->parent_checksum};
    for(unsigned i=0;i<5;i++) {
        unsigned size=(i==1)?2:(i==2||i==3)?1:4;
        for(unsigned j=0;j<size;j++) {
            hash^=(uint8_t)(fields[i]>>(8u*j));
            hash*=16777619u;
        }
    }
    return hash;
}
bool yellow_route3_trainer_save(uint8_t defeated_mask,
                                uint32_t parent_checksum,
                                YellowRoute3TrainerSave* out) {
    if(!out || (defeated_mask&~YELLOW_ROUTE3_DEFEATED_BITS))return false;
    YellowRoute3TrainerSave result={0};
    result.magic=YELLOW_ROUTE3_TRAINER_MAGIC;result.version=1;
    result.defeated_mask=defeated_mask;
    result.parent_checksum=parent_checksum;
    result.checksum=route3_trainer_checksum(&result);
    *out=result;
    return true;
}
bool yellow_route3_trainer_restore(uint8_t* defeated_mask,
                                   uint32_t parent_checksum,
                                   const YellowRoute3TrainerSave* saved) {
    if(!defeated_mask||!saved || saved->magic!=YELLOW_ROUTE3_TRAINER_MAGIC ||
       saved->version!=1 || saved->reserved ||
       (saved->defeated_mask&~YELLOW_ROUTE3_DEFEATED_BITS) ||
       saved->parent_checksum!=parent_checksum ||
       saved->checksum!=route3_trainer_checksum(saved))return false;
    *defeated_mask=saved->defeated_mask;
    return true;
}
