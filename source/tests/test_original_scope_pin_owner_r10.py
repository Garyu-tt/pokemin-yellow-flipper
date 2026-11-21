#!/usr/bin/env python3
"""Independent original-source dimensions must match runtime audit denominators."""
from pathlib import Path
import os,re
r=Path(__file__).resolve().parents[1]
up=Path(os.environ.get("PKYELLOW_SOURCE",r.parent.parent/"upstream/pokeyellow"))
maps=(up/"constants/map_constants.asm").read_text()
species=(up/"constants/pokedex_constants.asm").read_text()
moves=(up/"constants/move_constants.asm").read_text()
maprows=[(x,int(w),int(h)) for x,w,h in re.findall(
    r'^\s*map_const\s+([A-Z0-9_]+),\s*(\d+),\s*(\d+)',maps,re.M)]
dex=re.findall(r'^\s*const\s+DEX_([A-Z0-9_]+)',species,re.M)
pre=moves.split("DEF NUM_ATTACKS",1)[0]
original_moves=re.findall(r'^\s*const\s+([A-Z0-9_]+)',pre,re.M)
assert len(maprows)==249 and len(set(r[0] for r in maprows))==249
assert sum(w>0 and h>0 for _,w,h in maprows)==227
assert len(dex)==151
assert len(original_moves)==166 and original_moves[0]=="NO_MOVE"
header=(r/"yellow_original_game_audit.h").read_text()
for const,number in [
  ("YELLOW_ORIGINAL_MAP_IDENTIFIERS",249),
  ("YELLOW_ORIGINAL_MAP_GEOMETRIES",227),
  ("YELLOW_ORIGINAL_SPECIES_IDS",151),
  ("YELLOW_ORIGINAL_MOVE_IDS",165)
]:
    assert re.search(r"#define "+const+r" "+str(number)+r"u\b",header),(const,number)
runtime=(r/"yellow_original_game_audit.c").read_text()
for required in (
    "original_story_to_credits_verified=false",
    "original_all_maps_events_verified=false",
    "full_original_multichannel_audio_verified=false",
    "original_game_mechanics_parity_verified=false",
    "physical_flipper_full_playthrough_verified=false",
    "return false;"
):
    assert required in runtime,required
print("ORIGINAL_SOURCE_MAP_IDS_249_NONZERO_227_DEX_151_MOVES_165_AUDIT_PASS")
