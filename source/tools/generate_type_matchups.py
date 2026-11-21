#!/usr/bin/env python3
"""Generate static Gen1 type chart from pinned pret/pokeyellow.
Original Ghost/Psychic and Bug/Poison behavior intentionally preserved.
"""
import hashlib
import os
import re
from pathlib import Path

root=Path(__file__).resolve().parents[1]
upstream=Path(os.environ.get("PKYELLOW_SOURCE",
    str(root.parents[1]/"upstream/pokeyellow")))
source=upstream/"data/types/type_matchups.asm"
rows=[]
codes={"NORMAL":0,"FIGHTING":1,"FLYING":2,"POISON":3,"GROUND":4,
"ROCK":5,"BIRD":6,"BUG":7,"GHOST":8,"FIRE":20,"WATER":21,
"GRASS":22,"ELECTRIC":23,"PSYCHIC_TYPE":24,"ICE":25,"DRAGON":26}
amounts={"NO_EFFECT":0,"NOT_VERY_EFFECTIVE":5,"SUPER_EFFECTIVE":20}
for line in source.read_text().splitlines():
    m=re.match(r"\s*db\s+([A-Z_]+),\s*([A-Z_]+),\s*([A-Z_]+)",line)
    if m and m.group(3) in amounts:
        a,b,e=m.groups()
        rows.append((codes[a],codes[b],amounts[e]))
assert len(rows)>=75,len(rows)
out=root/"yellow_type_matchups.generated.h"
lines=[
 "/* Generated from pret/pokeyellow data/types/type_matchups.asm.",
 " * Regenerate using tools/generate_type_matchups.py. Do not edit. */",
 "#ifndef YELLOW_TYPE_MATCHUPS_GENERATED_H",
 "#define YELLOW_TYPE_MATCHUPS_GENERATED_H",
 "typedef struct {uint8_t attack,defend,multiplier_tenths;} YTypeMatchup;",
 "static const YTypeMatchup yellow_type_matchups[]={",
]
lines.extend("    {%u,%u,%u}," % r for r in rows)
lines.extend([
 "};",
 "#define YELLOW_TYPE_MATCHUP_COUNT (sizeof(yellow_type_matchups)/sizeof(yellow_type_matchups[0]))",
 "#endif",
])
out.write_text("\n".join(lines)+"\n")
print("TYPE_MATCHUPS",len(rows),"source_sha256",hashlib.sha256(source.read_bytes()).hexdigest())
