from pathlib import Path
import os,re
src=Path(__file__).resolve().parents[1]
up=Path(os.environ.get('PKYELLOW_SOURCE',src.parent.parent/'upstream/pokeyellow'))
base=(up/'data/pokemon/base_stats/zubat.asm').read_text()
evo=(up/'data/pokemon/evos_moves.asm').read_text()
wild=(up/'data/wild/maps/MtMoon1F.asm').read_text()
moves=(up/'data/moves/moves.asm').read_text()
assert 'db  40,  45,  35,  55,  40' in base
assert 'db POISON, FLYING' in base and 'db 54 ; base exp' in base
assert 'db LEECH_LIFE, NO_MOVE' in base
assert re.search(r'ZubatEvosMoves:.*?db 10, SUPERSONIC',evo,re.S)
assert 'move SUPERSONIC,   CONFUSION_EFFECT,             0, NORMAL,        55, 20' in moves
assert 'move LEECH_LIFE,   DRAIN_HP_EFFECT,             20, BUG,          100, 15' in moves
assert len(re.findall(r'db\s+\d+, ZUBAT',wild))==6
core=(src/'yellow_core.c').read_text()
assert 'YM_SUPERSONIC' in core and 'YM_LEECH_LIFE' in core
assert 'native_zubat_extension' in core
assert 'yellow_mtmoon_zubat_wild_slot' in core
assert 'native_zubat_extension=true' in (src/'yellow_flipper.c').read_text()
print('ZUBAT_PINNED_STATS_MOVES_6_WILD_SLOTS_NATIVE_RUNTIME_PASS')
