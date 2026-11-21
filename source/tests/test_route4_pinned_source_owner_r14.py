#!/usr/bin/env python3
"""Independent whole Route4 source block, collision and reciprocal-header audit."""
from pathlib import Path
import re,os,hashlib
p=Path(__file__).resolve().parents[1]
u=Path(os.environ.get("PKYELLOW_SOURCE",p.parent.parent/"upstream/pokeyellow"))
raw=(u/"maps/Route4.blk").read_bytes()
original3=(u/"maps/Route3.blk").read_bytes()
assert len(raw)==405 and len(original3)==315
constants=(u/"constants/map_constants.asm").read_text()
assert re.search(r"^\s*map_const\s+ROUTE_4,\s*45,\s*9\b",constants,re.M)
r3=(u/"data/maps/headers/Route3.asm").read_text()
r4=(u/"data/maps/headers/Route4.asm").read_text()
assert re.search(r"connection\s+north,\s*Route4,\s*ROUTE_4,\s*25\b",r3)
assert re.search(r"connection\s+south,\s*Route3,\s*ROUTE_3,\s*-25\b",r4)
scene=(p/"yellow_route4_scene.c").read_text()
over=(p/"yellow_pewter_city_scene.c").read_text()
def values(text,name):
 m=re.search(r"const uint8_t "+name+r"\[(\d+)\]=\{(.*?)\};",text,re.S)
 assert m,name
 result=[int(x,16) for x in re.findall(r"0x[0-9a-f]+",m.group(2))]
 assert len(result)==int(m.group(1))
 return result
assert values(scene,"ymap_route4")==list(raw)
blocks=values(over,"ypewter_overworld_blocks")
col=values(over,"ypewter_overworld_col")
def walking(data,width,x,y):
 bid=data[(y//2)*width+x//2]
 tile=blocks[bid*16+(y%2*2+1)*4+x%2*2]
 return tile<len(col) and bool(col[tile])
bitmap=[0]*203
for y in range(18):
 for x in range(90):
  if walking(raw,45,x,y):
   bit=y*90+x
   bitmap[bit//8]|=1<<(bit%8)
assert bitmap==values(scene,"ycollision_route4")
links=[(x,x-50) for x in range(70) if 0<=x-50<90 and
       walking(original3,35,x,0) and walking(raw,45,x-50,17)]
assert links==[(57,7),(58,8),(59,9),(60,10),(61,11),(62,12),(63,13)]
core=(p/"yellow_core.c").read_text()
assert "yellow_route4_vertical_connection" in core
assert "yellow_route4_passable" in core
assert "YMapRoute4" in core
assert "s->map=YMapRoute2" in core
assert "YMapMtMoon" not in core
print("ORIGINAL_ROUTE4_405_BLOCKS_ALL_1620_COLLISIONS_7_NORTH_SOUTH_LINKS_PINNED_PASS",
      hashlib.sha256(raw).hexdigest())
