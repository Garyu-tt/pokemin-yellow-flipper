from pathlib import Path
import re
root=Path("/home/mirbi/projects/pokemon-yellow-glitchlab")
base=root/"desktop-firmware/applications/services/desktop/views"
p=base/"desktop_view_main.c"
s=p.read_text()
# Header contains the exact catalog plus three original fossil/ghost bitmaps.
src=(root/"source/glitchdex.c").read_text()
entries=re.search(r'static const uint8_t entries\[GLITCHDEX_COUNT\]=\{(.*?)\};',src,re.S).group(1)
pixels=(root/"source/glitchdex_sprites.c").read_text()
decls=pixels.replace('#include "glitchdex_sprites.h"\n','')
decls=re.sub(r'const uint8_t (glitch_\w+)\[200\]',r'static const uint8_t \1[200]',decls)
h='''#ifndef PIKA_GLITCHDEX_H
#define PIKA_GLITCHDEX_H
#include <stdint.h>
#define PIKA_GLITCH_COUNT 105u
static const uint8_t pika_glitch_ids[PIKA_GLITCH_COUNT]={'''+entries+'''};
static inline uint8_t pika_glitch_id(uint8_t index){
    return index<PIKA_GLITCH_COUNT?pika_glitch_ids[index]:0;
}
static inline const char* pika_glitch_name(uint8_t id){
    if(id==0)return "INDEX ZERO";
    if(id==0xB6)return "KABUTOPS FOSSIL";
    if(id==0xB7)return "AERODACTYL";
    if(id==0xB8)return "GHOST";
    for(uint8_t i=1;i<40;i++)if(pika_glitch_ids[i]==id)return "MISSINGNO.";
    return "GLITCH INDEX";
}
'''+decls+'''
#endif
'''
(base/"pika_glitchdex.h").write_text(h)
s=s.replace('#include "pika_desktop_sprites.h"','#include "pika_desktop_sprites.h"\n#include "pika_glitchdex.h"')
s=s.replace('    bool card;\n    bool valid;',"""    bool card;
    bool valid;
    uint8_t gl_mode; /* 0 off / 1 list / 2 detail */
    uint8_t gl_index;
    uint8_t gl_id;
    bool gl_active;""")
i=s.index("static void pika_draw(")
helpers=r'''
/* The invalid Yellow ROM sprite pointers are never executed here. */
static void pika_glitch_sprite(uint8_t id,uint8_t out[200]) {
    const uint8_t* bitmap=NULL;
    if(id==0xB6)bitmap=glitch_kabutops;
    else if(id==0xB7)bitmap=glitch_aerodactyl;
    else if(id==0xB8)bitmap=glitch_ghost;
    if(bitmap) {
        memcpy(out,bitmap,200);
        return;
    }
    /* Bounded deterministic reconstruction, labelled SAFE PREVIEW. */
    uint32_t v=0x9E3779B9u ^ ((uint32_t)id*0x45D9F3Bu);
    for(int i=0;i<200;i++) {
        v^=v<<13;v^=v>>17;v^=v<<5;
        out[i]=((i/5)%7)==0?(uint8_t)v:
               (glitch_kabutops[(i+id)%200] ^ ((uint8_t)v&0x33u));
    }
}
static void pika_glitch_art(Canvas* canvas,uint8_t id,int x,int y) {
    uint8_t pixels[200];
    pika_glitch_sprite(id,pixels);
    canvas_set_color(canvas,ColorBlack);
    canvas_set_bitmap_mode(canvas,false);
    canvas_draw_xbm(canvas,x,y,40,40,pixels);
    canvas_set_bitmap_mode(canvas,true);
}
static void pika_glitch_list(Canvas* canvas,const PikaDesktopModel* m) {
    char txt[32];uint8_t ix=m->gl_index;
    snprintf(txt,sizeof(txt),"GLITCHDEX %u/105",(unsigned)ix+1u);
    canvas_draw_str(canvas,2,10,txt);
    canvas_draw_line(canvas,1,12,127,12);
    uint8_t first=(ix/4u)*4u;
    for(uint8_t row=0;row<4;row++) {
        uint8_t n=first+row;
        if(n>=PIKA_GLITCH_COUNT)break;
        uint8_t id=pika_glitch_id(n);
        snprintf(txt,sizeof(txt),"%c %02X %.16s",n==ix?'>':' ',id,pika_glitch_name(id));
        canvas_draw_str(canvas,2,22+row*10,txt);
    }
    canvas_draw_str(canvas,2,63,"OK VIEW  <> PAGE");
}
static void pika_glitch_detail(Canvas* canvas,const PikaDesktopModel* m) {
    uint8_t id=pika_glitch_id(m->gl_index);
    char txt[32];
    snprintf(txt,sizeof(txt),"GLITCH ID 0x%02X",id);
    canvas_draw_str(canvas,2,10,txt);
    canvas_draw_line(canvas,1,12,127,12);
    canvas_draw_str(canvas,2,24,pika_glitch_name(id));
    canvas_draw_str(canvas,2,36,id>=0xB6&&id<=0xB8?"ORIGINAL ART":"SAFE PREVIEW");
    canvas_draw_str(canvas,2,47,"NO ROM EXEC");
    canvas_draw_str(canvas,2,62,"OK SUMMON");
    pika_glitch_art(canvas,id,87,18);
}
'''
s=s[:i]+helpers+s[i:]
needle='''    canvas_set_font(canvas, FontSecondary);
    if(m->card) {'''
assert needle in s
s=s.replace(needle,'''    canvas_set_font(canvas, FontSecondary);
    if(m->gl_mode==1) {
        pika_glitch_list(canvas,m);
        return;
    }
    if(m->gl_mode==2) {
        pika_glitch_detail(canvas,m);
        return;
    }
    if(m->gl_active&&!m->card) {
        char name[32];
        const char* device=furi_hal_version_get_name_ptr();
        canvas_draw_str(canvas,3,9,device?device:"FLIPPER");
        canvas_draw_line(canvas,2,13,126,13);
        snprintf(name,sizeof(name),"GLITCH %02X",m->gl_id);
        canvas_draw_str(canvas,2,28,name);
        canvas_draw_str(canvas,2,41,"SUMMONED");
        canvas_draw_str(canvas,2,60,"BACK: PIKA");
        pika_glitch_art(canvas,m->gl_id,85,16);
        return;
    }
    if(m->card) {''',1)
needle='''    if(event->type == InputTypeShort && event->key == InputKeyRight) {'''
assert needle in s
s=s.replace(needle,'''    if(event->key == InputKeyOk && event->type == InputTypeLong) {
        model->gl_mode=model->gl_mode?0:1;
        view_commit_model(view->view,true);
        return true;
    }
    if(model->gl_mode) {
        if(event->type==InputTypeShort||event->type==InputTypeRepeat) {
            if(model->gl_mode==1) {
                if(event->key==InputKeyUp&&model->gl_index)model->gl_index--;
                else if(event->key==InputKeyDown&&model->gl_index+1u<PIKA_GLITCH_COUNT)model->gl_index++;
                else if(event->key==InputKeyLeft)model->gl_index=model->gl_index>8?model->gl_index-8:0;
                else if(event->key==InputKeyRight)model->gl_index=model->gl_index+8u<PIKA_GLITCH_COUNT?model->gl_index+8u:PIKA_GLITCH_COUNT-1u;
                else if(event->key==InputKeyOk)model->gl_mode=2;
                else if(event->key==InputKeyBack)model->gl_mode=0;
            } else if(model->gl_mode==2) {
                if(event->key==InputKeyBack)model->gl_mode=1;
                else if(event->key==InputKeyOk) {
                    model->gl_id=pika_glitch_id(model->gl_index);
                    model->gl_active=true;
                    model->gl_mode=0;
                    model->card=false;
                }
            }
            view_commit_model(view->view,true);
        }
        return true;
    }
    if(model->gl_active&&event->type==InputTypeShort&&event->key==InputKeyBack) {
        model->gl_active=false;
        model->card=false;
        view_commit_model(view->view,true);
        return true;
    }
    if(event->type == InputTypeShort && event->key == InputKeyRight) {''',1)
p.write_text(s)
print("PATCHED",p,len(s),"header",len(h))
