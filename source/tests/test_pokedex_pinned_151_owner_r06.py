#!/usr/bin/env python3
"""Pinned original Gen1 National Dex identity proof for native battle species."""
from pathlib import Path
import os,re
src=Path(__file__).resolve().parents[1]
root=Path(os.environ.get("PKYELLOW_SOURCE",src.parent.parent/"upstream/pokeyellow"))
source=(root/"constants/pokedex_constants.asm").read_text()
matches=re.findall(r"^\s*const\s+DEX_([A-Z0-9_]+)\s*;\s*(\d+)",source,re.M)
numbers={name:int(num) for name,num in matches}
assert len(numbers)==151,len(numbers)
assert sorted(numbers.values())==list(range(1,152))
native=["EEVEE","PIDGEY","RATTATA","PIKACHU","CATERPIE","METAPOD",
        "PIDGEOTTO","NIDORAN_F","NIDORAN_M","GEODUDE","ONIX",
        "DIGLETT","SANDSHREW"]
header=(src/"yellow_battle.h").read_text()
for i,name in enumerate(native):
    assert re.search(r"\bYK_"+name+r"\s*=\s*"+str(i)+r"\b",header),(i,name)
module=(src/"yellow_pokedex_ledger.c").read_text()
packed=re.search(r"static const uint8_t native_to_dex\[\]\s*=\s*\{([^}]+)\}",module)
assert packed
values=[int(x) for x in re.findall(r"\d+",packed.group(1))]
assert values==[numbers[x] for x in native]
assert values==[133,16,19,25,10,11,17,29,32,74,95,50,27]
print("POKEDEX_FULL_151_ORIGINAL_IDS_AND_NATIVE_13_MAPPING_PASS")
