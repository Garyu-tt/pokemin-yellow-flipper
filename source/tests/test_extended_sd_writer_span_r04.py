#!/usr/bin/env python3
"""Guard the real SD writer upper bound for every new native map.
Old bug: legacy v060/v062 compiled and passed in-memory save tests, but
save_game only wrote .v1 sidecars for map <= Cerulean City (id 18).
"""
from pathlib import Path
import re
S=Path(__file__).resolve().parents[1]
c=(S/"yellow_flipper.c").read_text()
h=(S/"yellow_core.h").read_text()
e=(S/"yellow_exact_position_v1.c").read_text()
assert re.search(r"g->map\s*>=\s*YMapPewterCity\s*&&\s*g->map\s*<=\s*YMapViridianMart",c)
assert "map>YMapViridianMart" in e
assert "YMapRoute9,YMapRoute22,YMapViridianMart" in h
assert "#define YELLOW_SAVE_MAGIC 0x59464C50u" in h
assert "if(ok)ok=exact_position_write(storage,pos_dest,&exact)" in c
assert "if(ok)ok=atomic_write_inactive(storage,dest,next)" in c
assert c.index("exact_position_write(storage,pos_dest")<c.index("atomic_write_inactive(storage,dest,next)")
assert "yellow_exact_position_restore(g,parent,&exact)" in c
assert "POSITION_SAVE_A" in c and "POSITION_SAVE_B" in c
print("REAL_SD_WRITER_MAPS_11_THRU_21_FAIL_CLOSED_PROTECTED_PASS")
