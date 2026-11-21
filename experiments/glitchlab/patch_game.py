from pathlib import Path
p=Path('/home/mirbi/projects/pokemon-yellow-glitchlab/source/yellow_flipper.c')
s=p.read_text()
s=s.replace('#include "yellow_assets.h"','#include "yellow_assets.h"\n#include "glitchdex.h"\n#include "glitchdex_sprites.h"')
s=s.replace('#define SAVE_PATH "/ext/apps_data/yellow_native/save_v13.dat"','#define SAVE_PATH "/ext/apps_data/yellow_glitchlab/save_v13.dat"')
s=s.replace('#define SAVE_DIR "/ext/apps_data/yellow_native"','#define SAVE_DIR "/ext/apps_data/yellow_glitchlab"')
s=s.replace('    Gui* gui;\n} YellowApp;',"""    Gui* gui;
    uint8_t lab_mode; /* 0=game, 1=list, 2=detail */
    uint8_t lab_index; /* 0-104 */
    uint8_t summoned_id;
    bool summoned;
} YellowApp;""")
i=s.index('static void render_world(')
helpers=r'''
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
'''
s=s[:i]+helpers+s[i:]
assert '    switch(a->g.screen){' in s
s=s.replace('    switch(a->g.screen){','    if(a->lab_mode==1)render_lab_list(c,a);\n    else if(a->lab_mode==2)render_lab_detail(c,a);\n    else switch(a->g.screen){',1)
needle='    furi_mutex_release(a->lock);\n}\nstatic void input_cb'
assert needle in s
s=s.replace(needle,'    if(!a->lab_mode)render_lab_summon(c,a);\n    furi_mutex_release(a->lock);\n}\nstatic void input_cb',1)
s=s.replace('    if(event->type==InputTypeShort||event->type==InputTypeRepeat)','    if(event->type==InputTypeShort||event->type==InputTypeRepeat||\n       (event->type==InputTypeLong&&event->key==InputKeyOk))')
first=s.index('static bool load_game(')
last=s.index('\nint32_t yellow_native_app(',first)
s=s[:first]+'''static bool load_game(Storage* storage,YellowGame* g) {
    YellowSave s;
    return read_saved_file(storage,SAVE_PATH,&s)&&yellow_save_load(g,&s);
}
'''+s[last:]
s=s.replace('int32_t yellow_native_app(void* context)','int32_t yellow_glitchlab_app(void* context)')
s=s.replace('    yellow_init(&a->g);','    yellow_init(&a->g);\n    a->lab_mode=0;a->lab_index=0;a->summoned=false;a->summoned_id=0;',1)
needle='''            YellowKey key;
            if(event.key==InputKeyUp)'''
assert needle in s
s=s.replace(needle,'''            if(event.type==InputTypeLong&&event.key==InputKeyOk) {
                furi_mutex_acquire(a->lock,FuriWaitForever);
                if(!a->lab_mode && (a->g.screen==YScreenWorld||a->g.screen==YScreenMenu))
                    a->lab_mode=1;
                else if(a->lab_mode)a->lab_mode=0;
                furi_mutex_release(a->lock);
                view_port_update(a->viewport);
                continue;
            }
            YellowKey key;
            if(event.key==InputKeyUp)''',1)
needle='''            yellow_input(&a->g,key);
            if(a->g.request_save){'''
assert needle in s
s=s.replace(needle,'''            if(a->lab_mode==1) {
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
            if(a->g.request_save){''',1)
p.write_text(s)
print('patched',p,'bytes',p.stat().st_size)
