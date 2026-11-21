/* Exact position sidecar v1 -- separate from old v13 and atomic v1.
 * Bound to BOTH the selected atomic generation and its signed parent.
 * Missing or invalid sidecar leaves the already-valid old checkpoint intact.
 * All maps use native enum IDs; never write original source IDs directly.
 */
#ifndef YELLOW_EXACT_POSITION_V1_INCLUDED
#define YELLOW_EXACT_POSITION_V1_INCLUDED
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define YELLOW_EXACT_POSITION_MAGIC 0x31535059u /* YPS1 */
#define YELLOW_EXACT_POSITION_VERSION 1u
#define YELLOW_EXACT_POSITION_SEAL 0x45534f50u /* POSE */
typedef struct {
    uint32_t magic,version,generation,parent_checksum;
    uint8_t map,x,y,facing;
    uint32_t checksum,seal;
} YellowExactPositionV1;
_Static_assert(sizeof(YellowExactPositionV1)==28,
               "Exact-position sidecar ABI must be 28 bytes");

static uint32_t yellow_exact_position_hash(const YellowExactPositionV1* p){
    const uint8_t* b=(const uint8_t*)p;
    uint32_t h=2166136261u;
    for(size_t i=0;i<offsetof(YellowExactPositionV1,checksum);i++)
        h=(h^b[i])*16777619u;
    return h;
}
static bool yellow_exact_position_bounds(uint8_t map,int x,int y){
    /* Original maps 0..10 store exact positions directly in YellowSave v13.
     * Extended maps 11..19 require a full native gameplay scene and safe
     * legal grid coordinates (not necessarily walkable spawn: maps may
     * legitimately contain warp transitions at their edges). */
    if(map<YMapPewterCity || (map>YMapViridianMart && map!=YMapCeruleanMart && map!=YMapRoute10))return false;
    const YellowLevelAsset* a=yellow_gameplay_scene_asset(map);
    return a && x>=0 && y>=0 && x<(int)a->w*2 && y<(int)a->h*2;
}
static bool yellow_exact_position_make(const YellowGame* game,
        const YellowAtomicSnapshot* parent,YellowExactPositionV1* out){
    if(!game||!parent||!out || !yellow_atomic_unpack(parent,NULL))
        return false;
    /* Do not commit a sidecar the restore path will reject. */
    if(game->facing>YRight ||
       !yellow_exact_position_bounds(game->map,game->x,game->y))
        return false;
    if(parent->parent.map!=YMapRoute2 ||
       parent->parent.x!=9 || parent->parent.y!=0)
        return false; /* v13 extended-map fallback invariant */
    YellowExactPositionV1 p={0};
    p.magic=YELLOW_EXACT_POSITION_MAGIC;
    p.version=YELLOW_EXACT_POSITION_VERSION;
    p.generation=parent->generation;
    p.parent_checksum=parent->parent.checksum;
    p.map=game->map;p.x=(uint8_t)game->x;
    p.y=(uint8_t)game->y;p.facing=game->facing;
    p.checksum=yellow_exact_position_hash(&p);
    p.seal=YELLOW_EXACT_POSITION_SEAL;
    *out=p;
    return true;
}
static bool yellow_exact_position_restore(YellowGame* game,
        const YellowAtomicSnapshot* parent,const YellowExactPositionV1* p){
    if(!game||!parent||!p || !yellow_atomic_unpack(parent,NULL))return false;
    if(p->magic!=YELLOW_EXACT_POSITION_MAGIC ||
       p->version!=YELLOW_EXACT_POSITION_VERSION ||
       p->seal!=YELLOW_EXACT_POSITION_SEAL ||
       p->checksum!=yellow_exact_position_hash(p) ||
       p->generation!=parent->generation ||
       p->parent_checksum!=parent->parent.checksum ||
       parent->parent.map!=YMapRoute2 || parent->parent.x!=9 ||
       parent->parent.y!=0 ||
       !yellow_exact_position_bounds(p->map,p->x,p->y) ||
       p->facing>YRight)return false;
    game->map=p->map;
    game->x=(int8_t)p->x;game->y=(int8_t)p->y;
    game->facing=p->facing;
    return true;
}
#endif
