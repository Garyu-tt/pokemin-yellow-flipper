#!/usr/bin/env python3
"""Independent pinned Mt Moon geometry, Cavern collisions, lossy mono projection."""
from pathlib import Path
import os,re,struct,zlib,hashlib
p=Path(__file__).resolve().parents[1]
u=Path(os.environ.get("PKYELLOW_SOURCE",p.parent.parent/"upstream/pokeyellow"))
scene=(p/"yellow_mtmoon1f_scene.c").read_text()
def data(name):
 m=re.search(r"const uint8_t "+name+r"\[(\d+)\]=\{(.*?)\};",scene,re.S)
 assert m,name
 a=[int(x,16) for x in re.findall(r"0x[0-9a-f]+",m.group(2))]
 assert len(a)==int(m.group(1))
 return a
world=(u/"maps/MtMoon1F.blk").read_bytes()
blocks=(u/"gfx/blocksets/cavern.bst").read_bytes()
assert len(world)==360 and len(blocks)==2048
assert data("ymap_mtmoon1f")==list(world)
assert data("yblocks_cavern")==list(blocks)
source=(u/"data/tilesets/collision_tile_ids.asm").read_text()
cavern=re.search(r"Cavern_Coll::\s*\n\s*coll_tiles\s+([^\n]+)",source)
assert cavern
allowed={int(x,16) for x in re.findall(r"\$([0-9a-fA-F]+)",cavern.group(1))}
assert allowed=={0x05,0x15,0x18,0x1a,0x20,0x21,0x22,0x2a,0x2d,0x30}
assert data("ycollision_cavern")==[int(i in allowed) for i in range(80)]
collision=[0]*180
for y in range(36):
 for x in range(40):
  bid=world[(y//2)*20+x//2]
  tile=blocks[bid*16+(y%2*2+1)*4+(x%2)*2]
  if tile in allowed:
   i=y*40+x;collision[i//8]|=1<<(i%8)
assert collision==data("ycollision_mtmoon1f")
# Decode original 2-bit grayscale PNG, reconstruct scanline filters and apply
# explicit lossy <=1 to 1-bit projection, NOT claiming original color parity.
png=(u/"gfx/tilesets/cavern.png").read_bytes()
assert png[:8]==b"\x89PNG\r\n\x1a\n"
pos=8;compressed=b"";width=height=0
while pos<len(png):
 count=struct.unpack_from(">I",png,pos)[0];pos+=4
 kind=png[pos:pos+4];pos+=4
 chunk=png[pos:pos+count];pos+=count+4
 if kind==b'IHDR':
  width,height,depth,color,comp,flt,interlace=struct.unpack(">IIBBBBB",chunk)
  assert (width,height,depth,color,comp,flt,interlace)==(128,40,2,0,0,0,0)
 elif kind==b'IDAT':compressed+=chunk
 elif kind==b'IEND':break
scan=zlib.decompress(compressed)
stride=width//4
assert len(scan)==height*(stride+1)
rows=[];before=[0]*stride
for y in range(height):
 off=y*(stride+1);flt=scan[off];line=list(scan[off+1:off+1+stride])
 assert 0<=flt<=4
 for x in range(stride):
  left=line[x-1] if x else 0
  upper=before[x]
  up_left=before[x-1] if x else 0
  if flt==1:line[x]=(line[x]+left)&255
  elif flt==2:line[x]=(line[x]+upper)&255
  elif flt==3:line[x]=(line[x]+((left+upper)//2))&255
  elif flt==4:
   predictor=left+upper-up_left
   dl,du,dul=(abs(predictor-v) for v in (left,upper,up_left))
   best=left if dl<=du and dl<=dul else upper if du<=dul else up_left
   line[x]=(line[x]+best)&255
 rows.append([(line[x//4]>>(6-2*(x%4)))&3 for x in range(width)])
 before=line
reduced=[]
for by in range(height//8):
 for bx in range(width//8):
  for y in range(8):
   byte=0
   for x in range(8):
    if rows[by*8+y][bx*8+x]<=1:byte|=1<<(7-x)
   reduced.append(byte)
assert len(reduced)==640
assert data("ytiles_cavern")==reduced
objects4=(u/"data/maps/objects/Route4.asm").read_text()
objects1f=(u/"data/maps/objects/MtMoon1F.asm").read_text()
assert re.search(r"warp_event\s+18,\s*5,\s*MT_MOON_1F,\s*1",objects4)
assert re.search(r"warp_event\s+14,\s*35,\s*LAST_MAP,\s*2",objects1f)
assert re.search(r"warp_event\s+15,\s*35,\s*LAST_MAP,\s*2",objects1f)
runtime=(p/"yellow_mtmoon1f_runtime.c").read_text()
assert "map==YMapRoute4 && x==18 && y==5" in runtime
assert "map==YMapMtMoon1F && (x==14||x==15) && y==35" in runtime
print("PINNED_MTMOON1F_360_BLOCKS_2048_CAVERN_BLOCKS_640_MONO_BYTES_1440_COLLISION_BITS_WARP_PASS",
      hashlib.sha256(world).hexdigest())
