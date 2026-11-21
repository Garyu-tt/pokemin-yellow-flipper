#!/usr/bin/env python3
"""Independent pinned Gen1 Route 2/Forest/gate asset and event provenance check."""
from pathlib import Path
import hashlib,json,os,re,subprocess
src=Path(__file__).resolve().parents[1]
up=Path(os.environ.get('PKYELLOW_SOURCE',src.parent.parent/'upstream/pokeyellow'))
prov=json.loads((src/'assets/route13_provenance.json').read_text())
sha=lambda f:hashlib.sha256(f.read_bytes()).hexdigest()
assert prov['git_commit']=='b6696f24b3be84fe64cf702f5a15ba40c66b7f39'
# Permit this exact local repository despite shared-volume ownership without
# changing global Git configuration; reject parent-repository discovery.
up=up.resolve(strict=True)
git=['git','-c',f'safe.directory={up}','-C',str(up)]
assert (up/'.git').exists(), 'upstream checkout must have its own .git metadata'
assert Path(subprocess.check_output(git+['rev-parse','--show-toplevel'],text=True).strip()).resolve()==up
assert subprocess.check_output(git+['rev-parse','HEAD'],text=True).strip()==prov['git_commit']
for name,h in prov['source_sha256'].items():
    f=src/('tools/'+name if name=='generate_assets.py' else name)
    assert sha(f)==h,name
assert sha(up/'data/tilesets/collision_tile_ids.asm')==prov['source_collisions_sha256']
assets=(src/'yellow_assets.c').read_text()
def arr(prefix,i):
    m=re.search(r'(?:static )?const uint8_t '+prefix+'_'+str(i)+r'\[\d+\] = \{([^}]+)\}',assets)
    assert m,(prefix,i)
    return bytes(int(x,16) for x in re.findall(r'0x[0-9a-f]+',m.group(1)))
collisions={
 'overworld':{0x00,0x10,0x1b,0x20,0x21,0x23,0x2c,0x2d,0x2e,0x30,0x31,0x33,0x39,0x3c,0x3e,0x52,0x54,0x58,0x5b},
 'forest':{0x1e,0x20,0x2e,0x30,0x34,0x37,0x39,0x3a,0x40,0x51,0x52,0x5a,0x5c,0x5e,0x5f},
 'gate':{0x01,0x12,0x14,0x1a,0x1c,0x37,0x38,0x3b,0x3c,0x5e}
}
collision_src=(up/'data/tilesets/collision_tile_ids.asm').read_text()
for tiles,label in [('overworld','Overworld_Coll'),('forest','Forest_Coll'),('gate','ForestGate_Coll')]:
    m=re.search(r'^'+label+r'::([\s\S]{0,120}?)^\s*coll_tiles\s+([^\n]+)',collision_src,re.M)
    assert m,label
    assert {int(x,16) for x in re.findall(r'\$([0-9a-fA-F]+)',m.group(2))}==collisions[tiles]
for i,m in enumerate(prov['maps'],start=7):
    n=m['name'];tiles=m['tileset']
    assert arr('ymap',i)==(up/f'maps/{n}.blk').read_bytes(),n
    assert arr('yblock',i)==(up/f'gfx/blocksets/{tiles}.bst').read_bytes(),n
    assert len(arr('ytile',i))==96*8,n
    assert arr('ycoll',i)==bytes(int(x in collisions[tiles]) for x in range(96)),n
    assert len(arr('ymap',i))==m['width_blocks']*m['height_blocks']
    for k,f in [('map_blk_sha256',f'maps/{n}.blk'),
                ('blockset_sha256',f'gfx/blocksets/{tiles}.bst'),
                ('tileset_png_sha256',f'gfx/tilesets/{tiles}.png'),
                ('object_events_sha256',f'data/maps/objects/{n}.asm'),
                ('header_sha256',f'data/maps/headers/{n}.asm')]:
        assert sha(up/f)==m[k],(n,k)
route2=(up/'data/maps/objects/Route2.asm').read_text()
forest=(up/'data/maps/objects/ViridianForest.asm').read_text()
sg=(up/'data/maps/objects/ViridianForestSouthGate.asm').read_text()
ng=(up/'data/maps/objects/ViridianForestNorthGate.asm').read_text()
for body,coords in [
    (route2,[(3,11,'VIRIDIAN_FOREST_NORTH_GATE'),(3,43,'VIRIDIAN_FOREST_SOUTH_GATE')]),
    (forest,[(1,0,'VIRIDIAN_FOREST_NORTH_GATE'),(2,0,'VIRIDIAN_FOREST_NORTH_GATE'),
             (15,47,'VIRIDIAN_FOREST_SOUTH_GATE'),(18,47,'VIRIDIAN_FOREST_SOUTH_GATE')]),
    (sg,[(4,0,'VIRIDIAN_FOREST'),(4,7,'LAST_MAP')]),
    (ng,[(4,7,'VIRIDIAN_FOREST'),(4,0,'LAST_MAP')])
]:
    for x,y,target in coords:
        assert re.search(r'\bwarp_event\s+'+str(x)+r',\s*'+str(y)+r',\s*'+target+r'\b',body),(x,y,target)
headers=(up/'data/maps/headers/ViridianCity.asm').read_text()
assert 'connection north, Route2, ROUTE_2, 5' in headers
headers=(up/'data/maps/headers/Route2.asm').read_text()
assert 'connection south, ViridianCity, VIRIDIAN_CITY, -5' in headers
assert 'connection north, PewterCity, PEWTER_CITY, -5' in headers
assert len(re.findall(r'^\s*object_event\b',forest,re.M))==10
assert len(re.findall(r'^\s*object_event\b',route2,re.M))==2
print('R13_PINNED_ASSETS_OBJECTS_CONNECTIONS_SHA256_ALL_PASS')
