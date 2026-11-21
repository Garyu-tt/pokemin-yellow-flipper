#!/usr/bin/env python3
"""Pinned MtMoon1F exact 10/10 original slots and no substitutions."""
from pathlib import Path
import re,os
source=Path(__file__).resolve().parents[1]
original=Path(os.environ.get("PKYELLOW_SOURCE",source.parent.parent/"upstream/pokeyellow"))
table=(original/"data/wild/maps/MtMoon1F.asm").read_text()
match=re.search(r"def_grass_wildmons\s+(\d+)",table)
assert match and int(match.group(1))==10
rows=re.findall(r"(?m)^\s*db\s+(\d+),\s*(ZUBAT|GEODUDE|SANDSHREW|CLEFAIRY)\s*$",table)
assert len(rows)==10
dex={"ZUBAT":41,"GEODUDE":74,"SANDSHREW":27,"CLEFAIRY":35}
native={"GEODUDE":"YK_GEODUDE","SANDSHREW":"YK_SANDSHREW"}
expected=[(dex[k],int(level),bool(k in native),native.get(k)) for level,k in rows]
assert expected==[
 (41,8,False,None),(41,9,False,None),(74,10,True,"YK_GEODUDE"),
 (41,6,False,None),(41,7,False,None),(41,10,False,None),
 (74,10,True,"YK_GEODUDE"),(41,11,False,None),
 (27,12,True,"YK_SANDSHREW"),(35,11,False,None)]
c=(source/"yellow_mtmoon_wild.c").read_text()
from_source=re.findall(r"(?m)^\s*\{(\d+),(\d+),([01]),(YK_[A-Z_]+|0)\}",c)
actual=[(int(n),int(l),bool(int(ready)),None if k=="0" else k)
        for n,l,ready,k in from_source]
assert actual==expected,(actual,expected)
assert "#define YELLOW_MTMOON_WILD_RATE 10u" in c
assert "if(!s->native_supported)return false;" in c
battle=(source/"yellow_battle.c").read_text()
assert "species==YK_SANDSHREW && level==10" in battle
evos=(original/"data/pokemon/evos_moves.asm").read_text()
assert re.search(r"SandshrewEvosMoves:.*?db\s+10,\s*SAND_ATTACK",evos,re.S)
for who in ("geodude","sandshrew"):
    base=(original/f"data/pokemon/base_stats/{who}.asm").read_text()
    assert "db 255 ; catch rate" in base
core=(source/"yellow_core.c").read_text()
pc=(source/"yellow_pc_box.c").read_text()
assert "m->species!=YK_GEODUDE" in core and "m->species!=YK_SANDSHREW" in core
assert "case YK_GEODUDE:case YK_SANDSHREW:break;" in pc
print("PINNED_MTMOON_10_OF_10_SLOTS_3_NATIVE_7_UNSUPPORTED_SAND_ATTACK_SAVE_PC_PASS")
