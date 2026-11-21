#!/usr/bin/env python3
"""Independent pinned pret/pokeyellow map/block/collision/header verification."""
from pathlib import Path
import os,re,hashlib
src=Path(__file__).resolve().parents[1]
original=Path(os.environ.get("PKYELLOW_SOURCE",src.parent.parent/"upstream/pokeyellow"))
raw=(original/"maps/Route3.blk").read_bytes()
assert len(raw)==35*9
maps=(original/"constants/map_constants.asm").read_text()
assert re.search(r"^\s*map_const ROUTE_3,\s*35,\s*9\b",maps,re.M)
pewter=(original/"data/maps/headers/PewterCity.asm").read_text()
route=(original/"data/maps/headers/Route3.asm").read_text()
assert re.search(r"connection\s+east,\s+Route3,\s+ROUTE_3,\s+4\b",pewter)
assert re.search(r"connection\s+west,\s+PewterCity,\s+PEWTER_CITY,\s+-4\b",route)
s=(src/"yellow_route3_scene.c").read_text()
city=(src/"yellow_pewter_city_scene.c").read_text()
def data(text,name):
 m=re.search(r"const uint8_t "+name+r"\[(\d+)\]=\{(.*?)\};",text,re.S)
 assert m,name
 arr=[int(x,16) for x in re.findall(r"0x[0-9a-f]+",m.group(2))]
 assert len(arr)==int(m.group(1))
 return arr
assert data(s,"ymap_route3")==list(raw)
block=data(city,"ypewter_overworld_blocks")
coll=data(city,"ypewter_overworld_col")
citymap=data(city,"ymap_pewter_city")
citybits=data(city,"ycollision_pewter_city")
def walk(grid,w,x,y):
 bid=grid[(y//2)*w+(x//2)]
 tile=block[bid*16+(y%2*2+1)*4+(x%2*2)]
 return tile<96 and bool(coll[tile])
for y in range(36):
 for x in range(40):
  i=y*40+x
  assert walk(citymap,20,x,y)==bool(citybits[i//8]&(1<<(i%8)))
expected=[0]*((70*18+7)//8)
for y in range(18):
 for x in range(70):
  if walk(raw,35,x,y):
   i=y*70+x
   expected[i//8]|=1<<(i%8)
assert data(s,"ycollision_route3")==expected
corridor=[y for y in range(8,26) if 0<=y-8<18 and
          walk(citymap,20,39,y) and walk(raw,35,0,y-8)]
assert corridor==[16,17,18,19]
s=(src/"yellow_core.c").read_text()
assert "yellow_route3_horizontal_connection" in s
assert "YMapRoute3" in s and "s->map=YMapRoute2" in s
print("PINNED_ROUTE3_315_BLOCKS_FULL_1260_COLLISION_BITS_EAST_WEST_4_ROWS_PASS",
      hashlib.sha256(raw).hexdigest())
