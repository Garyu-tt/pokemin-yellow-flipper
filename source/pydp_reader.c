#include "pydp_reader.h"
#include <string.h>
#define PYDP_MAX_SIZE (512u * 1024u)
static uint16_t u16(const uint8_t *p) {
    return (uint16_t)((uint16_t)p[0] | (uint16_t)p[1]<<8);
}
static uint32_t u32(const uint8_t *p) {
    return (uint32_t)p[0] | (uint32_t)p[1]<<8 |
           (uint32_t)p[2]<<16 | (uint32_t)p[3]<<24;
}
static const PydpSection *sect(const PydpPack *p,const char tag[4]) {
    uint16_t i;
    for (i=0;i<p->sections;i++) {
        if (memcmp(p->section[i].tag,tag,4)==0) return &p->section[i];
    }
    return 0;
}
static int record(const PydpPack *p,const char tag[4],uint32_t index,
                  uint32_t size,uint8_t *data) {
    const PydpSection *s=sect(p,tag);
    if (!s || index>=s->count || size>s->length ||
        index>(s->length-size)/size)return 0;
    return p->read(p->ctx,s->offset+index*size,data,size);
}
int pydp_open(PydpPack *p,PydpRead callback,void *ctx) {
    uint8_t hdr[12],dir[20];
    uint16_t n,i;
    uint32_t full;
    if (!p || !callback)return 0;
    memset(p,0,sizeof(*p));
    p->read=callback;p->ctx=ctx;
    if (!callback(ctx,0,hdr,12))return 0;
    if (memcmp(hdr,"PYDP",4) || u16(hdr+4)!=1)return 0;
    n=u16(hdr+6);full=u32(hdr+8);
    if (n!=8 || full<12u+20u*n || full>PYDP_MAX_SIZE)return 0;
    p->length=full;p->sections=n;
    for (i=0;i<n;i++) {
        PydpSection *s=&p->section[i];
        if (!callback(ctx,12u+i*20u,dir,20))return 0;
        memcpy(s->tag,dir,4);s->offset=u32(dir+4);
        s->length=u32(dir+8);s->crc32=u32(dir+12);s->count=u32(dir+16);
        if (s->offset<12u+20u*n || s->offset>full ||
            s->length>full-s->offset)return 0;
    }
    if (!sect(p,"SPEC") || !sect(p,"MAPS") || !sect(p,"TBLK") ||
        !sect(p,"MOVE") || !sect(p,"WILD") || !sect(p,"TRNR"))return 0;
    return 1;
}
static uint32_t crc32_bytes(uint32_t crc,const uint8_t *data,size_t size) {
    size_t i;unsigned j;
    for (i=0;i<size;i++) {
        crc^=data[i];
        for (j=0;j<8;j++)
            crc=(crc>>1) ^ (0xEDB88320u & (0u-(crc&1u)));
    }
    return crc;
}
int pydp_verify(const PydpPack *p) {
    uint8_t scratch[128];
    uint16_t i;uint32_t remain,where,crc;
    if (!p || !p->read)return 0;
    for (i=0;i<p->sections;i++) {
        const PydpSection *s=&p->section[i];
        remain=s->length;where=s->offset;crc=0xffffffffu;
        while(remain) {
            uint32_t n=remain>sizeof(scratch)?sizeof(scratch):remain;
            if (!p->read(p->ctx,where,scratch,n))return 0;
            crc=crc32_bytes(crc,scratch,n);
            where+=n;remain-=n;
        }
        if ((crc^0xffffffffu)!=s->crc32)return 0;
    }
    return 1;
}
int pydp_species(const PydpPack *p,uint8_t dex,PydpSpecies *out) {
    uint8_t b[8];const PydpSection *s=sect(p,"SPEC");
    if (!out || dex<1 || dex>151 || !s || s->count!=151 ||
        !record(p,"SPEC",dex-1u,8,b) || b[0]!=dex)return 0;
    out->dex=b[0];out->hp=b[1];out->atk=b[2];out->def=b[3];
    out->spd=b[4];out->spc=b[5];out->catch_rate=b[6];out->base_exp=b[7];
    return 1;
}
int pydp_map(const PydpPack *p,uint8_t map_id,PydpMap *out) {
    uint8_t b[9];const PydpSection *s=sect(p,"MAPS");
    if (!out || map_id>=249 || !s || s->count!=249 ||
        !record(p,"MAPS",map_id,9,b) || b[0]!=map_id)return 0;
    out->id=b[0];out->width=b[1];out->height=b[2];
    out->layout_offset=u32(b+3);out->layout_bytes=u16(b+7);
    return 1;
}
int pydp_map_blocks(const PydpPack *p,const PydpMap *m,
                    uint8_t *buffer,size_t buffer_size) {
    const PydpSection *s=sect(p,"TBLK");
    if (!m || !s || !buffer || m->layout_offset==0xffffffffu ||
        m->layout_bytes==0 || m->layout_bytes>buffer_size ||
        m->layout_offset>s->length ||
        m->layout_bytes>s->length-m->layout_offset ||
        m->layout_bytes!=(uint32_t)m->width*m->height)return 0;
    /* External SD Storage callback is intentionally capped at 128 bytes. */
    size_t done=0;
    while(done<m->layout_bytes) {
        size_t chunk=(size_t)m->layout_bytes-done;
        if(chunk>128)chunk=128;
        if(!p->read(p->ctx,s->offset+m->layout_offset+(uint32_t)done,
                    buffer+done,chunk))return 0;
        done+=chunk;
    }
    return 1;
}

int pydp_move(const PydpPack *p,uint8_t move_id,PydpMove *out) {
    uint8_t b[6];const PydpSection *s=sect(p,"MOVE");
    if (!out || move_id<1 || move_id>165 || !s || s->count!=165 ||
        !record(p,"MOVE",move_id-1,6,b) || b[0]!=move_id)return 0;
    out->id=b[0];out->power=b[1];out->accuracy=b[2];
    out->pp=b[3];out->type_id=b[4];out->effect_id=b[5];return 1;
}
int pydp_wild(const PydpPack *p,uint8_t map_id,int water,
              uint8_t *rate,PydpEncounter *slots,size_t capacity,
              size_t *slot_count) {
    const PydpSection *s=sect(p,"WILD");
    uint32_t off=0;uint8_t hdr[5],record2[2];uint8_t count;uint32_t j;
    if (!s || !rate || !slots || !slot_count || (water!=0 && water!=1))return 0;
    while(off<s->length) {
        if (s->length-off<5 || !p->read(p->ctx,s->offset+off,hdr,5))return 0;
        off+=5;
        if ((uint32_t)(hdr[2]+hdr[4])*2>s->length-off)return 0;
        if(hdr[0]==map_id) {
            uint32_t start=off+(water?hdr[2]*2u:0u);
            count=water?hdr[4]:hdr[2];
            if (count>capacity)return 0;
            for(j=0;j<count;j++) {
                if(!p->read(p->ctx,s->offset+start+j*2u,record2,2))return 0;
                if(record2[0]<1 || record2[0]>151 || record2[1]<1 ||
                   record2[1]>100)return 0;
                slots[j].dex=record2[0];slots[j].level=record2[1];
            }
            *rate=water?hdr[3]:hdr[1];*slot_count=count;
            return 1;
        }
        off+=(hdr[2]+hdr[4])*2u;
    }
    return 0;
}
int pydp_trainer_party(const PydpPack *p,uint8_t class_id,uint8_t party_number,
                       PydpEncounter *team,size_t capacity,size_t *team_count) {
    const PydpSection *s=sect(p,"TRNR");
    uint32_t off=0;uint8_t hdr[2],n,entry[2];uint32_t j,k;
    if (!s || !team || !team_count || party_number==0)return 0;
    while(off<s->length) {
        if(s->length-off<2 || !p->read(p->ctx,s->offset+off,hdr,2))return 0;
        off+=2;
        for(j=1;j<=hdr[1];j++) {
            if(off>=s->length || !p->read(p->ctx,s->offset+off,&n,1))return 0;
            off++;
            if(n<1 || n>6 || (uint32_t)n*2u>s->length-off)return 0;
            if(hdr[0]==class_id && j==party_number) {
                if(n>capacity)return 0;
                for(k=0;k<n;k++) {
                    if(!p->read(p->ctx,s->offset+off+k*2u,entry,2))return 0;
                    if(entry[0]<1 || entry[0]>151 || entry[1]<1 || entry[1]>100)return 0;
                    team[k].dex=entry[0];team[k].level=entry[1];
                }
                *team_count=n;return 1;
            }
            off+=n*2u;
        }
    }
    return 0;
}

/* Original (width x height) 2x2-tile map block indices. A map with an
   unverified source dimension is never addressable as valid geometry. */
int pydp_map_block_at(const PydpPack *p,uint8_t map_id,uint8_t bx,
                      uint8_t by,uint8_t *out) {
    if(!p || !out)return 0;
    PydpMap map;
    const PydpSection* s=sect(p,"TBLK");
    if(!s || !pydp_map(p,map_id,&map) ||
       bx>=map.width || by>=map.height ||
       map.layout_offset==0xffffffffu || !map.layout_bytes ||
       (uint32_t)map.width*map.height!=map.layout_bytes ||
       map.layout_offset>s->length ||
       map.layout_bytes>s->length-map.layout_offset)return 0;
    uint32_t index=(uint32_t)by*map.width+bx;
    return p->read(p->ctx,s->offset+map.layout_offset+index,out,1);
}
