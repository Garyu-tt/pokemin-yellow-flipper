from pathlib import Path
import os
root=Path(os.environ["PKYELLOW_SOURCE"])
setmap=(root/"engine/events/set_blackout_map.asm").read_text()
constants=(root/"constants/map_constants.asm").read_text()
assert "ld a, [wLastMap]" in setmap
assert "ld [wLastBlackoutMap], a" in setmap
assert "map_const VIRIDIAN_CITY," in constants
assert "; $01" in next(x for x in constants.splitlines() if "map_const VIRIDIAN_CITY," in x)
assert "; $29" in next(x for x in constants.splitlines() if "map_const VIRIDIAN_POKECENTER," in x)
print("R012C3_PINNED_ORIGINAL_MAP_ID_NOT_NATIVE_ENUM_PASS")
