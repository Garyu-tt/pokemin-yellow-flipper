/* PYDP/1 reader for Flipper firmware / native host.
   Caller supplies a bounded Storage-read callback. RAM use ~192 bytes.
   This is a data API, not executable Pokemon Yellow gameplay. */
#ifndef POKELLOW_PYDP_READER_H
#define POKELLOW_PYDP_READER_H
#include <stddef.h>
#include <stdint.h>
typedef int (*PydpRead)(void *ctx, uint32_t offset, uint8_t *dst, size_t n);
typedef struct {
    char tag[4];
    uint32_t offset, length, crc32, count;
} PydpSection;
typedef struct {
    PydpRead read;
    void *ctx;
    uint32_t length;
    uint16_t sections;
    PydpSection section[8];
} PydpPack;
typedef struct {
    uint8_t dex, hp, atk, def, spd, spc, catch_rate, base_exp;
} PydpSpecies;
typedef struct {
    uint8_t id, width, height;
    uint32_t layout_offset;
    uint16_t layout_bytes;
} PydpMap;
typedef struct {uint8_t dex,level;} PydpEncounter;
typedef struct {uint8_t id,power,accuracy,pp,type_id,effect_id;} PydpMove;

int pydp_open(PydpPack *p,PydpRead callback,void *ctx);
int pydp_verify(const PydpPack *p); /* CRC32 every section in 128-byte blocks */
int pydp_species(const PydpPack *p,uint8_t dex,PydpSpecies *out);
int pydp_map(const PydpPack *p,uint8_t map_id,PydpMap *out);
int pydp_map_blocks(const PydpPack *p,const PydpMap *m,
                    uint8_t *buffer,size_t buffer_size);
/* Read a single original map BLOCK (not rendered 8x8 tiles).
   One SD seek + one byte; constant RAM regardless of map dimensions. */
int pydp_map_block_at(const PydpPack *p,uint8_t map_id,uint8_t bx,
                      uint8_t by,uint8_t *out);
int pydp_move(const PydpPack *p,uint8_t move_id,PydpMove *out);
/* Water=1, grass=0; returns 10 slots for an encounter map or zero for absent. */
int pydp_wild(const PydpPack *p,uint8_t map_id,int water,
              uint8_t *rate,PydpEncounter *slots,size_t slots_capacity,
              size_t *slot_count);
/* Class index follows original TrainerDataPointers (Bug Catcher =1).
   Team numbers are 1-based, exactly as upstream parties.asm. */
int pydp_trainer_party(const PydpPack *p,uint8_t class_id,uint8_t party_number,
                       PydpEncounter *team,size_t capacity,size_t *team_count);
#endif
