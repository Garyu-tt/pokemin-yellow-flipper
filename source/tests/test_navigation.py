from pathlib import Path
import os
from collections import deque
ROOT=Path(os.environ.get('PKYELLOW_SOURCE', '/tmp/pokeyellow'))
spec=[('PalletTown','overworld',10,9),('RedsHouse1F','reds_house',4,4),('BluesHouse','house',4,4),('OaksLab','gym',5,6)]
coll=[
{0x00,0x10,0x1b,0x20,0x21,0x23,0x2c,0x2d,0x2e,0x30,0x31,0x33,0x39,0x3c,0x3e,0x52,0x54,0x58,0x5b},
{0x01,0x02,0x03,0x11,0x12,0x13,0x14,0x1c,0x1a},
{0x01,0x12,0x14,0x28,0x32,0x37,0x44,0x54,0x5c},
{0x3a,0x03,0x11,0x16,0x19,0x2b,0x3c,0x3d,0x3f,0x4a,0x4c,0x4d}]
npc=[
(0,10,4),(0,3,8),(0,11,14),(1,5,4),
(2,2,3),(2,6,4),(2,3,3),
(3,4,3),(3,7,3),(3,5,2),(3,5,10),(3,1,9),(3,2,10),(3,8,10)]
warps={
(0,5,5):(1,2,6),(0,13,5):(2,2,6),(0,12,11):(3,4,10),
(1,2,7):(0,5,6),(1,3,7):(0,5,6),
(2,2,7):(0,13,6),(2,3,7):(0,13,6),
(3,4,11):(0,12,12),(3,5,11):(0,12,12)}
maps=[(list((ROOT/'maps'/ (n+'.blk')).read_bytes()),
       list((ROOT/'gfx/blocksets'/(s+'.bst')).read_bytes()),w,h) for n,s,w,h in spec]
def walk(m,x,y):
 mp,b,w,h=maps[m]
 if (m,x,y) in warps:return True
 if not (0<=x<w*2 and 0<=y<h*2) or (m,x,y) in npc:return False
 tile=b[mp[(y//2)*w+(x//2)]*16+(y%2*2+1)*4+x%2*2+1]
 return tile in coll[m]
start=(0,10,12)
seen={start};queue=deque([start])
while queue:
 m,x,y=queue.popleft()
 for dx,dy in ((0,-1),(0,1),(-1,0),(1,0)):
  xx,yy=x+dx,y+dy
  if not walk(m,xx,yy):continue
  dest=warps.get((m,xx,yy),(m,xx,yy))
  if dest not in seen: seen.add(dest);queue.append(dest)
from collections import Counter
print('reachable',len(seen),'maps',dict(Counter(x[0] for x in seen)))
for m in range(4):
 print(spec[m][0], 'npcs next to reachable',[(x,y) for mm,x,y in npc if mm==m and any((m,x+dx,y+dy) in seen for dx,dy in ((0,-1),(0,1),(-1,0),(1,0)))])
print('warp entry accessible',[(k,v,k in seen, any((k[0],k[1]+dx,k[2]+dy) in seen for dx,dy in ((0,-1),(0,1),(-1,0),(1,0))) ) for k,v in warps.items()])
for m,(_,_,w,h) in enumerate(maps):
 print('map',m,'step passability')
 for y in range(h*2):
  print('%02d'%y,''.join('O' if (m,x,y) in seen else 'n' if (m,x,y) in npc else '.' if walk(m,x,y) else '#' for x in range(w*2)))

# Assertions: four maps reachable and all warp entrances have reachable neighbors.
assert {m for m,x,y in seen} == {0,1,2,3}, 'Not all maps reachable'
assert len(seen) >= 300, 'Unexpectedly small reachable region'
assert all(any((k[0], k[1]+dx, k[2]+dy) in seen for dx,dy in ((0,-1),(0,1),(-1,0),(1,0))) for k in warps), 'Door unreachable'
assert all(any((m,x+dx,y+dy) in seen for dx,dy in ((0,-1),(0,1),(-1,0),(1,0))) for m,x,y in npc), 'NPC not interactable'
print('PASS: map connectivity, warps, NPC adjacency')
