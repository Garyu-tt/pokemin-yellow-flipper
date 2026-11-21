from pathlib import Path
import os,re
s=Path(__file__).resolve().parents[1]
u=Path(os.environ.get('PKYELLOW_SOURCE',s.parent.parent/'upstream/pokeyellow'))
b=(u/'data/pokemon/base_stats/clefairy.asm').read_text()
e=(u/'data/pokemon/evos_moves.asm').read_text()
w=(u/'data/wild/maps/MtMoon1F.asm').read_text()
m=(u/'data/moves/moves.asm').read_text()
assert 'db  70,  45,  48,  35,  60' in b
assert 'db NORMAL, NORMAL' in b
assert 'db 150 ; catch rate' in b and 'db 68 ; base exp' in b
assert 'db POUND, GROWL, NO_MOVE, NO_MOVE' in b
assert 'db GROWTH_FAST' in b
assert re.search(r'ClefairyEvosMoves:.*?EVOLVE_ITEM, MOON_STONE.*?db 13, SING',e,re.S)
assert re.search(r'db\s+11,\s+CLEFAIRY',w)
assert re.search(r'move POUND,\s+NO_ADDITIONAL_EFFECT,\s+40, NORMAL,\s+100, 35',m)
a=(s/'yellow_battle.c').read_text()
assert 'y_source_growth_threshold(35,level)' in a
assert 'YM_POUND' in a
assert 'native_clefairy_extension=true' in (s/'yellow_flipper.c').read_text()
print('CLEFAIRY_PINNED_SOURCE_FAST_POUND_LV11_SINGLE_SLOT_PASS')
