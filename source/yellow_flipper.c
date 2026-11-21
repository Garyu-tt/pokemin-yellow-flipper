#include <furi.h>
#include <gui/gui.h>
#include <furi_hal_speaker.h>
#include <storage/storage.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "yellow_core.h"
#include "yellow_pewter_progress.h"
#include "yellow_bulk_storage.h"
#include "yellow_route1_viewport.h"
#include "yellow_assets.h"
#include "yellow_soundtrack_r02.c" /* source-derived mono music excerpts + routing */
#include "yellow_route22_crisp_tiles.c" /* pinned 2bpp->1bpp */
/* Only the Flipper runtime writes v4; existing host ABI tests remain v3. */
#define YELLOW_ATOMIC_ALLOW_V4 1
#include "yellow_atomic_snapshot.c"
#include "yellow_oak_parcel_slot.h"
#include "yellow_whiteout_slot.h"
#include "yellow_antidote_inventory_slot.h"
#include "yellow_money_stock_slot.h"
/* Additional source-derived city scene, provided by yellow_core.c. */
extern const YellowLevelAsset* yellow_scene_asset(uint8_t map);
extern const YellowLevelAsset* yellow_world_scene_asset(uint8_t map);
extern const YellowLevelAsset* yellow_extended_scene_asset(uint8_t map);
extern const YellowLevelAsset* yellow_full_scene_asset(uint8_t map);
extern const YellowLevelAsset* yellow_gameplay_scene_asset(uint8_t map);
#include "yellow_exact_position_v1.c" /* local two-slot complete save container */

#include "yellow_forest_sprites.h"
#include "ux_visual_math.h"

/* PikaDisplay Crisp R03 variant camera8: preserve logical tile-step 280ms. */
#define PD_CAMERA_QUANTUM 8
#define PD_VISUAL_TICK_MS 35u
#define PD_MAIN_TICK_MS 33u
#define SAVE_PATH "/ext/apps_data/yellow_native/save_v13.dat"
#define FOREST_SAVE_PATH "/ext/apps_data/yellow_native/forest_trainers_v01.dat"
#define FOREST_ITEMS_PATH "/ext/apps_data/yellow_native/forest_items_v01.dat"
#define PC_BOX_PATH "/ext/apps_data/yellow_native/pc_box_v01.dat"
#define DEX_LEDGER_PATH "/ext/apps_data/yellow_native/pokedex_ledger_v01.dat"
#define ROUTE3_TRAINERS_PATH "/ext/apps_data/yellow_native/route3_trainers_v01.dat"
#define PEWTER_PROGRESS_PATH "/ext/apps_data/yellow_native/pewter_progress_v01.dat"
#define PRIOR_SAVE_PATH_R12C "/ext/apps_data/yellow_native/save_v12c.dat"
#define PRIOR_SAVE_PATH_R12B "/ext/apps_data/yellow_native/save_v12b.dat"
#define PRIOR_SAVE_PATH_R12 "/ext/apps_data/yellow_native/save_v12.dat"
#define PRIOR_SAVE_PATH_R11 "/ext/apps_data/yellow_native/save_v11.dat"
#define PRIOR_SAVE_PATH_R10 "/ext/apps_data/yellow_native/save_v10.dat"
#define SAVE_DIR "/ext/apps_data/yellow_native"
#define ATOMIC_SAVE_A SAVE_DIR "/snapshot_gen_a_v01.dat"
#define ATOMIC_SAVE_B SAVE_DIR "/snapshot_gen_b_v01.dat"
#define POSITION_SAVE_A SAVE_DIR "/position_a.v1"
#define POSITION_SAVE_B SAVE_DIR "/position_b.v1"
#define OAK_PARCEL_SAVE_A SAVE_DIR "/oak_parcel_a.v1"
#define OAK_PARCEL_SAVE_B SAVE_DIR "/oak_parcel_b.v1"
#define WHITEOUT_SAVE_A SAVE_DIR "/whiteout_anchor_a.v2"
#define WHITEOUT_SAVE_B SAVE_DIR "/whiteout_anchor_b.v2"
#define ANTIDOTE_SAVE_A SAVE_DIR "/antidote_a.v1"
#define ANTIDOTE_SAVE_B SAVE_DIR "/antidote_b.v1"
#define MONEY_STOCK_SAVE_A SAVE_DIR "/money_stock_a.v1"
#define MONEY_STOCK_SAVE_B SAVE_DIR "/money_stock_b.v1"

typedef enum {
    SceneNone=0,SceneBattleIntro,SceneOakCapture,SceneWildCapture,SceneHpDrain,SceneFaint
} YellowScene;
typedef struct {
    YellowGame g;
    YellowBulkMount bulk;
    YellowRoute1Viewport source_world_tiles;
    bool source_world_pending;
    uint8_t source_cached_map;
    FuriMessageQueue* queue;
    FuriMutex* lock;
    ViewPort* viewport;
    Gui* gui;
    /* Visual state only; never serialized into YellowSave. */
    bool motion_active, fx_active, pending_attack;
    uint32_t motion_start, fx_start;
    int16_t from_x, from_y, from_follow_x, from_follow_y;
    uint8_t step_phase, fx_type;
    bool direction_held, suppress_direction_short, buffered_step;
    YellowKey held_direction, suppressed_key, buffered_direction;
    YellowScene scene;
    uint32_t scene_start;
    YellowGame scene_before; /* in-memory render snapshot, never saved */
    bool capture_success;
    uint8_t hp_foe_end, hp_party_end;
    uint8_t bag_category; /* 0=Medicine, 1=Poke Balls, 2=Key Items */
    bool bag_inside;
    uint8_t bag_medicine_selection; /* 0 potion, 1 antidote */
    uint8_t antidote_feedback; /* volatile, never in v13 */
    bool faint_enemy;
    /* Audio is a volatile client-side effect, never in YellowSave. */
    YellowSoundtrack pallet_audio;
    bool pallet_speaker_owned;
    bool audio_debug;
    bool parcel_warning_acknowledged; /* volatile UI only; never clears save-integrity failure */
    uint16_t audio_last_hz;
    uint8_t audio_debug_track;
} YellowApp;

/* Do not stall rendering/input for tones. The Game Boy music map in
 * pret/pokeyellow/data/maps/songs.asm identifies tracks per location. */
static uint8_t music_for_map(const YellowApp* a) {
    if(!a->g.running || (a->g.screen!=YScreenWorld &&
                        a->g.screen!=YScreenDialog))return YTrackNone;
    switch(a->g.map) {
    case YMapPallet:
    case YMapRedsHouse:
    case YMapBluesHouse: return YTrackPallet;
    case YMapOaksLab: return YTrackOaksLab;
    case YMapRoute1:
    case YMapRoute2: return YTrackRoutes1;
    case YMapViridian:
    case YMapPewterCity:
    case YMapForestSouthGate:
    case YMapForestNorthGate: return YTrackCities1;
    case YMapViridianCenter: return YTrackPokeCenter;
    case YMapViridianForest: return YTrackForest;
    case YMapPewterGym: return YTrackGym;
    case YMapRoute3:
    case YMapRoute4: return YTrackRoutes3;
    case YMapMtMoon1F:
    case YMapMtMoonB1F:
    case YMapMtMoonB2F: return YTrackMtMoon;
    default: return YTrackNone;
    }
}
static bool pallet_music_wanted(const YellowApp* a) {
    return music_for_map(a)!=YTrackNone;
}
static void pallet_audio_release(YellowApp* a) {
    if(a->pallet_speaker_owned) {
        furi_hal_speaker_stop();
        furi_hal_speaker_release();
        a->pallet_speaker_owned=false;
    }
    ysound_reset(&a->pallet_audio);
    a->audio_last_hz=0;
}
static void pallet_audio_tick(YellowApp* a,uint32_t now_ms) {
    uint8_t wanted=music_for_map(a);
    a->audio_debug_track=wanted;
    if(wanted==YTrackNone) {
        if(a->pallet_speaker_owned || a->pallet_audio.started)
            pallet_audio_release(a);
        return;
    }
    /* Force a clean channel transition when changing map music.
       No artificial pauses in movement or waiting for a tone. */
    if(a->pallet_audio.track!=wanted && a->pallet_speaker_owned) {
        furi_hal_speaker_stop();
        a->audio_last_hz=0;
    }
    if(!a->pallet_speaker_owned) {
        if(!furi_hal_speaker_acquire(0))return;
        a->pallet_speaker_owned=true;
        ysound_reset(&a->pallet_audio);
    }
    YSoundTone tone;
    if(!ysound_next(&a->pallet_audio,wanted,now_ms,&tone))return;
    a->audio_last_hz=tone.hz;
    if(tone.hz)furi_hal_speaker_start((float)tone.hz,0.075f);
    else furi_hal_speaker_stop();
}
static int clamp(int x,int lo,int hi) {
    if(x<lo)return lo;
    if(x>hi)return hi;
    return x;
}
static const uint8_t* actor_pixels(uint8_t type) {
    switch(type) {
    case YActorOak:return ysprite_oak;
    case YActorGirl:return ysprite_little_girl;
    case YActorFisher:return ysprite_fisher;
    case YActorBlue:return ysprite_blue;
    case YActorPikachu:return ysprite_pikachu;
    case YActorMom:return ysprite_mom;
    case YActorDaisy:return ysprite_daisy;
    case YActorPokedex:return ysprite_pokedex;
    case YActorPokeBall:return ysprite_poke_ball;
    case YActorScientist:return ysprite_scientist;
    case YActorYoungster:return ysprite_youngster;
    case YActorGambler:return ysprite_gambler;
    case YActorNurse:return ysprite_nurse;
    case YActorGentleman:return ysprite_gentleman;
    case YActorCooltrainer:return ysprite_cooltrainer_m;
    case YActorReceptionist:return ysprite_link_receptionist;
    case YActorChansey:return ysprite_chansey;
    case YActorRed:default:return ysprite_red;
    }
}
static void actor(Canvas* c,int x,int y,uint8_t type,int facing,bool walking,bool alternate) {
    if(x<=-16||x>=128||y<=-16||y>=64)return;
    const uint8_t* sheet=actor_pixels(type);
    bool stationary=(type==YActorPokedex||type==YActorPokeBall||
        type==YActorMom||type==YActorNurse||type==YActorChansey);
    int frame=stationary?0:ux_sprite_frame((uint8_t)facing,walking);
    const uint8_t* pixels=sheet+frame*32;
    uint8_t flip[32];
    /* Standing and walking SIDE frames face left; right is X-flipped. */
    if(!stationary && (facing==YRight ||
       (walking && alternate && (facing==YUp||facing==YDown)))) {
        for(unsigned row=0;row<16;row++) {
            flip[2*row]=ux_reverse_bits(pixels[2*row+1]);
            flip[2*row+1]=ux_reverse_bits(pixels[2*row]);
        }
        pixels=flip;
    }
    canvas_set_bitmap_mode(c,true);
    canvas_draw_xbm(c,x,y,16,16,pixels);
}

/* Screen-specific optical persistence pacing; do not change shared gameplay
 * UX helper semantics or the accepted R05 walk regression contract. */
static inline bool pd_lcd_walk_pose(uint16_t t) {
    return t >= 48 && t < 208;
}
static void render_world(Canvas* c,const YellowApp* a) {
    const YellowGame* g=&a->g;
    const YellowLevelAsset* level=yellow_gameplay_scene_asset(g->map);
    const int world_w=level->w*32,world_h=level->h*32;
    int player_x=(int)g->x*16, player_y=(int)g->y*16;
    uint16_t alpha=256;
    bool walking=false;
    if(a->motion_active && g->screen==YScreenWorld) {
        /* Optical persistence on the ST756x LCD makes 30 Hz 1px
         * sprite shifts look like two sprites. Keep game logic at 280 ms
         * per tile but only expose a fresh sprite position every 50 ms.
         * Identical frame data is suppressed by PikaDisplay dirty pages.
         */
        uint32_t elapsed=furi_get_tick()-a->motion_start;
        uint32_t visual_ms=elapsed>=280?280:(elapsed/PD_VISUAL_TICK_MS)*PD_VISUAL_TICK_MS;
        alpha=ux_linear_q8(visual_ms,280);
        walking=alpha<256;
        player_x=ux_interpolate((int)a->from_x*16,player_x,alpha);
        player_y=ux_interpolate((int)a->from_y*16,player_y,alpha);
    }
    /* Camera moves in coarser whole-pixel quanta than the actor.
       Static world pixels no longer flip on each actor sub-tile position.
       The alternative 1px actor mode tests whether more temporal samples
       look better or simply produce more LCD persistence trails.
       Camera quantization is done in SCREEN-space, not world-space. */
    const int camera_target_x=clamp(player_x-56,0,world_w-128);
    const int camera_target_y=clamp(player_y-24,0,world_h-64);
    const int cam_x=(camera_target_x/PD_CAMERA_QUANTUM)*PD_CAMERA_QUANTUM;
    const int cam_y=(camera_target_y/PD_CAMERA_QUANTUM)*PD_CAMERA_QUANTUM;
    const int first_col=cam_x/8,first_row=cam_y/8;
    canvas_set_color(c,ColorBlack);
    canvas_set_bitmap_mode(c,false);
    /* Outdoor road is made quiet, preserving distinct, dense wild-grass tiles. */
    static const uint8_t path_tile[8]={0,0,0x02,0,0,0,0,0};
    bool outdoor=g->map==YMapPallet||g->map==YMapRoute1||
                 g->map==YMapViridian||g->map==YMapRoute2||
                 g->map==YMapPewterCity||
                 g->map==YMapCeruleanCity||
                 g->map==YMapRoute9||g->map==YMapRoute10||
                 g->map==YMapRoute22;
    for(int row=first_row;row<=first_row+8&&row<world_h/8;row++) {
        for(int col=first_col;col<=first_col+16&&col<world_w/8;col++) {
            int x=col*8-cam_x,y=row*8-cam_y;
            uint8_t tile=yellow_world_tile_id(g->map,col,row);
            if(tile<level->tiles_count) {
                const uint8_t* image=(outdoor&&tile==0x2c)?
                    path_tile:level->tiles+tile*8;
                if(g->map==YMapRoute22)
                    image=ytiles_overworld_route22_crisp+tile*8u;
                if((g->map==YMapRoute1 || g->map==YMapViridianForest) &&
                   a->bulk.graphics_ready) {
                    const uint8_t* original=yellow_route1_cached_tile(
                        &a->source_world_tiles,col,row);
                    if(original)image=original;
                }
                canvas_draw_xbm(c,x,y,8,8,image);
            }
        }
    }
    for(uint8_t i=0;i<yellow_npc_count;i++) {
        const YellowNpc* n=&yellow_npcs[i];
        if(yellow_npc_visible(g,n))
            actor(c,n->x*16-cam_x,n->y*16-cam_y,n->type,YDown,false,false);
    }
    /* Original ViridianMart_Object clerk (0,5), visual affordance for
     * a real adjacent interaction, not a hidden debug purchase hotkey. */
    if(g->map==YMapViridianMart)
        actor(c,-cam_x,5*16-cam_y,YActorReceptionist,YRight,false,false);
    if(g->story==YStoryPikachuFollows && g->has_pikachu &&
       g->follower_valid && g->follower_map==g->map) {
        int fx=(int)g->follower_x*16,fy=(int)g->follower_y*16;
        if(walking) {
            fx=ux_interpolate((int)a->from_follow_x*16,fx,alpha);
            fy=ux_interpolate((int)a->from_follow_y*16,fy,alpha);
        }
        actor(c,fx-cam_x,fy-cam_y,YActorPikachu,g->facing,walking && pd_lcd_walk_pose(alpha),a->step_phase&1);
    }
    actor(c,player_x-cam_x,player_y-cam_y,YActorRed,g->facing,
        walking && pd_lcd_walk_pose(alpha),a->step_phase&1);
}

/* Only the application task reads Storage. The GUI render callback
   never blocks on SD card: missing imported pixels fall back to v026. */
static bool source_route1_read(void* ctx,uint16_t col,uint16_t row,
                               uint8_t original[8]) {
    const YellowBulkMount* pack=(const YellowBulkMount*)ctx;
    return yellow_bulk_original_tile_pixels(pack,12,(uint8_t)(col/4),
              (uint8_t)(row/4),(uint8_t)(col%4),(uint8_t)(row%4),original);
}
static bool source_forest_read(void* ctx,uint16_t col,uint16_t row,
                               uint8_t original[8]) {
    const YellowBulkMount* pack=(const YellowBulkMount*)ctx;
    return yellow_bulk_original_tile_pixels(pack,51,(uint8_t)(col/4),
              (uint8_t)(row/4),(uint8_t)(col%4),(uint8_t)(row%4),original);
}
static bool source_prepare_viewport(YellowApp* a) {
    if(!a->bulk.graphics_ready ||
       (a->g.map!=YMapRoute1 && a->g.map!=YMapViridianForest) ||
       (a->g.screen!=YScreenWorld && a->g.screen!=YScreenDialog))
        return true;
    const YellowGame* g=&a->g;
    const YellowLevelAsset* level=&yellow_levels[g->map];
    if(a->source_cached_map!=g->map) {
        memset(&a->source_world_tiles,0,sizeof(a->source_world_tiles));
        a->source_cached_map=g->map;
    }
    int player_x=(int)g->x*16,player_y=(int)g->y*16;
    if(a->motion_active && g->screen==YScreenWorld) {
        uint16_t alpha=ux_linear_q8(furi_get_tick()-a->motion_start,280);
        player_x=ux_interpolate((int)a->from_x*16,player_x,alpha);
        player_y=ux_interpolate((int)a->from_y*16,player_y,alpha);
    }
    int cam_x=clamp(player_x-56,0,(int)level->w*32-128);
    int cam_y=clamp(player_y-24,0,(int)level->h*32-64);
    YellowRoute1TileReader reader=
        g->map==YMapViridianForest?source_forest_read:source_route1_read;
    return yellow_route1_prefetch(&a->source_world_tiles,reader,&a->bulk,
            cam_x/8,cam_y/8,(int)level->w*4,(int)level->h*4,12);
}
static void render_dialog(Canvas* c,const YellowGame* g) {
    canvas_set_color(c,ColorWhite);
    canvas_draw_box(c,0,34,128,30);
    canvas_set_color(c,ColorBlack);
    canvas_draw_frame(c,0,34,128,30);
    canvas_set_font(c,FontSecondary);
    const char* const* lines=&g->dialog_lines[2*g->dialog_page];
    canvas_draw_str(c,4,45,lines[0]);
    canvas_draw_str(c,4,56,lines[1]);
    canvas_draw_str(c,120,62,">");
}
static void render_nurse_choice(Canvas* c,const YellowApp* a) {
    render_world(c,a);
    canvas_set_color(c,ColorWhite);
    canvas_draw_box(c,0,31,128,33);
    canvas_set_color(c,ColorBlack);
    canvas_draw_frame(c,0,31,128,33);
    canvas_set_font(c,FontSecondary);
    canvas_draw_str(c,4,42,"HEAL YOUR POKEMON?");
    canvas_draw_str(c,5,55,a->g.selected==0?"> YES":"  YES");
    canvas_draw_str(c,72,55,a->g.selected==1?"> NO":"  NO");
}
static void render_menu(Canvas* c,const YellowGame* g) {
    static const char* const items[]={"PARTY","BAG","SAVE","EXIT","LINK LAB","PC BOX"};
    canvas_set_font(c,FontSecondary);
    canvas_draw_str(c,2,10,"POKEMON YELLOW [PORT]");
    int first=g->selected>=5?2:(g->selected>=4?1:0);
    for(int i=first;i<first+4;i++){
        int y=23+(i-first)*12;
        if(g->selected==i)canvas_draw_str(c,6,y,">");
        canvas_draw_str(c,19,y,items[i]);
    }
    canvas_draw_str(c,79,10,"RIGHT:DEX");
    if(g->request_save)canvas_draw_str(c,78,61,"SAVING");
    else if(g->oak_parcel_integrity_error || g->last_blackout_integrity_error ||
            g->antidote_integrity_error || g->money_integrity_error)
        canvas_draw_str(c,56,61,"SAVE LOCKED");
    else if(g->message_saved)canvas_draw_str(c,84,61,"SAVED");
    else {
        char dex_line[26];
        snprintf(dex_line,sizeof(dex_line),"DEX %u/%u",
                 (unsigned)yellow_dex_caught_count(&g->dex),
                 (unsigned)yellow_dex_seen_count(&g->dex));
        canvas_draw_str(c,2,62,dex_line);
    }
}
static void render_pokedex(Canvas* c,const YellowGame* g) {
    uint8_t num=g->selected;
    if(num==0||num>YELLOW_DEX_SPECIES)num=1;
    canvas_set_font(c,FontSecondary);
    char line[44];
    snprintf(line,sizeof(line),"POKEDEX  %03u/151",(unsigned)num);
    canvas_draw_str(c,3,10,line);
    canvas_draw_line(c,0,13,127,13);
    const char* name=yellow_dex_name(num);
    canvas_draw_str(c,4,27,name?name:"?");
    const char* status=yellow_dex_caught(&g->dex,num)?"CAUGHT":
                       (yellow_dex_seen(&g->dex,num)?"SEEN":"NOT SEEN");
    canvas_draw_str(c,4,40,status);
    snprintf(line,sizeof(line),"SEEN:%u  OWNED:%u",
      (unsigned)yellow_dex_seen_count(&g->dex),
      (unsigned)yellow_dex_caught_count(&g->dex));
    canvas_draw_str(c,3,51,line);
    canvas_draw_str(c,3,62,"UP/DN  LEFT/RIGHT x10");
}
static void render_pc(Canvas* c,const YellowGame* g){
    canvas_set_font(c,FontSecondary);
    canvas_draw_str(c,3,9,"PC BOX (VIRIDIAN)");
    canvas_draw_line(c,0,12,127,12);
    char line[38];
    snprintf(line,sizeof(line),"BOX %u/%u   PARTY %u/%u",
        (unsigned)g->pc.count,(unsigned)YELLOW_PC_CAPACITY,
        (unsigned)yellow_party_size(g),(unsigned)YELLOW_PARTY_MAX);
    canvas_draw_str(c,3,22,line);
    if(g->pc_mode==0) {
        const char* opts[]={"DEPOSIT","WITHDRAW","EXIT"};
        for(uint8_t i=0;i<3;i++) {
            canvas_draw_str(c,3,32+10*i,g->selected==i?">":" ");
            canvas_draw_str(c,16,32+10*i,opts[i]);
        }
    } else {
        uint8_t count=g->pc_mode==1?g->caught_count:g->pc.count;
        canvas_draw_str(c,3,32,g->pc_mode==1?"DEPOSIT FROM PARTY":"WITHDRAW FROM BOX");
        if(!count)canvas_draw_str(c,3,47,"NO POKEMON");
        else {
            uint8_t i=g->selected;
            if(i>=count)i=count-1u;
            uint8_t species=g->pc_mode==1?g->caught[i].species:g->pc.slots[i].species;
            const YSpeciesInfo* info=y_species_info(species);
            snprintf(line,sizeof(line),"%u/%u %s",(unsigned)i+1u,(unsigned)count,
                info?info->name:"UNKNOWN");
            canvas_draw_str(c,3,46,line);
            canvas_draw_str(c,3,59,"UP/DN: SELECT  OK: MOVE");
        }
    }
}
static void render_link_offline(Canvas* c,const YellowGame* g) {
    canvas_set_font(c,FontSecondary);
    canvas_draw_str(c,2,10,"OFFLINE LINK LAB");
    canvas_draw_line(c,0,13,127,13);
    canvas_draw_str(c,3,25,"LOCAL LOOPBACK ONLY");
    canvas_draw_str(c,3,36,"NO USB / NO BT");
    char line[32];
    snprintf(line,sizeof(line),"SEQ %u: %s",
        (unsigned)g->link_sequence,
        g->link_result==1?"CRC OK":g->link_result==2?"FAILED":"READY");
    canvas_draw_str(c,3,47,line);
    canvas_draw_str(c,3,62,"OK:TEST  BACK:MENU");
}
static const char* battle_status_tag(uint8_t status) {
    if(status==YStatusParalyzed)return "PAR";
    if(status==YStatusBurned)return "BRN";
    if(status==YStatusPoisoned || y_status_is_toxic(status))return "PSN";
    if(status==YStatusFrozen)return "FRZ";
    if(y_status_is_sleep(status))return "SLP";
    return "";
}
static void render_info(Canvas* c,const YellowGame* g) {
    canvas_set_font(c,FontSecondary);
    char line[36];
    if(g->screen==YScreenParty) {
        if(g->party_battle)
            snprintf(line,sizeof(line),g->force_switch?"FAINTED - CHOOSE":"SWITCH POKEMON");
        else
            snprintf(line,sizeof(line),"PARTY %u/6",(unsigned)yellow_party_size(g));
        canvas_draw_str(c,3,9,line);
        canvas_draw_line(c,2,12,126,12);
        uint8_t first=(uint8_t)((g->selected/3u)*3u);
        for(uint8_t n=0;n<3;n++) {
            uint8_t i=(uint8_t)(first+n);
            if(i>=yellow_party_size(g))break;
            int y=25+12*(int)n;
            if(i==g->selected)canvas_draw_str(c,2,y,">");
            if(i==g->active_party)canvas_draw_str(c,119,y,"*");
            if(i==0&&g->has_pikachu) {
                snprintf(line,sizeof(line),"PIKACHU L%u %u/%u",
                    (unsigned)g->pika_level,(unsigned)g->hp,
                    (unsigned)yellow_player_max_hp(g));
            } else {
                const YCapturedMon* mon=&g->caught[i-1u];
                const YSpeciesInfo* info=y_species_info(mon->species);
                snprintf(line,sizeof(line),"%s L%u %u/%u",
                    info?info->name:"???",(unsigned)mon->level,
                    (unsigned)mon->hp,
                    (unsigned)y_calc_stats(mon->species,mon->level).hp);
            }
            canvas_draw_str(c,14,y,line);
            if(yellow_party_status(g,i)!=YStatusNone)
                canvas_draw_str(c,99,y,battle_status_tag(yellow_party_status(g,i)));
        }
        canvas_draw_str(c,3,62,g->force_switch?"OK: SELECT (REQUIRED)":
                           g->party_battle?"OK: SWITCH  BACK: NO":"OK: ACTIVE  BACK");
    }
}
static void render_bag(Canvas* c,const YellowApp* a) {
    const YellowGame* g=&a->g;
    canvas_set_font(c,FontSecondary);
    if(!a->bag_inside) {
        canvas_draw_str(c,4,10,"BAG - POCKETS");
        canvas_draw_line(c,0,13,127,13);
        static const char* const names[]={"MEDICINE","POKE BALLS","KEY ITEMS"};
        for(uint8_t i=0;i<3;i++) {
            int y=27+(int)i*13;
            if(i==a->bag_category)canvas_draw_str(c,3,y,">");
            canvas_draw_str(c,17,y,names[i]);
        }
        canvas_draw_str(c,2,63,"OK: OPEN  BACK: MENU");
    } else {
        static const char* const names[]={"MEDICINE","POKE BALLS","KEY ITEMS"};
        canvas_draw_str(c,3,10,names[a->bag_category]);
        canvas_draw_line(c,0,13,127,13);
        char line[36];
        if(a->bag_category==0) {
            snprintf(line,sizeof(line),"POTION x%u",(unsigned)g->potions);
            canvas_draw_str(c,3,26,line);
            if(a->bag_medicine_selection==0)canvas_draw_str(c,110,26,"<");
            snprintf(line,sizeof(line),"ANTIDOTE x%u",(unsigned)g->antidotes);
            canvas_draw_str(c,3,37,line);
            if(a->bag_medicine_selection==1)canvas_draw_str(c,110,37,"<");
            uint8_t party=g->active_party;
            if(g->has_pikachu && party<yellow_party_size(g)) {
                snprintf(line,sizeof(line),"HP %u/%u STATUS %u",
                         (unsigned)yellow_party_hp(g,party),
                         (unsigned)yellow_party_max_hp(g,party),
                         (unsigned)yellow_party_status(g,party));
                canvas_draw_str(c,3,48,line);
            }
            if(a->bag_medicine_selection==1) {
                switch(a->antidote_feedback) {
                case 1:snprintf(line,sizeof(line),"POISON CURED");break;
                case 2:snprintf(line,sizeof(line),"NO ANTIDOTE");break;
                case 3:snprintf(line,sizeof(line),"NOT POISONED / FAINT");break;
                case 4:snprintf(line,sizeof(line),"ITEM DATA LOCKED");break;
                default:snprintf(line,sizeof(line),"OK: USE ANTIDOTE");break;
                }
            } else {
                switch((YBagNotice)g->bag_notice) {
                case YBagHealed:
                    snprintf(line,sizeof(line),"HEALED +%u HP",(unsigned)g->bag_last_heal);break;
                case YBagFullHP:snprintf(line,sizeof(line),"HP ALREADY FULL");break;
                case YBagNoPotions:snprintf(line,sizeof(line),"NO POTIONS LEFT");break;
                case YBagFainted:snprintf(line,sizeof(line),"CANNOT HEAL FAINTED");break;
                case YBagNoParty:snprintf(line,sizeof(line),"NO ACTIVE POKEMON");break;
                default:snprintf(line,sizeof(line),"OK: USE POTION");break;
                }
            }
            canvas_draw_str(c,3,61,line);
        } else if(a->bag_category==1) {
            snprintf(line,sizeof(line),"POKE BALL x%u",(unsigned)g->balls);
            canvas_draw_str(c,3,27,line);
            canvas_draw_str(c,3,41,"USE IN WILD BATTLE");
        } else {
            /* Minimal present inventory: the source-verified TM34 reward
               is represented as a single saved machine, not a fake
               Potion/Poke Ball or a newly invented native save field. */
            if(y_oak_parcel_in_bag(g->oak_parcel_flags,
                                   g->oak_parcel_integrity_error)) {
                canvas_draw_str(c,3,27,"OAK'S PARCEL x1");
                canvas_draw_str(c,3,41,(g->pewter_flags&YPewterFlagTM34)?
                                      "TM34 (BIDE) x1":"TAKE TO PROF.OAK");
            } else if(g->pewter_flags&YPewterFlagTM34) {
                canvas_draw_str(c,3,27,"TM34 (BIDE) x1");
                canvas_draw_str(c,3,41,"BROCK'S REWARD");
            } else {
                canvas_draw_str(c,3,27,"(EMPTY)");
                canvas_draw_str(c,3,41,"NO KEY ITEMS");
            }
        }
        canvas_draw_str(c,3,62,"BACK: POCKETS");
    }
}

static void render_oak_capture(Canvas* c) {
    canvas_set_font(c,FontSecondary);
    canvas_draw_str(c,2,10,"WILD PIKACHU!  Lv5");
    canvas_set_bitmap_mode(c,false);
    canvas_draw_xbm(c,87,14,40,40,ypokemon_pikachu_front);
    canvas_set_bitmap_mode(c,true);
    canvas_draw_str(c,3,34,"OAK THROWS");
    canvas_draw_str(c,3,46,"A POKE BALL!");
    canvas_draw_str(c,3,62,"OK: CONTINUE");
}
static void render_battle_motion(Canvas* c,const YellowGame* g,
                                 int player_dx,int foe_dx,
                                 uint8_t foe_cover,uint8_t player_cover) {
    char line[26];
    canvas_set_font(c,FontSecondary);
    const YSpeciesInfo* foe=y_species_info(g->foe_kind);
    snprintf(line,sizeof(line),"%s Lv%u",foe?foe->name:"???",(unsigned)g->foe_level);
    canvas_draw_str(c,1,9,line);
    canvas_draw_frame(c,1,12,68,5);
    if(g->foe_hp&&g->foe_max_hp)
        canvas_draw_box(c,3,14,g->foe_hp*64/g->foe_max_hp,2);
    canvas_set_bitmap_mode(c,false);
    const uint8_t* front=g->foe_kind==YK_PIDGEY?ypokemon_pidgey_front:
                         g->foe_kind==YK_RATTATA?ypokemon_rattata_front:
                         g->foe_kind==YK_CATERPIE?ypokemon_caterpie_front:
                         g->foe_kind==YK_METAPOD?ypokemon_metapod_front:
                         g->foe_kind==YK_PIDGEOTTO?ypokemon_pidgeotto_front:
                         ypokemon_eevee_front;
    canvas_draw_xbm(c,86+foe_dx,0,40,40,front);
    if(foe_cover) {
        canvas_set_color(c,ColorWhite);
        canvas_draw_box(c,85,0,43,foe_cover>=40?40:foe_cover);
        canvas_set_color(c,ColorBlack);
    }
    const uint8_t* back=g->active_party==0?ypokemon_pikachu_back:
        yellow_party_species(g,g->active_party)==YK_PIDGEY?ypokemon_pidgey_back:
        yellow_party_species(g,g->active_party)==YK_CATERPIE?ypokemon_caterpie_back:
        yellow_party_species(g,g->active_party)==YK_METAPOD?ypokemon_metapod_back:
        yellow_party_species(g,g->active_party)==YK_PIDGEOTTO?ypokemon_pidgeotto_back:
            ypokemon_rattata_back;
    canvas_draw_xbm(c,4+player_dx,32,32,32,back);
    if(player_cover) {
        canvas_set_color(c,ColorWhite);
        canvas_draw_box(c,3,32,39,player_cover>=32?32:player_cover);
        canvas_set_color(c,ColorBlack);
    }
    canvas_set_bitmap_mode(c,true);
    const uint8_t active=g->active_party;
    const YSpeciesInfo* mon=y_species_info(yellow_party_species(g,active));
    snprintf(line,sizeof(line),"%s L%u",mon?mon->name:"???",
             (unsigned)yellow_party_level(g,active));
    canvas_draw_str(c,49,45,line);
    uint8_t current=yellow_party_hp(g,active);
    uint8_t capacity=yellow_party_max_hp(g,active);
    canvas_draw_frame(c,49,48,77,5);
    if(current&&capacity)
        canvas_draw_box(c,51,50,current*73/capacity,1);
    snprintf(line,sizeof(line),"HP %u/%u",(unsigned)yellow_party_hp(g,active),
             (unsigned)yellow_party_max_hp(g,active));
    canvas_draw_str(c,49,57,line);
    canvas_draw_str(c,85,63,"OK:MOVE");
    if(g->last_critical)canvas_draw_str(c,44,63,"CRIT");
    else if(yellow_party_status(g,active)!=YStatusNone)
        canvas_draw_str(c,56,63,battle_status_tag(yellow_party_status(g,active)));
    if(g->foe_status!=YStatusNone) {
        canvas_draw_str(c,1,25,"FOE");
        canvas_draw_str(c,23,25,battle_status_tag(g->foe_status));
    }
}
static void render_battle(Canvas* c,const YellowGame* g) {
    render_battle_motion(c,g,0,0,0,0);
}
static void render_fight_menu(Canvas* c,const YellowGame* g) {
    canvas_set_font(c,FontSecondary);
    uint8_t n_moves=yellow_active_move_count(g);
    uint8_t last=(uint8_t)(n_moves+2u);
    char headline[25];
    snprintf(headline,sizeof(headline),"ACTION %u/%u",
             (unsigned)(g->selected+1u),(unsigned)(last+1u));
    canvas_draw_str(c,2,10,headline);
    canvas_draw_line(c,0,13,127,13);
    uint8_t first=g->selected>2?(uint8_t)(g->selected-2):0;
    if(first+4u>last+1u)first=last>=3?(uint8_t)(last-3u):0;
    for(uint8_t i=first;i<=last&&i<first+4u;i++) {
        int y=24+11*(int)(i-first);
        if(i==g->selected)canvas_draw_str(c,1,y,">");
        if(i==n_moves) {
            char msg[27];
            if(g->foe_kind==YK_EEVEE)snprintf(msg,sizeof(msg),"BALL (TRAINER)");
            else if(yellow_party_size(g)>=YELLOW_PARTY_MAX)
                snprintf(msg,sizeof(msg),"BALL (FULL)");
            else snprintf(msg,sizeof(msg),"BALL x%u",(unsigned)g->balls);
            canvas_draw_str(c,11,y,msg);
        } else if(i==n_moves+1u) {
            canvas_draw_str(c,11,y,g->foe_kind?"RUN":"RUN (NO)");
        } else if(i==last) {
            canvas_draw_str(c,11,y,"SWITCH");
        } else {
            uint8_t move=yellow_party_move(g,g->active_party,i);
            const YMoveInfo* info=y_move_info(move);
            if(info) {
                canvas_draw_str(c,11,y,info->name);
                char pp[8];
                snprintf(pp,sizeof(pp),"%u",
                         (unsigned)yellow_party_pp(g,g->active_party,i));
                canvas_draw_str(c,115,y,pp);
            }
        }
    }
}
static void render_move_forget(Canvas* c,const YellowGame* g) {
    canvas_set_font(c,FontSecondary);
    if(g->learn_index>=g->learn_count) {
        canvas_draw_str(c,3,11,"LEARNING COMPLETE");
        return;
    }
    uint8_t q=g->learn_index;
    uint8_t party=g->learn_party[q];
    uint8_t new_move=g->learn_moves[q];
    const YSpeciesInfo* who=y_species_info(yellow_party_species(g,party));
    const YMoveInfo* next=y_move_info(new_move);
    char line[32];
    snprintf(line,sizeof(line),"%s LEARNS",who?who->name:"POKEMON");
    canvas_draw_str(c,2,9,line);
    snprintf(line,sizeof(line),"%s  (%u/%u)",next?next->name:"???",
             (unsigned)(q+1),(unsigned)g->learn_count);
    canvas_draw_str(c,2,19,line);
    for(uint8_t row=0;row<3;row++) {
        uint8_t entry=(uint8_t)((g->selected/3u)*3u+row);
        if(entry>YELLOW_MOVE_SLOTS)break;
        int y=32+11*row;
        if(entry==g->selected)canvas_draw_str(c,1,y,">");
        if(entry==YELLOW_MOVE_SLOTS) {
            canvas_draw_str(c,12,y,"KEEP OLD / SKIP");
        } else {
            const YMoveInfo* old=y_move_info(yellow_party_move(g,party,entry));
            canvas_draw_str(c,12,y,old?old->name:"(EMPTY)");
        }
    }
    canvas_draw_str(c,2,63,"OK REPLACE / BACK SKIP");
}
/* Short, nonblocking combat feedback. Purely visual: no extra damage,
   no changes to battle order, PP, status or saved game data. */
static void render_attack_fx(Canvas* c,const YellowApp* a) {
    if(!a->fx_active || !a->pending_attack)return;
    uint32_t age=furi_get_tick()-a->fx_start;
    if(age>=440)return;
    int cx=105,cy=18;
    int t=(int)((age%440)/55);
    canvas_set_bitmap_mode(c,true);
    /* Wind-up, strike and crackle; unlike R04 the attacker moves visibly. */
    if(age>=175 && age<410) {
        int flash=(int)(age/48);
        if((flash&1)==0) {
            canvas_draw_line(c,cx-14,cy-15,cx+14,cy+10);
            canvas_draw_line(c,cx-12,cy+11,cx+14,cy-14);
            canvas_draw_circle(c,cx,cy,7+(int)(age%80)/12);
        }
    }
    if(a->fx_type==YT_ELECTRIC) {
        int off=t&1?3:-3;
        canvas_draw_line(c,cx-10+off,cy-11,cx-2+off,cy-3);
        canvas_draw_line(c,cx-2+off,cy-3,cx-7+off,cy+4);
        canvas_draw_line(c,cx-7+off,cy+4,cx+8+off,cy+11);
        canvas_draw_line(c,cx+5-off,cy-8,cx+10-off,cy-2);
    } else if(a->fx_type==YT_FIRE) {
        for(int i=0;i<4;i++) {
            int xx=cx-8+i*5+(t&1)*2, yy=cy+8-i*4-t*2;
            canvas_draw_circle(c,xx,yy,1+(i%2));
        }
    } else if(a->fx_type==YT_GRASS||a->fx_type==YT_POISON) {
        canvas_draw_circle(c,cx,cy,3+t*3);
        canvas_draw_circle(c,cx,cy,1+t);
    } else {
        int delta=t%2?4:0;
        canvas_draw_line(c,cx-12+delta,cy-9,cx+9+delta,cy+9);
        canvas_draw_line(c,cx-9+delta,cy-12,cx+12+delta,cy+7);
    }
}

static void draw_ball(Canvas* c,int x,int y,int wobble) {
    canvas_set_bitmap_mode(c,true);
    canvas_set_color(c,ColorBlack);
    canvas_draw_circle(c,x+wobble,y,5);
    canvas_draw_line(c,x-4+wobble,y,x+4+wobble,y);
    canvas_draw_disc(c,x+wobble,y,1);
    canvas_draw_line(c,x-2+wobble,y-3,x+2+wobble,y-3);
}

/* A single timed visual pipeline, independent of game rules and save state.
   Every scene is advanced by elapsed monotonic milliseconds. */
static void render_scene(Canvas* c,const YellowApp* a) {
    uint32_t age=furi_get_tick()-a->scene_start;
    canvas_set_font(c,FontSecondary);
    if(a->scene==SceneBattleIntro) {
        render_battle(c,&a->scene_before);
        canvas_set_bitmap_mode(c,true);
        canvas_set_color(c,ColorWhite);
        uint32_t reveal=age>=390?40:(age*40/390);
        if(reveal<40)canvas_draw_box(c,85,0,43,40-(int)reveal);
        /* Do not show the player's monster fully formed in frame zero.
           Reveal its actual sprite after the opposing Pokemon enters. */
        if(age<420)canvas_draw_box(c,0,31,38,33);
        else if(age<570 && ((age/55)&1)==0)
            canvas_draw_box(c,0,31,38,33);
        canvas_set_color(c,ColorBlack);
        canvas_draw_frame(c,0,52,128,12);
        canvas_draw_str(c,4,61,age>=420?"GO! YOUR POKEMON!":
            a->scene_before.foe_kind==YK_EEVEE?"RIVAL WANTS TO FIGHT!":"A WILD POKEMON!");
        return;
    }
    if(a->scene==SceneOakCapture) {
        canvas_clear(c);
        canvas_set_color(c,ColorBlack);
        if(age<700) {
            canvas_set_bitmap_mode(c,false);
            canvas_draw_xbm(c,87,8,40,40,ypokemon_pikachu_front);
            canvas_set_bitmap_mode(c,true);
        }
        canvas_draw_str(c,2,10,age<550?"OAK: WILD PIKACHU!":
                             age<1450?"OAK THROWS A BALL!":"CLICK! OAK CAUGHT IT!");
        if(age>=250) {
            int bx=age<730?12+(int)(age-250)*89/480:100;
            int by=age<730?54-(int)(age-250)*15/480:50;
            int offset=age>840&&age<1450?(((age-840)/100)&1?3:-3):0;
            draw_ball(c,bx,by,offset);
        }
        if(age>=1650)canvas_draw_str(c,4,62,"OK: OAK'S LAB NEXT");
        else canvas_draw_str(c,4,62,"CAPTURING...");
        return;
    }
    if(a->scene==SceneWildCapture) {
        render_battle(c,&a->scene_before);
        canvas_set_color(c,ColorWhite);
        if(age>=420 && (age<1450 || a->capture_success)) {
            canvas_draw_box(c,85,0,43,40);
        }
        canvas_set_color(c,ColorBlack);
        if(age<420) {
            int bx=10+(int)age*90/420;
            int by=52-(int)age*29/420;
            draw_ball(c,bx,by,0);
        } else if(age<1600) {
            int wobble=age<680?0:(((age-680)/125)&1?3:-3);
            draw_ball(c,103,49,wobble);
            if(age>=580 && age<1450) {
                canvas_draw_str(c,5,27,"SHAKE... SHAKE...");
            }
        }
        if(age>=1450) {
            canvas_draw_str(c,3,27,a->capture_success?
                    "CLICK! GOTCHA!":"OH! IT BROKE FREE!");
            if(!a->capture_success) {
                canvas_draw_circle(c,103,49,8);
                canvas_draw_line(c,97,43,110,56);
            }
        }
        return;
    }
    if(a->scene==SceneFaint) {
        YellowGame last=a->scene_before;
        last.foe_hp=a->hp_foe_end;
        if(a->faint_enemy) {
            uint8_t crop=(uint8_t)(age>=400?40:(age*40/400));
            render_battle_motion(c,&last,0,0,crop,0);
            canvas_draw_str(c,4,29,"FOE FAINTED!");
        } else {
            uint8_t crop=(uint8_t)(age>=400?32:(age*32/400));
            render_battle_motion(c,&last,0,0,0,crop);
            canvas_draw_str(c,4,29,"YOUR POKEMON FAINTED!");
        }
        return;
    }
    if(a->scene==SceneHpDrain) {
        YellowGame visual=a->scene_before;
        uint32_t elapsed=age>690?690:age;
        uint16_t alpha=(uint16_t)(elapsed*256/690);
        if(visual.foe_hp!=a->hp_foe_end)
            visual.foe_hp=(uint8_t)ux_interpolate(
                visual.foe_hp,a->hp_foe_end,alpha);
        uint8_t active=visual.active_party;
        uint8_t hp_before=yellow_party_hp(&visual,active);
        uint8_t hp_after=a->hp_party_end;
        uint8_t progress=(uint8_t)ux_interpolate(hp_before,hp_after,alpha);
        if(active==0)visual.hp=progress;
        else visual.caught[active-1].hp=progress;
        int foe_shake=age<240?((age/55)&1?3:-3):0;
        int player_shake=age<240?((age/55)&1?-2:2):0;
        render_battle_motion(c,&visual,
            a->hp_party_end<hp_before?player_shake:0,
            a->hp_foe_end<a->scene_before.foe_hp?foe_shake:0,0,0);
        canvas_set_color(c,ColorBlack);
        if(age<290) {
            int cx=(a->hp_foe_end<a->scene_before.foe_hp)?107:20;
            int cy=(cx==107)?18:45;
            int dir=(age/75)&1?2:-2;
            for(int i=0;i<5;i++) {
                int dx=(int)(i*7)-14+dir;
                canvas_draw_line(c,cx+dx,cy-10,cx+dx+8,cy+6);
            }
            canvas_draw_str(c,74,63,"HIT!");
        }
        if(age>=300 && age<850)canvas_draw_str(c,1,31,"HP...");
    }
}

static void draw_cb(Canvas* c,void* context) {
    YellowApp* a=context;
    if(furi_mutex_acquire(a->lock,FuriWaitForever)!=FuriStatusOk)return;
    canvas_clear(c);
    canvas_set_color(c,ColorBlack);
    /* A present but corrupt Oak Parcel SD record must never be silently
     * interpreted as a new quest. Show a one-time visible warning; the
     * acknowledgement only dismisses UI, never changes the save guard.
     * Back/OK continue in read-only-for-save mode. Recovery is external.
     */
    if(a->g.oak_parcel_integrity_error && !a->parcel_warning_acknowledged) {
        canvas_set_font(c,FontSecondary);
        canvas_draw_frame(c,0,0,128,64);
        canvas_draw_str(c,3,10,"SAVE DATA WARNING");
        canvas_draw_str(c,3,23,"PARCEL RECORD BAD");
        canvas_draw_str(c,3,36,"SAVING DISABLED");
        canvas_draw_str(c,3,49,"SD DATA UNCHANGED");
        canvas_draw_str(c,3,61,"OK/BACK: CONTINUE");
        furi_mutex_release(a->lock);
        return;
    }
    if(a->scene!=SceneNone) {
        render_scene(c,a);
        furi_mutex_release(a->lock);
        return;
    }
    switch(a->g.screen){
    case YScreenWorld:render_world(c,a);break;
    case YScreenDialog:render_world(c,a);render_dialog(c,&a->g);break;
    case YScreenNurseChoice:render_nurse_choice(c,a);break;
    case YScreenMenu:render_menu(c,&a->g);break;
    case YScreenParty:render_info(c,&a->g);break;
    case YScreenBag:render_bag(c,a);break;
    case YScreenPC:render_pc(c,&a->g);break;
    case YScreenPokedex:render_pokedex(c,&a->g);break;
    case YScreenLinkOffline:render_link_offline(c,&a->g);break;
    case YScreenOakEncounter:render_oak_capture(c);break;
    case YScreenRivalBattle:render_battle(c,&a->g);render_attack_fx(c,a);break;
    case YScreenFightMenu:
        if(a->pending_attack) {
            uint32_t age=furi_get_tick()-a->fx_start;
            int shift=(age<120)?(int)(age/25):
                (age<250?6:(age<390?3:0));
            render_battle_motion(c,&a->g,shift,0,0,0);
            render_attack_fx(c,a);
        } else render_fight_menu(c,&a->g);
        break;
    case YScreenMoveForget:render_move_forget(c,&a->g);break;
    }
    if(a->audio_debug) {
        char diag[32];
        snprintf(diag,sizeof(diag),"M:%u H:%u S:%u",
                 (unsigned)a->audio_debug_track,
                 (unsigned)a->audio_last_hz,
                 (unsigned)a->pallet_speaker_owned);
        canvas_set_color(c,ColorWhite);
        canvas_draw_box(c,0,54,106,10);
        canvas_set_color(c,ColorBlack);
        canvas_set_font(c,FontSecondary);
        canvas_draw_str(c,1,63,diag);
    }
    furi_mutex_release(a->lock);
}
static void input_cb(InputEvent* event,void* context) {
    YellowApp* a=context;
    /* Hardware emits Press, Release, and then Short for a quick tap.
       Repeats are advisory; continuous motion is driven by our own clock. */
    if((event->type==InputTypeLong && event->key==InputKeyOk) ||
       event->type==InputTypeShort||event->type==InputTypeRepeat||
       ((event->key==InputKeyUp||event->key==InputKeyDown||
         event->key==InputKeyLeft||event->key==InputKeyRight) &&
        (event->type==InputTypePress||event->type==InputTypeRelease)))
        (void)furi_message_queue_put(a->queue,event,0);
}
/* Read a self-contained, sealed generation; incomplete files fail closed.
 * Never read orphaned new-format sidecars from legacy names. */
static bool atomic_read_slot(Storage* storage,const char* path,
                              YellowAtomicSnapshot* out) {
    if(!out)return false;
    File* f=storage_file_alloc(storage);
    bool ok=storage_file_open(f,path,FSAM_READ,FSOM_OPEN_EXISTING);
    if(ok)ok=storage_file_read(f,out,sizeof(*out))==sizeof(*out);
    /* Reject appended bytes: a sealed snapshot must have exact length. */
    if(ok) {
        uint8_t trailing=0;
        ok=storage_file_read(f,&trailing,1u)==0;
    }
    if(storage_file_is_open(f) && !storage_file_close(f))ok=false;
    storage_file_free(f);
    return ok && yellow_atomic_unpack(out,NULL);
}
static bool atomic_write_inactive(Storage* storage,const char* path,
                                  const YellowAtomicSnapshot* record) {
    if(!record || !yellow_atomic_unpack(record,NULL))return false;
    /* Commit marker physically issued as last separate write. A short
       write or interrupted record must fail the checksum/seal gate.
       SD/FAT power-loss durability beyond this remains unverified. */
    File* f=storage_file_alloc(storage);
    bool ok=storage_file_open(f,path,FSAM_WRITE,FSOM_CREATE_ALWAYS);
    const size_t bytes=offsetof(YellowAtomicSnapshot,committed);
    if(ok)ok=storage_file_write(f,record,bytes)==bytes;
    if(ok)ok=storage_file_write(f,&record->committed,
                                  sizeof(record->committed))==
                                  sizeof(record->committed);
    if(storage_file_is_open(f) && !storage_file_close(f))ok=false;
    storage_file_free(f);
    return ok;
}
/* Two independent tiny sidecars mirror the existing atomic slot names.
 * Seal field emitted as the final write; exact sizeof is required on read.
 */
static bool exact_position_read(Storage* storage,const char* path,
                                YellowExactPositionV1* out) {
    if(!out)return false;
    File* f=storage_file_alloc(storage);
    bool ok=storage_file_open(f,path,FSAM_READ,FSOM_OPEN_EXISTING);
    if(ok)ok=storage_file_read(f,out,sizeof(*out))==sizeof(*out);
    /* The next read must hit EOF. This checks exact record length without
     * relying on a 64-bit storage_file_size() ABI in the ARM emulator. */
    if(ok) {
        uint8_t trailing=0;
        ok=storage_file_read(f,&trailing,1u)==0;
    }
    if(storage_file_is_open(f) && !storage_file_close(f))ok=false;
    storage_file_free(f);
    return ok;
}
static bool exact_position_write(Storage* storage,const char* path,
                                 const YellowExactPositionV1* p) {
    if(!p || p->seal!=YELLOW_EXACT_POSITION_SEAL ||
       p->checksum!=yellow_exact_position_hash(p))return false;
    File* f=storage_file_alloc(storage);
    bool ok=storage_file_open(f,path,FSAM_WRITE,FSOM_CREATE_ALWAYS);
    const size_t first=offsetof(YellowExactPositionV1,seal);
    if(ok)ok=storage_file_write(f,p,first)==first;
    if(ok)ok=storage_file_write(f,&p->seal,sizeof(p->seal))==sizeof(p->seal);
    if(storage_file_is_open(f) && !storage_file_close(f))ok=false;
    storage_file_free(f);
    return ok;
}
/* Per-generation quest record. An interrupted write cannot affect the
 * currently selected sealed parent; only the inactive slot is overwritten.
 * The slot generation AND parent checksum must match before restoring flags.
 */
static bool oak_parcel_slot_read(Storage* storage,const char* path,
                                 YOakParcelSlot* out){
    if(!out)return false;
    File* f=storage_file_alloc(storage);
    bool ok=storage_file_open(f,path,FSAM_READ,FSOM_OPEN_EXISTING);
    if(ok)ok=storage_file_read(f,out,sizeof(*out))==sizeof(*out);
    if(ok){uint8_t extra=0;ok=storage_file_read(f,&extra,1u)==0;}
    if(storage_file_is_open(f) && !storage_file_close(f))ok=false;
    storage_file_free(f);
    return ok;
}
static bool oak_parcel_slot_write(Storage* storage,const char* path,
                                  const YOakParcelSlot* record){
    if(!record)return false;
    uint8_t check=0;
    if(!y_oak_parcel_slot_restore(&check,record->parcel.parent_checksum,
                                  record->generation,record))return false;
    File* f=storage_file_alloc(storage);
    bool ok=storage_file_open(f,path,FSAM_WRITE,FSOM_CREATE_ALWAYS);
    const size_t prefix=offsetof(YOakParcelSlot,seal);
    if(ok)ok=storage_file_write(f,record,prefix)==prefix;
    if(ok)ok=storage_file_write(f,&record->seal,sizeof(record->seal))==sizeof(record->seal);
    if(storage_file_is_open(f) && !storage_file_close(f))ok=false;
    storage_file_free(f);
    return ok;
}
/* The per-generation anchor is optional for legacy saves. New saves
 * ALWAYS write 0 (unset) or 1 (Viridian) to the inactive slot before
 * committing the parent, so an old stale slot cannot resurrect an anchor.
 */
static bool whiteout_slot_read(Storage* storage,const char* path,YWhiteoutSlot* out) {
    if(!out)return false;
    File* f=storage_file_alloc(storage);
    bool ok=storage_file_open(f,path,FSAM_READ,FSOM_OPEN_EXISTING);
    if(ok)ok=storage_file_read(f,out,sizeof(*out))==sizeof(*out);
    if(ok){uint8_t extra=0;ok=storage_file_read(f,&extra,1u)==0;}
    if(storage_file_is_open(f) && !storage_file_close(f))ok=false;
    storage_file_free(f);return ok;
}
static bool whiteout_slot_write(Storage* storage,const char* path,const YWhiteoutSlot* record) {
    if(!record)return false;
    uint8_t verified=255u;
    if(!y_whiteout_slot_restore(&verified,record->parent_checksum,record->generation,record))return false;
    File* f=storage_file_alloc(storage);
    bool ok=storage_file_open(f,path,FSAM_WRITE,FSOM_CREATE_ALWAYS);
    const size_t prefix=offsetof(YWhiteoutSlot,seal);
    if(ok)ok=storage_file_write(f,record,prefix)==prefix;
    if(ok)ok=storage_file_write(f,&record->seal,sizeof(record->seal))==sizeof(record->seal);
    if(storage_file_is_open(f) && !storage_file_close(f))ok=false;
    storage_file_free(f);return ok;
}
/* Exact 24-byte item slot, seal last; generation+parent checksum are
 * verified by the codec. Never accept trailing bytes or truncated records.
 * A missing slot is legacy stock zero, NOT a claim of an existing item.
 */
static bool antidote_slot_read(Storage* storage,const char* path,
                               YellowAntidoteInventorySlot* out) {
    if(!out)return false;
    File* f=storage_file_alloc(storage);
    bool ok=storage_file_open(f,path,FSAM_READ,FSOM_OPEN_EXISTING);
    if(ok)ok=storage_file_read(f,out,sizeof(*out))==sizeof(*out);
    if(ok){uint8_t extra=0;ok=storage_file_read(f,&extra,1u)==0;}
    if(storage_file_is_open(f) && !storage_file_close(f))ok=false;
    storage_file_free(f);
    return ok;
}
static bool antidote_slot_write(Storage* storage,const char* path,
                                const YellowAntidoteInventorySlot* record) {
    if(!record)return false;
    uint8_t stock=0;
    /* Read little-endian parent checksum and generation from the wire
     * and independently validate the codec before touching the SD slot. */
    const uint8_t* b=record->wire;
    uint32_t parent=0,generation=0;
    for(unsigned i=0;i<4;i++) {
        parent|=(uint32_t)b[8+i]<<(8u*i);
        generation|=(uint32_t)b[12+i]<<(8u*i);
    }
    if(!yellow_antidote_inventory_restore(record,parent,generation,&stock))return false;
    File* f=storage_file_alloc(storage);
    bool ok=storage_file_open(f,path,FSAM_WRITE,FSOM_CREATE_ALWAYS);
    if(ok)ok=storage_file_write(f,record->wire,20u)==20u;
    if(ok)ok=storage_file_write(f,record->wire+20u,4u)==4u;
    if(storage_file_is_open(f) && !storage_file_close(f))ok=false;
    storage_file_free(f);
    return ok;
}
/* Paired funds+inventory: exact 28-byte wire, checksum and seal-last.
 * Both are committed to the inactive generation before its parent snapshot.
 * A corrupt present record never silently becomes 3000 starting funds.
 */
static bool money_stock_slot_read(Storage* storage,const char* path,
                                  YellowMoneyStockSlot* out) {
    if(!out)return false;
    File* f=storage_file_alloc(storage);
    bool ok=storage_file_open(f,path,FSAM_READ,FSOM_OPEN_EXISTING);
    if(ok)ok=storage_file_read(f,out,sizeof(*out))==sizeof(*out);
    if(ok){uint8_t extra=0;ok=storage_file_read(f,&extra,1u)==0;}
    if(storage_file_is_open(f) && !storage_file_close(f))ok=false;
    storage_file_free(f);
    return ok;
}
static bool money_stock_slot_write(Storage* storage,const char* path,
                                   const YellowMoneyStockSlot* slot) {
    if(!slot)return false;
    const uint8_t* b=slot->wire;
    uint32_t parent=0,generation=0;
    for(unsigned i=0;i<4u;i++) {
        parent|=(uint32_t)b[12u+i]<<(i*8u);
        generation|=(uint32_t)b[16u+i]<<(i*8u);
    }
    uint8_t money[3],stock=0;
    if(!yellow_money_stock_restore(slot,parent,generation,money,&stock))
        return false;
    File* f=storage_file_alloc(storage);
    bool ok=storage_file_open(f,path,FSAM_WRITE,FSOM_CREATE_ALWAYS);
    if(ok)ok=storage_file_write(f,b,24u)==24u;
    if(ok)ok=storage_file_write(f,b+24u,4u)==4u;
    if(storage_file_is_open(f) && !storage_file_close(f))ok=false;
    storage_file_free(f);
    return ok;
}
/* A v4 parent is not a recoverable generation without its required
 * generation-bound money+stock and inventory records. Evaluate BOTH slots
 * before selecting the winner, including when preparing the next save. */
static bool atomic_generation_complete(Storage* storage,
                                       const YellowAtomicSnapshot* parent,
                                       bool slot_a) {
    if(!parent || !yellow_atomic_unpack(parent,NULL))return false;
    if(parent->version<4u)return true; /* legacy migration stays fail-closed */
    const char* item_path=slot_a?ANTIDOTE_SAVE_A:ANTIDOTE_SAVE_B;
    const char* money_path=slot_a?MONEY_STOCK_SAVE_A:MONEY_STOCK_SAVE_B;
    YellowAntidoteInventorySlot item={{0}};
    YellowMoneyStockSlot pair={{0}};
    uint8_t stock=0,pair_stock=0,money[3];
    const char* quest_path=slot_a?OAK_PARCEL_SAVE_A:OAK_PARCEL_SAVE_B;
    const char* anchor_path=slot_a?WHITEOUT_SAVE_A:WHITEOUT_SAVE_B;
    YOakParcelSlot quest={0};
    YWhiteoutSlot anchor={0};
    uint8_t quest_flags=0,blackout_map=0;
    return oak_parcel_slot_read(storage,quest_path,&quest) &&
           y_oak_parcel_slot_restore(&quest_flags,parent->parent.checksum,
                                     parent->generation,&quest) &&
           whiteout_slot_read(storage,anchor_path,&anchor) &&
           y_whiteout_slot_restore(&blackout_map,parent->parent.checksum,
                                   parent->generation,&anchor) &&
           antidote_slot_read(storage,item_path,&item) &&
           yellow_antidote_inventory_restore(&item,parent->parent.checksum,
                                             parent->generation,&stock) &&
           money_stock_slot_read(storage,money_path,&pair) &&
           yellow_money_stock_restore(&pair,parent->parent.checksum,
                                      parent->generation,money,&pair_stock) &&
           stock==pair_stock;
}
static bool save_game(Storage* storage,YellowGame* g) {
    /* Never erase or replace an untrusted key-item state by saving zero flags.
     * The parent remains playable, but further saves require explicit repair.
     */
    if(!storage||!g||!y_oak_parcel_runtime_safe(g->oak_parcel_flags,g->oak_parcel_integrity_error) ||
       g->last_blackout_integrity_error || g->last_blackout_original_map>1u ||
       g->antidote_integrity_error || g->antidotes>99u ||
       g->money_integrity_error)return false;
    storage_common_mkdir(storage,SAVE_DIR);
    YellowAtomicSnapshot* a=malloc(sizeof(*a));
    YellowAtomicSnapshot* b=malloc(sizeof(*b));
    YellowAtomicSnapshot* next=malloc(sizeof(*next));
    if(!a||!b||!next) {
        free(a);free(b);free(next);
        return false;
    }
    bool valid_a=atomic_read_slot(storage,ATOMIC_SAVE_A,a) &&
                 atomic_generation_complete(storage,a,true);
    bool valid_b=atomic_read_slot(storage,ATOMIC_SAVE_B,b) &&
                 atomic_generation_complete(storage,b,false);
    int selected=yellow_atomic_select(valid_a?a:NULL,valid_b?b:NULL);
    uint32_t generation=selected<0?1u:
       (selected==0?a->generation:b->generation)+1u;
    if(!generation)generation=1u;
    const char* dest=selected==0?ATOMIC_SAVE_B:ATOMIC_SAVE_A;
    bool ok=yellow_atomic_make(g,generation,next);
    /* Position sidecar is written to the INACTIVE slot before its parent
     * atomic record is sealed. An interrupted write leaves the previous
     * slot usable; a truncated/missing sidecar can never corrupt v13 data.
     * No position sidecar needed for old maps 0..10. */
    const char* pos_dest=selected==0?POSITION_SAVE_B:POSITION_SAVE_A;
    if(ok && ((g->map>=YMapPewterCity && g->map<=YMapViridianMart) || g->map==YMapCeruleanMart || g->map==YMapRoute10)) {
        YellowExactPositionV1 exact;
        ok=yellow_exact_position_make(g,next,&exact);
        if(ok)ok=exact_position_write(storage,pos_dest,&exact);
        if(ok) {
            YellowExactPositionV1 verify_pos={0};
            ok=exact_position_read(storage,pos_dest,&verify_pos)
               && memcmp(&exact,&verify_pos,sizeof(exact))==0;
        }
    }
    /* A complete, checked parcel sidecar is committed to the inactive slot
     * before its matching atomic parent is sealed. Never mutate v13 bytes.
     * The quest flags remain zero until genuine original gameplay implements
     * acquisition/delivery; this only persists an independently set state.
     */
    if(ok){
        YOakParcelSlot quest={0},verified={0};
        const char* path=selected==0?OAK_PARCEL_SAVE_B:OAK_PARCEL_SAVE_A;
        ok=y_oak_parcel_slot_make(g->oak_parcel_flags,next->parent.checksum,
                                   generation,&quest);
        if(ok)ok=oak_parcel_slot_write(storage,path,&quest);
        if(ok)ok=oak_parcel_slot_read(storage,path,&verified) &&
                  memcmp(&quest,&verified,sizeof(quest))==0;
    }
    if(ok){
        YWhiteoutSlot anchor={0},checked_anchor={0};
        const char* path=selected==0?WHITEOUT_SAVE_B:WHITEOUT_SAVE_A;
        ok=y_whiteout_slot_make(g->last_blackout_original_map,
                                 next->parent.checksum,generation,&anchor);
        if(ok)ok=whiteout_slot_write(storage,path,&anchor);
        if(ok)ok=whiteout_slot_read(storage,path,&checked_anchor) &&
                  memcmp(&anchor,&checked_anchor,sizeof(anchor))==0;
    }
    if(ok){
        YellowAntidoteInventorySlot item={{0}},checked_item={{0}};
        const char* path=selected==0?ANTIDOTE_SAVE_B:ANTIDOTE_SAVE_A;
        ok=yellow_antidote_inventory_create(g->antidotes,next->parent.checksum,
                                              generation,&item);
        if(ok)ok=antidote_slot_write(storage,path,&item);
        if(ok)ok=antidote_slot_read(storage,path,&checked_item) &&
                  memcmp(&item,&checked_item,sizeof(item))==0;
    }
    /* One indivisible payload for money and stock. Never seal the parent
     * unless the paired payload has passed readback verification. */
    if(ok) {
        YellowMoneyStockSlot pair={{0}},checked_pair={{0}};
        const char* path=selected==0?MONEY_STOCK_SAVE_B:MONEY_STOCK_SAVE_A;
        ok=yellow_money_stock_create(g->money_bcd,g->antidotes,
                                      next->parent.checksum,generation,&pair);
        if(ok)ok=money_stock_slot_write(storage,path,&pair);
        if(ok)ok=money_stock_slot_read(storage,path,&checked_pair) &&
                  memcmp(&pair,&checked_pair,sizeof(pair))==0;
    }
    if(ok)ok=atomic_write_inactive(storage,dest,next);
    /* Read-after-write verifies storage bytes, not physical fsync.
       Never overwrite the last valid generation as part of a save. */
    if(ok) {
        YellowAtomicSnapshot* checked=selected==0?a:b;
        ok=atomic_read_slot(storage,dest,checked) &&
           memcmp(next,checked,sizeof(*next))==0;
    }
    free(a);free(b);free(next);
    return ok;
}
static bool read_saved_file(Storage* storage,const char* path,YellowSave* out) {
    File* f=storage_file_alloc(storage);
    bool ok=storage_file_open(f,path,FSAM_READ,FSOM_OPEN_EXISTING);
    if(ok)ok=storage_file_read(f,out,sizeof(*out))==sizeof(*out);
    if(storage_file_is_open(f) && !storage_file_close(f))ok=false;
    storage_file_free(f);
    return ok;
}
static bool load_game(Storage* storage,YellowGame* g) {
    /* Prefer newest complete, valid generation. A torn latest slot
       falls back to prior intact generation; if neither is usable,
       legacy v13 sidecars below are READ-ONLY migration fallback. */
    YellowAtomicSnapshot* a=malloc(sizeof(*a));
    YellowAtomicSnapshot* b=malloc(sizeof(*b));
    if(a&&b) {
        bool valid_a=atomic_read_slot(storage,ATOMIC_SAVE_A,a) &&
                       atomic_generation_complete(storage,a,true);
        bool valid_b=atomic_read_slot(storage,ATOMIC_SAVE_B,b) &&
                       atomic_generation_complete(storage,b,false);
        int winner=yellow_atomic_select(valid_a?a:NULL,valid_b?b:NULL);
        if(winner<0) {
            /* No complete v4 generation: retain the existing fail-closed
             * partial-parent path, which raises integrity flags instead of
             * silently loading starting funds or migrating stale saves. */
            bool parent_a=atomic_read_slot(storage,ATOMIC_SAVE_A,a);
            bool parent_b=atomic_read_slot(storage,ATOMIC_SAVE_B,b);
            winner=yellow_atomic_select(parent_a?a:NULL,parent_b?b:NULL);
        }
        if(winner>=0) {
            const YellowAtomicSnapshot* parent=winner==0?a:b;
            bool ok=yellow_atomic_unpack(parent,g);
            if(ok) {
                /* Legacy atomic v1 records have no exact-position sidecar,
                 * so old fallback Route2 remains valid and loadable.
                 * Invalid/mismatched/truncated newer records are ignored
                 * (never partially applied), protecting parent progress.
                 */
                YellowExactPositionV1 exact={0};
                const char* path=winner==0?POSITION_SAVE_A:POSITION_SAVE_B;
                if(exact_position_read(storage,path,&exact))
                    (void)yellow_exact_position_restore(g,parent,&exact);
                YOakParcelSlot quest={0};
                const char* quest_path=winner==0?OAK_PARCEL_SAVE_A:OAK_PARCEL_SAVE_B;
                if(storage_file_exists(storage,quest_path)) {
                    /* A missing optional record is valid for old saves. A
                     * PRESENT corrupt/truncated/mismatched record must not
                     * become "never acquired" and allow item duplication.
                     * Keep the valid parent loaded but block unsafe resaves.
                     */
                    if(!oak_parcel_slot_read(storage,quest_path,&quest) ||
                       !y_oak_parcel_slot_restore(&g->oak_parcel_flags,
                           parent->parent.checksum,parent->generation,&quest))
                        g->oak_parcel_integrity_error=true;
                }
                YellowAntidoteInventorySlot item={{0}};
                const char* item_path=winner==0?ANTIDOTE_SAVE_A:ANTIDOTE_SAVE_B;
                if(storage_file_exists(storage,item_path)) {
                    if(!antidote_slot_read(storage,item_path,&item) ||
                       !yellow_antidote_inventory_restore(&item,
                            parent->parent.checksum,parent->generation,&g->antidotes))
                        g->antidote_integrity_error=true;
                } else if(parent->version>=3u) {
                    /* Preserve development-r08 fail-closed rule: a v3 parent
                     * must never load missing Antidote stock as zero. */
                    g->antidote_integrity_error=true;
                }
                /* No legacy atomic parent (v1 or v3) records a reliable
                 * historical wallet. Never mint the 3000 starting coins
                 * on restore. A valid manually prepared, generation-bound
                 * pair is required before spending or saving. */
                YellowMoneyStockSlot pair={{0}};
                const char* pair_path=winner==0?MONEY_STOCK_SAVE_A:MONEY_STOCK_SAVE_B;
                if(storage_file_exists(storage,pair_path)) {
                    uint8_t money[3],stock=0;
                    if(!money_stock_slot_read(storage,pair_path,&pair) ||
                       !yellow_money_stock_restore(&pair,
                           parent->parent.checksum,parent->generation,
                           money,&stock) ||
                       !storage_file_exists(storage,item_path) ||
                       g->antidote_integrity_error || stock!=g->antidotes) {
                        g->money_integrity_error=true;
                        g->antidote_integrity_error=true;
                    } else memcpy(g->money_bcd,money,3u);
                } else {
                    /* v1 AND v3 lack historical wallet data. The default
                     * 3000 coins from yellow_init is NOT a restored balance.
                     * No purchase, Antidote use or save until an explicit,
                     * externally reviewed money+stock sidecar is supplied.
                     * This also rejects a torn v4 wallet fail-closed. */
                    g->money_integrity_error=true;
                    g->antidote_integrity_error=true;
                }
                YWhiteoutSlot anchor={0};
                const char* anchor_path=winner==0?WHITEOUT_SAVE_A:WHITEOUT_SAVE_B;
                if(storage_file_exists(storage,anchor_path)) {
                    if(!whiteout_slot_read(storage,anchor_path,&anchor) ||
                       !y_whiteout_slot_restore(&g->last_blackout_original_map,
                            parent->parent.checksum,parent->generation,&anchor))
                        g->last_blackout_integrity_error=true;
                }
            }
            free(a);free(b);
            return ok;
        }
    }
    free(a);free(b);
    YellowSave s;
    if(read_saved_file(storage,SAVE_PATH,&s)) {
        YellowItemSave item={0};
        File* item_file=storage_file_alloc(storage);
        bool has_item=storage_file_open(item_file,FOREST_ITEMS_PATH,
                                        FSAM_READ,FSOM_OPEN_EXISTING);
        bool item_ok=has_item && storage_file_read(item_file,&item,sizeof(item))==sizeof(item);
        if(storage_file_is_open(item_file))storage_file_close(item_file);
        storage_file_free(item_file);
        /* A corrupt item sidecar must not silently erase pickup progress. */
        if(has_item&&!item_ok)return false;
        /* Validate the optional gym sidecar BEFORE mutating the loaded game.
           Invalid or cross-save badges must never partially enter the scene. */
        YPewterProgressSave pewter={0};
        uint8_t loaded_pewter_flags=0;
        File* pewter_file=storage_file_alloc(storage);
        bool has_pewter=storage_file_open(
            pewter_file,PEWTER_PROGRESS_PATH,FSAM_READ,FSOM_OPEN_EXISTING);
        bool read_pewter=has_pewter &&
            storage_file_read(pewter_file,&pewter,sizeof(pewter))==sizeof(pewter);
        if(storage_file_is_open(pewter_file))storage_file_close(pewter_file);
        storage_file_free(pewter_file);
        if(has_pewter && (!read_pewter ||
            !y_pewter_progress_restore(&loaded_pewter_flags,s.checksum,&pewter)))
            return false;
        YellowRoute3TrainerSave route3={0};
        uint8_t loaded_route3_mask=0;
        File* route3_file=storage_file_alloc(storage);
        bool has_route3=storage_file_open(route3_file,ROUTE3_TRAINERS_PATH,
                                          FSAM_READ,FSOM_OPEN_EXISTING);
        bool read_route3=has_route3 &&
            storage_file_read(route3_file,&route3,sizeof(route3))==sizeof(route3);
        if(storage_file_is_open(route3_file))storage_file_close(route3_file);
        storage_file_free(route3_file);
        if(has_route3 && (!read_route3 ||
           !yellow_route3_trainer_restore(&loaded_route3_mask,s.checksum,&route3)))
            return false;
        YellowDexSave dex_sidecar={0};
        YellowDex loaded_dex={0};
        File* dex_file=storage_file_alloc(storage);
        bool has_dex=storage_file_open(dex_file,DEX_LEDGER_PATH,FSAM_READ,FSOM_OPEN_EXISTING);
        bool dex_read=has_dex &&
            storage_file_read(dex_file,&dex_sidecar,sizeof(dex_sidecar))==sizeof(dex_sidecar);
        if(storage_file_is_open(dex_file))storage_file_close(dex_file);
        storage_file_free(dex_file);
        if(has_dex && (!dex_read ||
           !yellow_dex_restore(&loaded_dex,s.checksum,&dex_sidecar)))return false;
        YellowPCSave pc_sidecar={0};
        YellowPCBox loaded_pc={0};
        File* pc_file=storage_file_alloc(storage);
        bool has_pc=storage_file_open(pc_file,PC_BOX_PATH,FSAM_READ,FSOM_OPEN_EXISTING);
        bool pc_read=has_pc &&
             storage_file_read(pc_file,&pc_sidecar,sizeof(pc_sidecar))==sizeof(pc_sidecar);
        if(storage_file_is_open(pc_file))storage_file_close(pc_file);
        storage_file_free(pc_file);
        if(has_pc && (!pc_read ||
           !yellow_pc_restore(&loaded_pc,s.checksum,&pc_sidecar)))return false;
        if(!yellow_save_load_with_items(g,&s,item_ok?&item:NULL))return false;
        YellowTrainerSave sidecar={0};
        File* f=storage_file_alloc(storage);
        bool ok=storage_file_open(f,FOREST_SAVE_PATH,FSAM_READ,FSOM_OPEN_EXISTING);
        if(ok)ok=storage_file_read(f,&sidecar,sizeof(sidecar))==sizeof(sidecar);
        if(storage_file_is_open(f))storage_file_close(f);
        storage_file_free(f);
        if(ok)(void)yellow_trainer_save_load(g,&s,&sidecar);
        g->pewter_flags=loaded_pewter_flags;
        g->route3_defeated_mask=loaded_route3_mask;
        if(has_pc){
            g->pc=loaded_pc;
            for(unsigned i=0;i<g->pc.count;i++)
                (void)yellow_dex_mark_caught(&g->dex,
                    yellow_dex_number(g->pc.slots[i].species));
        }
        if(has_dex) {
            /* A valid signed ledger is authoritative for previous sightings.
               Never lose party-owned entries even on manually migrated saves. */
            for(unsigned i=0;i<YELLOW_DEX_BYTES;i++) {
                g->dex.seen[i]|=loaded_dex.seen[i];
                g->dex.caught[i]|=loaded_dex.caught[i];
            }
        }
        return true;
    }
    /* R12-B has exactly the same serialized layout/version as R12-A.
       Read and validate it, then write a separate R12-B save file.
       The original R12 data remains untouched. */
    if(read_saved_file(storage,PRIOR_SAVE_PATH_R12C,&s)&&
       yellow_save_migrate_r12(g,&s)) {
        /* R10-R12 do not serialize historical money or Antidote stock.
         * Do not seal yellow_init()'s starter wallet as restored funds.
         * Keep legacy progress readable, but require explicit offline
         * recovery before spending, consuming stock or saving. */
        g->money_integrity_error=true;
        g->antidote_integrity_error=true;
        return true;
    }
    if(read_saved_file(storage,PRIOR_SAVE_PATH_R12B,&s)&&
       yellow_save_migrate_r12(g,&s)) {
        /* R10-R12 do not serialize historical money or Antidote stock.
         * Do not seal yellow_init()'s starter wallet as restored funds.
         * Keep legacy progress readable, but require explicit offline
         * recovery before spending, consuming stock or saving. */
        g->money_integrity_error=true;
        g->antidote_integrity_error=true;
        return true;
    }
    if(read_saved_file(storage,PRIOR_SAVE_PATH_R12,&s)&&
       yellow_save_migrate_r12(g,&s)) {
        /* R10-R12 do not serialize historical money or Antidote stock.
         * Do not seal yellow_init()'s starter wallet as restored funds.
         * Keep legacy progress readable, but require explicit offline
         * recovery before spending, consuming stock or saving. */
        g->money_integrity_error=true;
        g->antidote_integrity_error=true;
        return true;
    }
    /* Migrate validated older saves, leaving original files unchanged. */
    if(read_saved_file(storage,PRIOR_SAVE_PATH_R11,&s)&&
       yellow_save_migrate_r11(g,&s)) {
        /* R10-R12 do not serialize historical money or Antidote stock.
         * Do not seal yellow_init()'s starter wallet as restored funds.
         * Keep legacy progress readable, but require explicit offline
         * recovery before spending, consuming stock or saving. */
        g->money_integrity_error=true;
        g->antidote_integrity_error=true;
        return true;
    }
    if(read_saved_file(storage,PRIOR_SAVE_PATH_R10,&s)&&
       yellow_save_migrate_r10(g,&s)) {
        /* R10-R12 do not serialize historical money or Antidote stock.
         * Do not seal yellow_init()'s starter wallet as restored funds.
         * Keep legacy progress readable, but require explicit offline
         * recovery before spending, consuming stock or saving. */
        g->money_integrity_error=true;
        g->antidote_integrity_error=true;
        return true;
    }
    return false;
}

int32_t yellow_native_app(void* context) {
    UNUSED(context);
    YellowApp* a=malloc(sizeof(YellowApp));
    if(!a)return -1;
    /* Do not start animation state from uninitialised heap bytes. */
    memset(a,0,sizeof(*a));
    yellow_b2f_set_enabled(true); /* R15 opt-in real gameplay, legacy host tests stay gated */ 
    yellow_init(&a->g);
    a->queue=furi_message_queue_alloc(12,sizeof(InputEvent));
    a->lock=furi_mutex_alloc(FuriMutexTypeNormal);
    a->viewport=view_port_alloc();
    a->gui=furi_record_open(RECORD_GUI);
    Storage* storage=furi_record_open(RECORD_STORAGE);
    (void)load_game(storage,&a->g);
    a->g.native_zubat_extension=true; /* opt-in, volatile, no v13 bytes */
    a->g.native_clefairy_extension=true; /* pinned source slot 9 */
    a->g.native_nurse_choice_extension=true; /* Yes required before heal/anchor */
    a->g.native_mart_intro_extension=true; /* source-backed automatic clerk greeting, no parcel */
    /* Optional source-derived data pack: validate CRC and both supported
       wild maps. If absent/corrupt, preserve the v021 tables and saves. */
    if(yellow_bulk_mount(storage,&a->bulk)) {
        if(!yellow_attach_bulk_wild(&a->g,&a->bulk.wild) ||
           !yellow_attach_bulk_lass(&a->g,&a->bulk.lass)) {
            (void)yellow_attach_bulk_wild(&a->g,NULL);
            (void)yellow_attach_bulk_lass(&a->g,NULL);
            yellow_bulk_unmount(&a->bulk);
        }
    }
    a->source_cached_map=255;
    a->source_world_pending=a->bulk.graphics_ready &&
        (a->g.map==YMapRoute1 || a->g.map==YMapViridianForest);
    view_port_draw_callback_set(a->viewport,draw_cb,a);
    view_port_input_callback_set(a->viewport,input_cb,a);
    gui_add_view_port(a->gui,a->viewport,GuiLayerFullscreen);
    InputEvent event;
    while(a->g.running) {
        /* Keep a 30 Hz animation clock as long as walking or attacking.
           Never wait for the OS's key-repeat timer between tile steps. */
        bool animate=pallet_music_wanted(a)||
                     a->motion_active||a->fx_active||a->direction_held||
                     a->buffered_step||a->scene!=SceneNone||
                     a->source_world_pending;
        FuriStatus status=furi_message_queue_get(
            a->queue,&event,animate?PD_MAIN_TICK_MS:FuriWaitForever);
        furi_mutex_acquire(a->lock,FuriWaitForever);
        uint32_t now=furi_get_tick();
        bool redraw=false;
        bool apply_key=false;
        YellowKey key=YBack;
        if(status==FuriStatusOk) {
            if(event.key==InputKeyUp)key=YUp;
            else if(event.key==InputKeyDown)key=YDown;
            else if(event.key==InputKeyLeft)key=YLeft;
            else if(event.key==InputKeyRight)key=YRight;
            else if(event.key==InputKeyOk)key=YOk;
            else if(event.key==InputKeyBack)key=YBack;
            else {furi_mutex_release(a->lock);continue;}
            if(a->g.oak_parcel_integrity_error &&
               !a->parcel_warning_acknowledged) {
                /* Ignore movement/menu inputs until explicit acknowledgement.
                 * Acknowledge neither clears the corrupt sidecar nor enables
                 * writes. Never apply the same OK/Back to the game state.
                 */
                if(event.type==InputTypeShort && (key==YOk||key==YBack)) {
                    a->parcel_warning_acknowledged=true;
                    a->direction_held=false;
                    a->buffered_step=false;
                    redraw=true;
                }
            } else if(event.type==InputTypeLong && event.key==InputKeyOk) {
                a->audio_debug=!a->audio_debug;
                redraw=true;
            } else if(key<=YRight && a->g.screen==YScreenWorld) {
                if(event.type==InputTypePress) {
                    a->direction_held=true;
                    a->held_direction=key;
                    a->suppress_direction_short=true;
                    a->suppressed_key=key;
                    if(!a->motion_active)apply_key=true;
                } else if(event.type==InputTypeRelease) {
                    if(a->direction_held && a->held_direction==key)
                        a->direction_held=false;
                } else if(event.type==InputTypeShort) {
                    if(a->suppress_direction_short &&
                       a->suppressed_key==key) {
                        a->suppress_direction_short=false;
                    } else if(a->motion_active) {
                        a->buffered_step=true;
                        a->buffered_direction=key;
                    } else apply_key=true;
                } else if(event.type==InputTypeRepeat) {
                    /* Release/short handling is authoritative. If the
                       firmware has no Press callback, Repeat can bootstrap. */
                    if(!a->direction_held) {
                        a->direction_held=true;
                        a->held_direction=key;
                        if(!a->motion_active)apply_key=true;
                    }
                }
            } else if(event.type==InputTypeShort ||
                      event.type==InputTypeRepeat) {
                /* A menu or battle input must not inherit held movement. */
                a->direction_held=false;
                a->buffered_step=false;
                apply_key=true;
            }
        }
        /* Carry overshoot into the next tile. Without this, the 33 ms
           display tick itself introduces a pause at every cell boundary. */
        uint32_t next_step_epoch=now;
        bool chained_step=false;
        /* Complete a logical tile at fixed cadence, not the repeat rate.
           A linear camera/character motion has no velocity-zero boundary. */
        if(a->motion_active && (uint32_t)(now-a->motion_start)>=280) {
            next_step_epoch=a->motion_start+280;
            chained_step=true;
            a->motion_active=false;
            redraw=true;
            if(a->g.screen==YScreenWorld && a->direction_held) {
                key=a->held_direction;apply_key=true;
            } else if(a->g.screen==YScreenWorld && a->buffered_step) {
                key=a->buffered_direction;apply_key=true;
            }
            a->buffered_step=false;
        }
        if(a->scene!=SceneNone)apply_key=false;
        if(apply_key && a->g.screen==YScreenBag) {
            redraw=true;
            if(!a->bag_inside) {
                if(key==YUp && a->bag_category)a->bag_category--;
                else if(key==YDown && a->bag_category<2)a->bag_category++;
                else if(key==YOk) {
                    a->bag_inside=true;
                    a->bag_medicine_selection=0;
                    a->antidote_feedback=0;
                }
                else if(key==YBack) {
                    a->g.screen=YScreenMenu;
                    a->g.selected=1;
                    a->bag_category=0;
                    a->bag_medicine_selection=0;
                    a->antidote_feedback=0;
                }
                apply_key=false;
            } else {
                if(key==YBack) {
                    a->bag_inside=false;
                    apply_key=false;
                } else if(a->bag_category==0 && (key==YUp||key==YDown)) {
                    a->bag_medicine_selection=key==YUp?0u:1u;
                    a->antidote_feedback=0;
                    apply_key=false;
                } else if(key==YOk && a->bag_category==0 &&
                          a->bag_medicine_selection==1) {
                    if(a->g.antidote_integrity_error || a->g.money_integrity_error)
                        a->antidote_feedback=4;
                    else if(!a->g.antidotes)
                        a->antidote_feedback=2;
                    else if(yellow_game_apply_antidote(&a->g,a->g.active_party,
                                                      &a->g.antidotes))
                        a->antidote_feedback=1;
                    else a->antidote_feedback=3;
                    apply_key=false; /* never accidentally use Potion too */
                } else if(key==YOk && a->bag_category==0) {
                    /* Existing core enforces Potion healing and stock rules. */
                    a->antidote_feedback=0;
                    a->g.selected=0;
                } else {
                    apply_key=false;
                }
            }
        }
        if(apply_key && a->pending_attack)apply_key=false;
        if(apply_key && key<=YRight && a->motion_active) {
            /* Direction presses during a step change the next tile only. */
            if(!a->direction_held) {
                a->buffered_step=true;
                a->buffered_direction=key;
            }
            apply_key=false;
        }
        if(apply_key) {
            YellowScreen prior_screen=a->g.screen;
            uint8_t prior_map=a->g.map;
            int16_t x=a->g.x,y=a->g.y;
            int16_t fx=a->g.follower_x,fy=a->g.follower_y;
            bool attack=false;
            uint8_t attack_type=YT_NORMAL;
            if(prior_screen==YScreenFightMenu && key==YOk &&
               a->g.selected<yellow_active_move_count(&a->g) &&
               yellow_party_pp(&a->g,a->g.active_party,a->g.selected)) {
                uint8_t move=yellow_party_move(
                    &a->g,a->g.active_party,a->g.selected);
                const YMoveInfo* info=y_move_info(move);
                if(info){attack=true;attack_type=info->type;}
            }
            bool oak_capture=prior_screen==YScreenOakEncounter && key==YOk;
            bool wild_capture=prior_screen==YScreenFightMenu && key==YOk &&
                a->g.selected==yellow_active_move_count(&a->g) &&
                a->g.foe_kind!=YK_EEVEE && a->g.balls>0 &&
                yellow_party_size(&a->g)<YELLOW_PARTY_MAX;
            if(oak_capture) {
                a->scene=SceneOakCapture;
                a->scene_start=now;
                a->scene_before=a->g;
            } else if(attack) {
                a->fx_type=attack_type;
                a->fx_start=now;
                a->fx_active=true;
                a->pending_attack=true;
            } else {
                if(wild_capture)a->scene_before=a->g;
                yellow_input(&a->g,key);
                if(wild_capture) {
                    a->capture_success=
                        a->g.caught_count>a->scene_before.caught_count;
                    a->scene=SceneWildCapture;
                    a->scene_start=now;
                    a->direction_held=false;
                } else if(a->g.screen==YScreenRivalBattle &&
                     prior_screen!=YScreenRivalBattle &&
                     prior_screen!=YScreenFightMenu) {
                    a->scene=SceneBattleIntro;
                    a->scene_start=now;
                    a->scene_before=a->g;
                    a->direction_held=false;
                }
                bool moved=(prior_screen==YScreenWorld &&
                    a->g.screen==YScreenWorld && prior_map==a->g.map &&
                    abs((int)a->g.x-x)+abs((int)a->g.y-y)==1);
                if(moved) {
                    a->motion_active=true;
                    a->motion_start=chained_step?next_step_epoch:now;
                    a->from_x=x;a->from_y=y;
                    a->from_follow_x=fx;a->from_follow_y=fy;
                    a->step_phase^=1;
                } else if(prior_screen==YScreenWorld) {
                    a->motion_active=false;
                    /* Collision or map change stops autonomous repetition,
                       instead of continuously pounding an unwalkable wall. */
                    a->direction_held=false;
                }
                if(prior_screen!=a->g.screen) {
                    if(a->g.screen==YScreenBag) {
                        a->bag_category=0;a->bag_inside=false;
                        a->bag_medicine_selection=0;a->antidote_feedback=0;
                    }
                    a->motion_active=false;
                    a->direction_held=false;
                    a->buffered_step=false;
                }
                if(a->g.request_save) {
                    a->g.message_saved=save_game(storage,&a->g);
                    a->g.request_save=false;
                }
            }
            redraw=true;
        }
        if(a->fx_active && (uint32_t)(now-a->fx_start)>=440) {
            a->fx_active=false;
                if(a->pending_attack) {
                    a->pending_attack=false;
                    a->scene_before=a->g;
                    yellow_input(&a->g,YOk);
                    a->hp_foe_end=a->g.foe_hp;
                    a->hp_party_end=yellow_party_hp(
                        &a->g,a->g.active_party);
                    a->faint_enemy=a->g.experience>a->scene_before.experience;
                    /* On forced defeat the wild-battle core may immediately
                       heal and teleport. The Faint scene stays visual-only. */
                    a->scene=SceneHpDrain;
                    a->scene_start=now;
                    a->direction_held=false;
                }
            redraw=true;
        }
        if(a->scene!=SceneNone) {
            uint32_t scene_age=now-a->scene_start;
            uint32_t duration=a->scene==SceneBattleIntro?750:
                a->scene==SceneHpDrain?1000:
                a->scene==SceneFaint?600:1850;
            if(scene_age>=duration) {
                YellowScene ended=a->scene;
                a->scene=SceneNone;
                if(ended==SceneOakCapture) {
                    yellow_input(&a->g,YOk);
                } else if(ended==SceneHpDrain &&
                          a->scene_before.foe_hp>0 &&
                          a->hp_foe_end==0 &&
                          a->g.screen==YScreenDialog) {
                    a->scene=SceneFaint;
                    a->scene_start=now;
                }
                redraw=true;
            } else redraw=true;
        }
        if(a->motion_active || a->fx_active)redraw=true;
        if(a->bulk.graphics_ready &&
           (a->g.map==YMapRoute1 || a->g.map==YMapViridianForest) &&
           (a->g.screen==YScreenWorld||a->g.screen==YScreenDialog)) {
            a->source_world_pending=!source_prepare_viewport(a);
            if(a->source_world_pending)redraw=true;
        } else a->source_world_pending=false;
        /* Advance by <= one source note each 33ms visual/audio tick;
           never sleep a note's duration or stall gameplay/input. */
        pallet_audio_tick(a,now);
        bool running=a->g.running;
        furi_mutex_release(a->lock);
        if(running && redraw)view_port_update(a->viewport);
    }
    /* Do not leave a tone running after the game exits. */
    pallet_audio_release(a);
    gui_remove_view_port(a->gui,a->viewport);
    view_port_free(a->viewport);
    (void)yellow_attach_bulk_wild(&a->g,NULL);
    (void)yellow_attach_bulk_lass(&a->g,NULL);
    yellow_bulk_unmount(&a->bulk);
    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_GUI);
    furi_mutex_free(a->lock);
    furi_message_queue_free(a->queue);
    free(a);
    return 0;
}
