from pathlib import Path
import os,re
upstream=Path(os.environ['PKYELLOW_SOURCE'])
special=(upstream/'engine/overworld/special_warps.asm').read_text()
fly=(upstream/'data/maps/special_warps.asm').read_text()
black=(upstream/'engine/events/black_out.asm').read_text()
source=(Path(__file__).resolve().parents[1]/'yellow_core.h').read_text()
assert re.search(r'bit\s+BIT_ESCAPE_WARP,\s*\[hl\]',special)
assert re.search(r'ld\s+a,\s*\[wLastBlackoutMap\]',special)
assert re.search(r'\.ViridianCity:\s+fly_warp\s+VIRIDIAN_CITY,\s*23,\s*26',fly)
assert re.search(r'fly_warp_spec\s+VIRIDIAN_CITY,\s*\.ViridianCity',fly)
assert 'set BIT_ESCAPE_WARP, [hl]' in black
ids=(upstream/'constants/map_constants.asm').read_text()
assert re.search(r'map_const\s+VIRIDIAN_CITY,.*;\s*\$01',ids)
assert re.search(r'map_const\s+VIRIDIAN_POKECENTER,.*;\s*\$29',ids)
assert re.search(r'YMapViridian,\s*YMapViridianCenter',source)
print('BLACKOUT_SPAWN_ORIGINAL_PIN_PASS')
