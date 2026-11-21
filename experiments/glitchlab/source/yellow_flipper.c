#include <furi.h>
#include <gui/gui.h>
#include <storage/storage.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "yellow_core.h"
#include "yellow_assets.h"
#include "glitchdex.h"
#include "glitchdex_sprites.h"

#define SAVE_PATH "/ext/apps_data/yellow_glitchlab/save_v13.dat"
#define PRIOR_SAVE_PATH_R12C "/ext/apps_data/yellow_native/save_v12c.dat"
#define PRIOR_SAVE_PATH_R12B "/ext/apps_data/yellow_native/save_v12b.dat"
#define PRIOR_SAVE_PATH_R12 "/ext/apps_data/yellow_native/save_v12.dat"
#define PRIOR_SAVE_PATH_R11 "/ext/apps_data/yellow_native/save_v11.dat"
#define PRIOR_SAVE_PATH_R10 "/ext/apps_data/yellow_native/save_v10.dat"
#define SAVE_DIR "/ext/apps_data/yellow_glitchlab"

typedef struct {
    YellowGame g;
    FuriMessageQueue* queue;
    FuriMutex* lock;
    ViewPort* viewport;
    Gui* gui;
    uint8_t lab_mode; /* 0=game, 1=list, 2=detail */
    uint8_t lab_index; /* 0-104 */
    uint8_t summoned_id;
    bool summoned;
} YellowApp;

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
static void actor(Canvas* c,int x,int y,uint8_t type,int facing) {
    if(x<=-16||x>=128||y<=-16||y>=64)return;
    const uint8_t* sheet=actor_pixels(type);
    // Original 16x96 2-bit sprite is converted to six 16x16 1-bit frames.
    // The first 2 frames face down, next 2 face up, last 2 face sideways.
    int frame=(type==YActorPokedex||type==YActorPokeBall||type==YActorMom||type==YActorNurse||type==YActorChansey)?0:
        (facing==YUp)?2:(facing==YLeft||facing==YRight)?4:0;
    canvas_set_bitmap_mode(c,true);
    canvas_draw_xbm(c,x,y,16,16,sheet+frame*32);
}

/* The original Yellow MissingNo. sprite pointer can be invalid. Never follow it.
 * Three special variants use authentic pixel data; others are safe simulations. */
static void lab_sprite(uint8_t id,uint8_t out[200]) {
    const uint8_t* src=NULL;
    if(id==0xB6)src=glitch_kabutops;
    else if(id==0xB7)src=glitch_aerodactyl;
    else if(id==0xB8)src=glitch_ghost;
    if(src) {
        for(int i=0;i<200;i++)out[i]=src[i];
        return;
    }
    uint32_t rng=0x9E3779B9u^(uint32_t)id*0x45D9F3Bu;
    for(int i=0;i<200;i++) {
        rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;
        uint8_t art=ypokemon_pikachu_front[(i+(unsigned)id*7u)%200u];
        out[i]=((i/5)%7)==0?(uint8_t)rng:(art^((uint8_t)rng&0x33u));
    }
}
static void render_lab_list(Canvas* c,const YellowApp* a) {
    char line[32];uint8_t selected=a->lab_index;
    canvas_set_font(c,FontSecondary);
    snprintf(line,sizeof(line),"GLITCHDEX %u/105",(unsigned)selected+1u);
    canvas_draw_str(c,2,10,line);
    canvas_draw_line(c,1,12,127,12);
    uint8_t start=(uint8_t)((selected/4u)*4u);
    for(uint8_t row=0;row<4;row++) {
        uint8_t index=start+row;
        if(index>=GLITCHDEX_COUNT)break;
        uint8_t id=glitchdex_id(index);
        snprintf(line,sizeof(line),"%c %02X %.16s",index==selected?'>':' ',id,glitchdex_label(id));
        canvas_draw_str(c,2,22+row*10,line);
    }
    canvas_draw_str(c,2,63,"OK VIEW   <> PAGE");
}
static void render_lab_detail(Canvas* c,const YellowApp* a) {
    uint8_t id=glitchdex_id(a->lab_index),img[200];
    char line[30];
    canvas_set_font(c,FontSecondary);
    snprintf(line,sizeof(line),"GLITCH ID 0x%02X",id);
    canvas_draw_str(c,2,10,line);
    canvas_draw_line(c,1,12,127,12);
    canvas_draw_str(c,2,24,glitchdex_label(id));
    canvas_draw_str(c,2,36,(id>=0xB6&&id<=0xB8)?"ORIGINAL ART":"SAFE PREVIEW");
    canvas_draw_str(c,2,47,"NO ROM EXEC");
    canvas_draw_str(c,2,62,"OK SUMMON");
    lab_sprite(id,img);
    canvas_set_bitmap_mode(c,false);
    canvas_draw_xbm(c,87,18,40,40,img);
    canvas_set_bitmap_mode(c,true);
}
static void render_lab_summon(Canvas* c,const YellowApp* a) {
    if(!a->summoned)return;
    const YellowGame* g=&a->g;
    if(g->screen!=YScreenWorld&&g->screen!=YScreenDialog)return;
    const YellowLevelAsset* level=&yellow_levels[g->map];
    int wx=level->w*32,wy=level->h*32;
    int cam_x=clamp(g->x*16-56,0,wx-128);
    int cam_y=clamp(g->y*16-24,0,wy-64);
    int draw_x=(g->x+1)*16-cam_x,draw_y=(g->y-1)*16-cam_y;
    uint8_t full[200],tiny[32]={0};
    lab_sprite(a->summoned_id,full);
    for(int y=0;y<16;y++)for(int x=0;x<16;x++){
        int xx=x*40/16,yy=y*40/16;
        if(full[yy*5+xx/8]&(1u<<(xx%8)))tiny[y*2+x/8]|=1u<<(x%8);
    }
    canvas_set_bitmap_mode(c,false);
    canvas_draw_xbm(c,draw_x,draw_y,16,16,tiny);
    canvas_set_bitmap_mode(c,true);
}
static void render_world(Canvas* c,const YellowGame* g) {
    const YellowLevelAsset* level=&yellow_levels[g->map];
    const int world_w=level->w*32,world_h=level->h*32;
    const int cam_x=clamp(g->x*16-56,0,world_w-128);
    const int cam_y=clamp(g->y*16-24,0,world_h-64);
    const int first_col=cam_x/8,first_row=cam_y/8;
    canvas_set_color(c,ColorBlack);
    // World is displayed at original pixel resolution: no shrink/stretch.
    canvas_set_bitmap_mode(c,false);
    for(int row=first_row;row<=first_row+8&&row<world_h/8;row++) {
        for(int col=first_col;col<=first_col+16&&col<world_w/8;col++) {
            int x=col*8-cam_x,y=row*8-cam_y;
            uint8_t tile=yellow_tile_id(g->map,col,row);
            if(tile<level->tiles_count) {
                canvas_draw_xbm(c,x,y,8,8,level->tiles+tile*8);
            }
        }
    }
    for(uint8_t i=0;i<yellow_npc_count;i++) {
        const YellowNpc* n=&yellow_npcs[i];
        if(yellow_npc_visible(g,n))
            actor(c,n->x*16-cam_x,n->y*16-cam_y,n->type,YDown);
    }
    if(g->story==YStoryPikachuFollows && g->has_pikachu &&
       g->follower_valid && g->follower_map==g->map) {
        actor(c,g->follower_x*16-cam_x,g->follower_y*16-cam_y,YActorPikachu,g->facing);
    }
    actor(c,g->x*16-cam_x,g->y*16-cam_y,YActorRed,g->facing);
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
static void render_menu(Canvas* c,const YellowGame* g) {
    static const char* const items[]={"PARTY","BAG","SAVE","EXIT"};
    canvas_set_font(c,FontSecondary);
    canvas_draw_str(c,2,10,"POKEMON YELLOW [PORT]");
    for(int i=0;i<4;i++){
        int y=23+i*12;
        if(g->selected==i)canvas_draw_str(c,6,y,">");
        canvas_draw_str(c,19,y,items[i]);
    }
    if(g->request_save)canvas_draw_str(c,78,61,"SAVING");
    else if(g->message_saved)canvas_draw_str(c,84,61,"SAVED");
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
    } else {
        canvas_draw_str(c,3,11,"BAG");
        canvas_draw_frame(c,1,15,126,39);
        snprintf(line,sizeof(line),"POKE BALL x%u",(unsigned)g->balls);
        canvas_draw_str(c,6,28,line);
        canvas_draw_str(c,6,41,"USE IN WILD BATTLE");
        canvas_draw_str(c,3,62,"BACK: RETURN");
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
static void render_battle(Canvas* c,const YellowGame* g) {
    char line[26];
    canvas_set_font(c,FontSecondary);
    const YSpeciesInfo* foe=y_species_info(g->foe_kind);
    snprintf(line,sizeof(line),"%s Lv%u",foe?foe->name:"???",(unsigned)g->foe_level);
    canvas_draw_str(c,1,9,line);
    canvas_draw_frame(c,1,12,68,5);
    if(g->foe_hp&&g->foe_max_hp)
        canvas_draw_box(c,3,14,g->foe_hp*64/g->foe_max_hp,1);
    canvas_set_bitmap_mode(c,false);
    const uint8_t* front=g->foe_kind==YK_PIDGEY?ypokemon_pidgey_front:
                         g->foe_kind==YK_RATTATA?ypokemon_rattata_front:ypokemon_eevee_front;
    canvas_draw_xbm(c,86,0,40,40,front);
    const uint8_t* back=g->active_party==0?ypokemon_pikachu_back:
        yellow_party_species(g,g->active_party)==YK_PIDGEY?
            ypokemon_pidgey_back:ypokemon_rattata_back;
    canvas_draw_xbm(c,4,32,32,32,back);
    canvas_set_bitmap_mode(c,true);
    const uint8_t active=g->active_party;
    const YSpeciesInfo* mon=y_species_info(yellow_party_species(g,active));
    snprintf(line,sizeof(line),"%s L%u",mon?mon->name:"???",
             (unsigned)yellow_party_level(g,active));
    canvas_draw_str(c,49,45,line);
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
static void draw_cb(Canvas* c,void* context) {
    YellowApp* a=context;
    if(furi_mutex_acquire(a->lock,FuriWaitForever)!=FuriStatusOk)return;
    canvas_clear(c);
    canvas_set_color(c,ColorBlack);
    if(a->lab_mode==1)render_lab_list(c,a);
    else if(a->lab_mode==2)render_lab_detail(c,a);
    else switch(a->g.screen){
    case YScreenWorld:render_world(c,&a->g);break;
    case YScreenDialog:render_world(c,&a->g);render_dialog(c,&a->g);break;
    case YScreenMenu:render_menu(c,&a->g);break;
    case YScreenParty:case YScreenBag:render_info(c,&a->g);break;
    case YScreenOakEncounter:render_oak_capture(c);break;
    case YScreenRivalBattle:render_battle(c,&a->g);break;
    case YScreenFightMenu:render_fight_menu(c,&a->g);break;
    case YScreenMoveForget:render_move_forget(c,&a->g);break;
    }
    if(!a->lab_mode)render_lab_summon(c,a);
    furi_mutex_release(a->lock);
}
static void input_cb(InputEvent* event,void* context) {
    YellowApp* a=context;
    if(event->type==InputTypeShort||event->type==InputTypeRepeat||
       (event->type==InputTypeLong&&event->key==InputKeyOk))
        (void)furi_message_queue_put(a->queue,event,0);
}
static bool save_game(Storage* storage,YellowGame* g) {
    YellowSave s;
    yellow_save_create(g,&s);
    storage_common_mkdir(storage,SAVE_DIR);
    File* f=storage_file_alloc(storage);
    bool ok=storage_file_open(f,SAVE_PATH,FSAM_WRITE,FSOM_CREATE_ALWAYS);
    if(ok)ok=storage_file_write(f,&s,sizeof(s))==sizeof(s);
    if(storage_file_is_open(f))storage_file_close(f);
    storage_file_free(f);
    return ok;
}
static bool read_saved_file(Storage* storage,const char* path,YellowSave* out) {
    File* f=storage_file_alloc(storage);
    bool ok=storage_file_open(f,path,FSAM_READ,FSOM_OPEN_EXISTING);
    if(ok)ok=storage_file_read(f,out,sizeof(*out))==sizeof(*out);
    if(storage_file_is_open(f))storage_file_close(f);
    storage_file_free(f);
    return ok;
}
static bool load_game(Storage* storage,YellowGame* g) {
    YellowSave s;
    return read_saved_file(storage,SAVE_PATH,&s)&&yellow_save_load(g,&s);
}

int32_t yellow_glitchlab_app(void* context) {
    UNUSED(context);
    YellowApp* a=malloc(sizeof(YellowApp));
    if(!a)return -1;
    yellow_init(&a->g);
    a->lab_mode=0;a->lab_index=0;a->summoned=false;a->summoned_id=0;
    a->queue=furi_message_queue_alloc(12,sizeof(InputEvent));
    a->lock=furi_mutex_alloc(FuriMutexTypeNormal);
    a->viewport=view_port_alloc();
    a->gui=furi_record_open(RECORD_GUI);
    Storage* storage=furi_record_open(RECORD_STORAGE);
    (void)load_game(storage,&a->g);
    view_port_draw_callback_set(a->viewport,draw_cb,a);
    view_port_input_callback_set(a->viewport,input_cb,a);
    gui_add_view_port(a->gui,a->viewport,GuiLayerFullscreen);
    InputEvent event;
    while(a->g.running){
        if(furi_message_queue_get(a->queue,&event,FuriWaitForever)==FuriStatusOk){
            if(event.type==InputTypeLong&&event.key==InputKeyOk) {
                furi_mutex_acquire(a->lock,FuriWaitForever);
                if(!a->lab_mode && (a->g.screen==YScreenWorld||a->g.screen==YScreenMenu))
                    a->lab_mode=1;
                else if(a->lab_mode)a->lab_mode=0;
                furi_mutex_release(a->lock);
                view_port_update(a->viewport);
                continue;
            }
            YellowKey key;
            if(event.key==InputKeyUp)key=YUp;
            else if(event.key==InputKeyDown)key=YDown;
            else if(event.key==InputKeyLeft)key=YLeft;
            else if(event.key==InputKeyRight)key=YRight;
            else if(event.key==InputKeyOk)key=YOk;
            else if(event.key==InputKeyBack)key=YBack;
            else continue;
            furi_mutex_acquire(a->lock,FuriWaitForever);
            if(a->lab_mode==1) {
                if(key==YUp&&a->lab_index>0)a->lab_index--;
                else if(key==YDown&&a->lab_index+1u<GLITCHDEX_COUNT)a->lab_index++;
                else if(key==YLeft)a->lab_index=a->lab_index>8?a->lab_index-8:0;
                else if(key==YRight)a->lab_index+8u<GLITCHDEX_COUNT?(a->lab_index+=8u):(a->lab_index=GLITCHDEX_COUNT-1u);
                else if(key==YOk)a->lab_mode=2;
                else if(key==YBack)a->lab_mode=0;
            } else if(a->lab_mode==2) {
                if(key==YBack)a->lab_mode=1;
                else if(key==YOk) {
                    a->summoned=true;
                    a->summoned_id=glitchdex_id(a->lab_index);
                    a->lab_mode=0;
                    a->g.screen=YScreenWorld;
                }
            } else yellow_input(&a->g,key);
            if(a->g.request_save){
                a->g.message_saved=save_game(storage,&a->g);
                a->g.request_save=false;
            }
            bool running=a->g.running;
            furi_mutex_release(a->lock);
            if(running)view_port_update(a->viewport);
        }
    }
    gui_remove_view_port(a->gui,a->viewport);
    view_port_free(a->viewport);
    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_GUI);
    furi_mutex_free(a->lock);
    furi_message_queue_free(a->queue);
    free(a);
    return 0;
}
