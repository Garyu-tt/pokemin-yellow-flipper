from pathlib import Path
import struct,zlib
b=Path("/home/mirbi/projects/pokemon-yellow-glitchlab/source")
up=Path("/home/mirbi/projects/pokemon-yellow-native/upstream/pokeyellow")
names={"kabutops":"gfx/pokemon/front/fossilkabutops.png","aerodactyl":"gfx/pokemon/front/fossilaerodactyl.png","ghost":"gfx/battle/ghost.png"}
def parse(path):
    dat=path.read_bytes();assert dat[:8]==b'\x89PNG\r\n\x1a\n'
    off=8;compressed=b'';info=None
    while off<len(dat):
        size=int.from_bytes(dat[off:off+4],'big'); typ=dat[off+4:off+8]; chunk=dat[off+8:off+8+size];off+=12+size
        if typ==b'IHDR': info=struct.unpack('>IIBBBBB',chunk)
        if typ==b'IDAT':compressed+=chunk
        if typ==b'IEND':break
    w,h,depth,color,_,_,inter=info
    assert color==0 and depth in (1,2,4,8) and inter==0,(path,info)
    rb=(w*depth+7)//8; raw=zlib.decompress(compressed)
    pos=0;previous=bytearray(rb);out=[]
    for y in range(h):
        fl=raw[pos];pos+=1;row=bytearray(raw[pos:pos+rb]);pos+=rb
        for i in range(rb):
            a=row[i-1] if i else 0;u=previous[i];q=previous[i-1] if i else 0
            if fl==1:row[i]=(row[i]+a)&255
            elif fl==2:row[i]=(row[i]+u)&255
            elif fl==3:row[i]=(row[i]+((a+u)//2))&255
            elif fl==4:
                p=a+u-q;pa=abs(p-a);pb=abs(p-u);pc=abs(p-q)
                row[i]=(row[i]+(a if pa<=pb and pa<=pc else u if pb<=pc else q))&255
            elif fl!=0:raise ValueError((path,fl))
        previous=row
        out.append([(row[(x*depth)//8]>>(8-depth-(x*depth)%8))&((1<<depth)-1) for x in range(w)])
    return out,w,h,depth
defs=""
for name,filename in names.items():
    rows,w,h,depth=parse(up/filename)
    buf=[0]*200
    for y in range(40):
        for x in range(40):
            xx=min(w-1,x*w//40); yy=min(h-1,y*h//40)
            if rows[yy][xx] < (1<<depth)//2:
                buf[y*5+x//8]|=1<<(x%8)
    defs+=f"const uint8_t glitch_{name}[200]={{"+','.join(map(str,buf))+"};\n"
    print(name,w,h,depth,"black",sum(v.bit_count() for v in buf))
(b/'glitchdex_sprites.h').write_text('#ifndef GLITCHDEX_SPRITES_H\n#define GLITCHDEX_SPRITES_H\n#include <stdint.h>\nextern const uint8_t glitch_kabutops[200];\nextern const uint8_t glitch_aerodactyl[200];\nextern const uint8_t glitch_ghost[200];\n#endif\n')
(b/'glitchdex_sprites.c').write_text('#include "glitchdex_sprites.h"\n'+defs)
