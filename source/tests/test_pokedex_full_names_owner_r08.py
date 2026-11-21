#!/usr/bin/env python3
"""Entire National Dex name table must exactly match pinned original source."""
from pathlib import Path
import os,re
root=Path(__file__).resolve().parents[1]
original=Path(os.environ.get("PKYELLOW_SOURCE",root.parent.parent/"upstream/pokeyellow"))
source=(original/"constants/pokedex_constants.asm").read_text()
pairs=[(m.group(1),int(m.group(2))) for m in re.finditer(
    r"^\s*const\s+DEX_([A-Z0-9_]+)\s*;\s*(\d+)\s*$",source,re.M)]
assert len(pairs)==151
assert [n for _,n in pairs]==list(range(1,152))
built=(root/"yellow_dex_names.c").read_text()
names=[m.group(1) for m in re.finditer(r'^\s+"([A-Z0-9_]+)",\s*/\*\s*\d+\s*\*/',built,re.M)]
assert names==[name for name,_ in pairs]
assert len(set(names))==151
print("DEX_NAMES_ALL_151_MATCH_PINNED_PRET_POKEYELLOW_PASS")
