#!/usr/bin/env python3
from pathlib import Path
import os,re
here=Path(__file__).resolve().parents[1]
source=Path(os.environ.get("PKYELLOW_SOURCE",
            str(here.parents[1]/"upstream/pokeyellow")))/"data/types/type_matchups.asm"
expected=[]
codes={"NORMAL":0,"FIGHTING":1,"FLYING":2,"POISON":3,"GROUND":4,
"ROCK":5,"BIRD":6,"BUG":7,"GHOST":8,"FIRE":20,"WATER":21,
"GRASS":22,"ELECTRIC":23,"PSYCHIC_TYPE":24,"ICE":25,"DRAGON":26}
factors={"NO_EFFECT":0,"NOT_VERY_EFFECTIVE":5,"SUPER_EFFECTIVE":20}
for l in source.read_text().splitlines():
    m=re.match(r"\s*db\s+([A-Z_]+),\s*([A-Z_]+),\s*([A-Z_]+)",l)
    if m and m[3] in factors:
        expected.append((codes[m[1]],codes[m[2]],factors[m[3]]))
actual=[tuple(map(int,m)) for m in re.findall(r"\{(\d+),(\d+),(\d+)\},",
          (here/"yellow_type_matchups.generated.h").read_text())]
assert actual==expected,(len(actual),len(expected))
assert len(expected)==82,len(expected)
assert len({(a,b) for a,b,v in actual})==82
assert (8,24,0) in actual, "Ghost -> Psychic Gen1 immunity lost"
assert (7,3,20) in actual, "Bug -> Poison Gen1 interaction lost"
print("R12_GENERATED_CHART_82_ENTRIES_UPSTREAM_EQUAL_PASS")
