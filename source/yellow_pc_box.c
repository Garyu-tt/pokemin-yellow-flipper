/* Bounded 20-slot Gen1 PC box for the currently supported 5 capturable
 * native species. No claim of 151 supported battle species. Included by
 * yellow_core.c AFTER the original yellow_valid_moves helper. The
 * original v13 (132-byte) parent save is deliberately unchanged. */
#include <string.h>
static bool yellow_pc_mon_valid(const YCapturedMon* m) {
    if(!m)return false;
    switch(m->species) {
    case YK_PIDGEY:case YK_RATTATA:case YK_CATERPIE:
    case YK_METAPOD:case YK_PIDGEOTTO:
    case YK_GEODUDE:case YK_SANDSHREW:break;
    case YK_ZUBAT:case YK_CLEFAIRY:break;
    default:return false;
    }
    if(m->level<2||m->level>100||!y_status_valid(m->status))return false;
    const YSpeciesInfo* species=y_species_info(m->species);
    if(!species||m->hp>y_calc_stats(m->species,m->level).hp)return false;
    if(m->experience<y_exp_threshold(m->species,m->level) ||
       (m->level<100 && m->experience>=y_exp_threshold(m->species,(uint8_t)(m->level+1u))))
        return false;
    return yellow_valid_moves(m->species,m->level,m->moves,m->pp);
}
static bool yellow_pc_box_valid(const YellowPCBox* box) {
    if(!box||box->count>YELLOW_PC_CAPACITY)return false;
    for(unsigned i=0;i<box->count;i++)
        if(!yellow_pc_mon_valid(&box->slots[i]))return false;
    return true;
}
bool yellow_pc_deposit(YellowGame* g,uint8_t party_index) {
    if(!g||g->screen!=YScreenPC||g->map!=YMapViridianCenter||
       !g->has_pikachu || party_index==0 ||
       party_index>g->caught_count ||
       g->pc.count>=YELLOW_PC_CAPACITY ||!yellow_pc_box_valid(&g->pc))
        return false;
    YCapturedMon member=g->caught[party_index-1u];
    if(!yellow_pc_mon_valid(&member))return false;
    g->pc.slots[g->pc.count++]=member;
    for(uint8_t i=party_index;i<g->caught_count;i++)
        g->caught[i-1u]=g->caught[i];
    --g->caught_count;
    memset(&g->caught[g->caught_count],0,sizeof(YCapturedMon));
    if(g->active_party==party_index)g->active_party=0;
    else if(g->active_party>party_index)g->active_party--;
    return true;
}
bool yellow_pc_withdraw(YellowGame* g,uint8_t box_index) {
    if(!g||g->screen!=YScreenPC||g->map!=YMapViridianCenter||
       !g->has_pikachu ||g->caught_count>=YELLOW_CAPTURE_SLOTS ||
       box_index>=g->pc.count || !yellow_pc_box_valid(&g->pc))
        return false;
    YCapturedMon member=g->pc.slots[box_index];
    g->caught[g->caught_count++]=member;
    for(uint8_t i=box_index;i+1u<g->pc.count;i++)
        g->pc.slots[i]=g->pc.slots[i+1u];
    --g->pc.count;
    memset(&g->pc.slots[g->pc.count],0,sizeof(YCapturedMon));
    (void)yellow_dex_mark_caught(&g->dex,yellow_dex_number(member.species));
    return true;
}
static void yellow_pc_slot_encode(const YCapturedMon* m,uint8_t data[16]) {
    data[0]=m->species;data[1]=m->level;data[2]=m->hp;
    memcpy(data+3,m->pp,4);
    memcpy(data+7,m->moves,4);
    data[11]=m->status;
    for(unsigned j=0;j<4;j++)data[12+j]=(uint8_t)(m->experience>>(8u*j));
}
static void yellow_pc_slot_decode(YCapturedMon* m,const uint8_t data[16]) {
    memset(m,0,sizeof(*m));
    m->species=data[0];m->level=data[1];m->hp=data[2];
    memcpy(m->pp,data+3,4);
    memcpy(m->moves,data+7,4);
    m->status=data[11];
    m->experience=0;
    for(unsigned j=0;j<4;j++)m->experience|=(uint32_t)data[12+j]<<(8u*j);
}
static uint32_t yellow_pc_hash(const YellowPCSave* p) {
    uint32_t hash=2166136261u;
#define PC_HASH_BYTE(value) do {hash^=(uint8_t)(value);hash*=16777619u;}while(0)
    for(unsigned j=0;j<4;j++)PC_HASH_BYTE(p->magic>>(8u*j));
    for(unsigned j=0;j<2;j++)PC_HASH_BYTE(p->version>>(8u*j));
    PC_HASH_BYTE(p->count);
    PC_HASH_BYTE(p->reserved);
    for(unsigned j=0;j<4;j++)PC_HASH_BYTE(p->parent_checksum>>(8u*j));
    for(unsigned i=0;i<YELLOW_PC_CAPACITY;i++)
        for(unsigned j=0;j<YELLOW_PC_SLOT_BYTES;j++)
            PC_HASH_BYTE(p->slots[i][j]);
#undef PC_HASH_BYTE
    return hash;
}
bool yellow_pc_save(const YellowPCBox* box,uint32_t parent_checksum,YellowPCSave* out) {
    if(!out || !yellow_pc_box_valid(box))return false;
    YellowPCSave saved={0};
    saved.magic=YELLOW_PC_MAGIC;saved.version=YELLOW_PC_VERSION;
    saved.count=box->count;saved.parent_checksum=parent_checksum;
    for(unsigned i=0;i<box->count;i++)
        yellow_pc_slot_encode(&box->slots[i],saved.slots[i]);
    saved.checksum=yellow_pc_hash(&saved);
    *out=saved;return true;
}
bool yellow_pc_restore(YellowPCBox* target,uint32_t parent_checksum,
                       const YellowPCSave* p) {
    if(!target||!p||p->magic!=YELLOW_PC_MAGIC ||
       p->version!=YELLOW_PC_VERSION||p->reserved ||
       p->count>YELLOW_PC_CAPACITY||
       p->parent_checksum!=parent_checksum||
       p->checksum!=yellow_pc_hash(p))return false;
    /* Canonical trailing empty slots prevent hidden data and ambiguity. */
    for(unsigned i=p->count;i<YELLOW_PC_CAPACITY;i++)
        for(unsigned j=0;j<YELLOW_PC_SLOT_BYTES;j++)
            if(p->slots[i][j])return false;
    YellowPCBox candidate={0};candidate.count=p->count;
    for(unsigned i=0;i<p->count;i++) {
        yellow_pc_slot_decode(&candidate.slots[i],p->slots[i]);
        if(!yellow_pc_mon_valid(&candidate.slots[i]))return false;
    }
    *target=candidate;return true;
}
