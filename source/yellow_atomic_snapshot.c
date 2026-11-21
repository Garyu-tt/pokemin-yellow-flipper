/* Crash-recoverable two-slot versioned aggregate snapshot.
 * Native v13 and all six existing sidecar record bytes remain unchanged.
 * This is an all-in-one serialization unit: slot0/slot1 each contain a
 * complete prior state; an inactive slot is rewritten and SEALED LAST.
 * On a torn/invalid newest slot, older valid slot is authoritative.
 * Guarantees tested: interrupted logical writes & checksum validation.
 * Not promised: absolute SD/FAT power-loss atomicity or hardware durability.
 */
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "yellow_core.h"
#include "yellow_pewter_progress.h"
#define YELLOW_ATOMIC_MAGIC 0x41545933u /* 3YTA */
/* Production Flipper saves gain a new parent version that REQUIRES both
 * the Antidote and the paired money/stock sidecars. Host legacy ABI tests
 * continue exercising v3, without changing their original assertions. */
#ifdef YELLOW_ATOMIC_ALLOW_V4
#define YELLOW_ATOMIC_VERSION 4u
#define YELLOW_ATOMIC_PREVIOUS_VERSION 3u
#else
#define YELLOW_ATOMIC_VERSION 3u
#endif
#define YELLOW_ATOMIC_LEGACY_VERSION 1u /* v13-era atomic envelope */
#define YELLOW_ATOMIC_SEAL 0x434d4954u /* TIMC */
typedef struct {
    uint32_t magic,version,generation,payload_size;
    YellowSave parent;
    YellowTrainerSave forest;
    YellowItemSave items;
    YPewterProgressSave pewter;
    YellowRoute3TrainerSave route3;
    YellowDexSave dex;
    YellowPCSave pc;
    uint32_t checksum;
    uint32_t committed;
} YellowAtomicSnapshot;
static uint32_t yellow_atomic_checksum(const YellowAtomicSnapshot* s) {
    const uint8_t* bytes=(const uint8_t*)s;
    uint32_t hash=2166136261u;
    for(size_t i=0;i<offsetof(YellowAtomicSnapshot,checksum);i++){
        hash^=bytes[i];
        hash*=16777619u;
    }
    return hash;
}
bool yellow_atomic_make(const YellowGame* g,uint32_t generation,
                         YellowAtomicSnapshot* out) {
    if(!g || !out || !generation)return false;
    YellowAtomicSnapshot snap={0};
    snap.magic=YELLOW_ATOMIC_MAGIC;
    snap.version=YELLOW_ATOMIC_VERSION;
    snap.generation=generation;
    snap.payload_size=(uint32_t)offsetof(YellowAtomicSnapshot,checksum);
    yellow_save_create(g,&snap.parent);
    yellow_trainer_save_create(g,&snap.parent,&snap.forest);
    yellow_item_save_create(g,&snap.parent,&snap.items);
    if(!y_pewter_progress_save(g->pewter_flags,snap.parent.checksum,&snap.pewter))
        return false;
    if(!yellow_route3_trainer_save(g->route3_defeated_mask,
                                 snap.parent.checksum,&snap.route3))
        return false;
    if(!yellow_dex_save(&g->dex,snap.parent.checksum,&snap.dex))return false;
    if(!yellow_pc_save(&g->pc,snap.parent.checksum,&snap.pc))return false;
    snap.checksum=yellow_atomic_checksum(&snap);
    snap.committed=YELLOW_ATOMIC_SEAL;
    *out=snap;
    return true;
}
bool yellow_atomic_unpack(const YellowAtomicSnapshot* s,YellowGame* out) {
    if(!s || s->magic!=YELLOW_ATOMIC_MAGIC ||
       (s->version!=YELLOW_ATOMIC_VERSION &&
#ifdef YELLOW_ATOMIC_ALLOW_V4
        s->version!=YELLOW_ATOMIC_PREVIOUS_VERSION &&
#endif
        s->version!=YELLOW_ATOMIC_LEGACY_VERSION) || !s->generation ||
       s->payload_size!=offsetof(YellowAtomicSnapshot,checksum) ||
       s->committed!=YELLOW_ATOMIC_SEAL ||
       s->checksum!=yellow_atomic_checksum(s))return false;
    YellowGame restored={0};
    uint8_t flags=0,route3=0;
    YellowDex dex={0};
    YellowPCBox box={0};
    if(!yellow_save_load_with_items(&restored,&s->parent,&s->items))return false;
    if(!yellow_trainer_save_load(&restored,&s->parent,&s->forest))return false;
    if(!y_pewter_progress_restore(&flags,s->parent.checksum,&s->pewter))return false;
    if(!yellow_route3_trainer_restore(&route3,s->parent.checksum,&s->route3))
        return false;
    if(!yellow_dex_restore(&dex,s->parent.checksum,&s->dex))return false;
    if(!yellow_pc_restore(&box,s->parent.checksum,&s->pc))return false;
    restored.pewter_flags=flags;
    restored.route3_defeated_mask=route3;
    restored.pc=box;
    /* Saved dex is authoritative, but any party/PC owned mon still exists. */
    for(unsigned i=0;i<YELLOW_DEX_BYTES;i++){
        restored.dex.seen[i]|=dex.seen[i];
        restored.dex.caught[i]|=dex.caught[i];
    }
    for(unsigned i=0;i<box.count;i++)
        (void)yellow_dex_mark_caught(&restored.dex,
                   yellow_dex_number(box.slots[i].species));
    if(!yellow_dex_valid(&restored.dex))return false;
    if(out)*out=restored;
    return true;
}
static bool yellow_atomic_is_newer(uint32_t a,uint32_t b) {
    return (int32_t)(a-b)>0;
}
/* Works on complete in-memory records. Caller is responsible for reading
 * exactly sizeof(record) bytes; negative indices mean no valid slots. */
int yellow_atomic_select(const YellowAtomicSnapshot* slot0,
                         const YellowAtomicSnapshot* slot1) {
    const bool valid0=yellow_atomic_unpack(slot0,NULL);
    const bool valid1=yellow_atomic_unpack(slot1,NULL);
    if(!valid0)return valid1?1:-1;
    if(!valid1)return 0;
    return yellow_atomic_is_newer(slot1->generation,slot0->generation)?1:0;
}
