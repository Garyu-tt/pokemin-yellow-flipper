from PIL import Image
from pathlib import Path
import hashlib, struct, zlib, json
import os
ROOT=Path(os.environ.get('PKYELLOW_SOURCE', str(Path(__file__).resolve().parents[2]/'upstream/pokeyellow')))
DST=Path(__file__).resolve().parents[1]
SPECS=[('PalletTown','overworld',10,9),('RedsHouse1F','reds_house',4,4),('BluesHouse','house',4,4),('OaksLab','gym',5,6),('Route1','overworld',10,18),('ViridianCity','overworld',20,18),('ViridianPokecenter','pokecenter',7,4),('Route2','overworld',10,36),('ViridianForest','forest',17,24),('ViridianForestSouthGate','gate',5,4),('ViridianForestNorthGate','gate',5,4)]
SPRITES=['red','oak','little_girl','fisher','mom','girl','blue','pikachu','daisy','pokedex','poke_ball','scientist','youngster','gambler','nurse','gentleman','cooltrainer_m','link_receptionist','chansey']
BAYER=((0,8,2,10),(12,4,14,6),(3,11,1,9),(15,7,13,5))
def black(px,x,y):
    # Preserve outlines, lightly dither original 4-level Game Boy pixels
    if px<44:return True
    if px<120:return BAYER[y&3][x&3]<11
    if px<200:return BAYER[y&3][x&3]<5
    return False
def hex_c_array(name,buf):
    out=['static const uint8_t %s[%d] = {'%(name,len(buf))]
    for i in range(0,len(buf),24):
        out.append(' '+','.join('0x%02x'%v for v in buf[i:i+24])+',')
    out.append('};')
    return out
def tiles(name):
    im=Image.open(ROOT/'gfx/tilesets'/('%s.png'%name)).convert('L')
    assert im.width==128 and im.height%8==0, (name,im.size)
    parts=[]
    for idx in range((im.width//8)*(im.height//8)):
        tx=(idx%16)*8;ty=(idx//16)*8
        for y in range(8):
            bits=0
            for x in range(8):
                if black(im.getpixel((tx+x,ty+y)),tx+x,ty+y):bits |= 1<<x
            parts.append(bits)
    return parts
def sprite(name):
    im=Image.open(ROOT/'gfx/sprites'/('%s.png'%name)).convert('L')
    assert im.width==16 and im.height in (16,48,64,96), (name,im.size)
    out=[]
    for f in range(6):
        frame_index=min(f,im.height//16-1)
        for y in range(16):
            for bx in range(2):
                bits=0
                for x in range(8):
                    gx=bx*8+x
                    if black(im.getpixel((gx,frame_index*16+y)),gx,y): bits|=1<<x
                out.append(bits)
    return out
lines=['/* Generated directly from pret/pokeyellow binary blocksets and actual PNG resources. */',
'#pragma once','#include <stdint.h>',
'typedef struct { uint8_t w,h; uint16_t blocks_count,tiles_count; const uint8_t* map; const uint8_t* blocks; const uint8_t* tiles; const uint8_t* collision; } YellowLevelAsset;']
assets=[]
for i,(mapname,setname,w,h) in enumerate(SPECS):
    tile=tiles(setname); blocks=(ROOT/'gfx/blocksets'/('%s.bst'%setname)).read_bytes()
    blockmap=(ROOT/'maps'/('%s.blk'%mapname)).read_bytes()
    assert len(blockmap)==w*h,(mapname,len(blockmap))
    assert len(blocks)%16==0 and max(blockmap)<len(blocks)//16,(mapname, max(blockmap))
    assert max(blocks[b*16+k] for b in blockmap for k in range(16))<len(tile)//8,(setname, max(blockmap),len(tile)//8)
    lines+=hex_c_array('ytile_%d'%i,tile)
    lines+=hex_c_array('yblock_%d'%i,blocks)
    lines+=hex_c_array('ymap_%d'%i,blockmap)
    print(mapname,w,h,'tile_count',len(tile)//8,'block_count',len(blocks)//16,'map_hash',hashlib.sha256(blockmap).hexdigest()[:16])
    assets.append((w,h,len(blocks)//16,len(tile)//8,i))
for label, png in [('pikachu_front','front/pikachu.png'),('eevee_front','front/eevee.png'),('pikachu_back','back/pikachub.png'),('pidgey_back','back/pidgeyb.png'),('rattata_back','back/rattatab.png'),('pidgey_front','front/pidgey.png'),('rattata_front','front/rattata.png')]:
    image=Image.open(ROOT/'gfx/pokemon'/png).convert('L')
    w,h=image.size
    assert (w,h) in ((40,40),(32,32))
    pixels=[]
    for y in range(h):
        for byte in range((w+7)//8):
            bits=0
            for k in range(8):
                x=byte*8+k
                if x<w and black(image.getpixel((x,y)),x,y):bits|=1<<k
            pixels.append(bits)
    lines+=hex_c_array('ypokemon_'+label,pixels)
    print('pokemon',label,w,h,len(pixels))
for name in SPRITES:
    try: s=sprite(name)
    except Exception as exc: print('SKIP',name,exc);continue
    lines+=hex_c_array('ysprite_'+name,s)
    print('sprite',name,len(s))
# Collision IDs: literal sets copied from data/tilesets/collision_tile_ids.asm
collisions=[
[0x00,0x10,0x1b,0x20,0x21,0x23,0x2c,0x2d,0x2e,0x30,0x31,0x33,0x39,0x3c,0x3e,0x52,0x54,0x58,0x5b],
[0x01,0x02,0x03,0x11,0x12,0x13,0x14,0x1c,0x1a],
[0x01,0x12,0x14,0x28,0x32,0x37,0x44,0x54,0x5c],
[0x3a,0x03,0x11,0x16,0x19,0x2b,0x3c,0x3d,0x3f,0x4a,0x4c,0x4d]]
collisions += [collisions[0], collisions[0], [0x11,0x1a,0x1c,0x3c,0x5e]]
# Pinned Gen1 Forest_Coll / Gate_Coll, source data/tilesets/collision_tile_ids.asm.
collisions += [collisions[0],
 [0x1e,0x20,0x2e,0x30,0x34,0x37,0x39,0x3a,0x40,0x51,0x52,0x5a,0x5c,0x5e,0x5f],
 [0x01,0x12,0x14,0x1a,0x1c,0x37,0x38,0x3b,0x3c,0x5e],
 [0x01,0x12,0x14,0x1a,0x1c,0x37,0x38,0x3b,0x3c,0x5e]]  # Route 1 and Viridian City use OVERWORLD
for i,vals in enumerate(collisions):
    flags=bytearray(96)
    for n in vals: flags[n]=1
    lines+=hex_c_array('ycoll_%d'%i,flags)
lines+=['static const YellowLevelAsset yellow_levels[] = {']
for w,h,nb,nt,i in assets:
    lines.append(' {%d,%d,%d,%d,ymap_%d,yblock_%d,ytile_%d,ycoll_%d},'%(w,h,nb,nt,i,i,i,i))
lines+=['};','#define YELLOW_LEVELS_COUNT %d'%len(assets)]
# Emit one asset translation unit instead of separate copies in each C module.
import re
complete='\n'.join(lines)+'\n'
prefix='#include "yellow_assets.h"\n'
header_struct='typedef struct { uint8_t w,h; uint16_t blocks_count,tiles_count; const uint8_t* map; const uint8_t* blocks; const uint8_t* tiles; const uint8_t* collision; } YellowLevelAsset;'
body=complete.replace('/* Generated directly from pret/pokeyellow binary blocksets and actual PNG resources. */\n#pragma once\n#include <stdint.h>\n'+header_struct+'\n',prefix)
body=body.replace('static const YellowLevelAsset yellow_levels[]','const YellowLevelAsset yellow_levels[]')
body=re.sub(r'static const uint8_t (ysprite_|ypokemon_)',r'const uint8_t \1',body)
public=re.findall(r'const uint8_t ((?:ysprite_|ypokemon_)[A-Za-z0-9_]+)\[(\d+)\]',body)
header=['#pragma once','#include <stdint.h>',header_struct,'#define YELLOW_LEVELS_COUNT %d'%len(assets),'extern const YellowLevelAsset yellow_levels[YELLOW_LEVELS_COUNT];']
for name,length in public:header.append('extern const uint8_t %s[%s];'%(name,length))
(DST/'yellow_assets.h').write_text('\n'.join(header)+'\n')
(DST/'yellow_assets.c').write_text(body)

# Map visual with original colored shades for QA; 320x288.
w,h=10,9
maptiles=list((ROOT/'maps/PalletTown.blk').read_bytes()); blocks=list((ROOT/'gfx/blocksets/overworld.bst').read_bytes())
im=Image.open(ROOT/'gfx/tilesets/overworld.png').convert('L')
output=Image.new('L',(w*32,h*32),255)
for by in range(h):
 for bx in range(w):
    bid=maptiles[by*w+bx]
    for j in range(4):
     for k in range(4):
      idx=blocks[bid*16+j*4+k]
      sx=(idx%16)*8; sy=(idx//16)*8
      output.paste(im.crop((sx,sy,sx+8,sy+8)),(bx*32+k*8,by*32+j*8))
output.save(DST/'pallet_original_preview.png')
p=Path(DST/'source_manifest.json')
p.write_text(json.dumps({'upstream':'https://github.com/pret/pokeyellow','git_commit':__import__('subprocess').check_output(['git','-C',str(ROOT),'rev-parse','HEAD'],text=True).strip(), 'maps':[x[0] for x in SPECS],'sprites':SPRITES},indent=2))
print('HEADER_SIZE', (DST/'yellow_assets.h').stat().st_size)
