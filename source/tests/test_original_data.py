#!/usr/bin/env python3
"""Fail if the R0.5 Pokémon stats/moves diverge from the pinned disassembly."""
from pathlib import Path
import re,os
root=Path(os.environ.get("PKYELLOW_SOURCE",
 str(Path(__file__).resolve().parents[2]/"upstream/pokeyellow")))
source=(root/"data/moves/moves.asm").read_text()
expected_moves={
 "GUST":("40","NORMAL","100","35"),
 "TACKLE":("35","NORMAL","95","35"),
 "TAIL_WHIP":("0","NORMAL","100","30"),
 "GROWL":("0","NORMAL","100","40"),
 "THUNDERSHOCK":("40","ELECTRIC","100","30"),
 "QUICK_ATTACK":("40","NORMAL","100","30")
}
for name,vals in expected_moves.items():
 pattern=r"\bmove\s+"+name+r",\s*[^,]+,\s*(\d+),\s*([A-Z_]+),\s*(\d+),\s*(\d+)"
 m=re.search(pattern,source)
 assert m,(name,"not found")
 assert m.groups()==vals,(name,m.groups(),vals)
for name,stats,exp,growth in [
 ("pikachu",[35,55,30,90,50],82,"GROWTH_MEDIUM_FAST"),
 ("eevee",[55,55,50,55,65],92,"GROWTH_MEDIUM_FAST"),
 ("pidgey",[40,45,40,56,35],55,"GROWTH_MEDIUM_SLOW"),
 ("rattata",[30,56,35,72,25],57,"GROWTH_MEDIUM_FAST")
]:
 s=(root/"data/pokemon/base_stats"/(name+".asm")).read_text()
 m=re.search(r"db\s+(\d+),\s*(\d+),\s*(\d+),\s*(\d+),\s*(\d+)",s)
 assert m and list(map(int,m.groups()))==stats,(name,"stats",m.groups() if m else None)
 assert re.search(r"db\s+"+str(exp)+r"\s*; base exp",s), (name,"exp")
 assert growth in s,(name,"growth")
ev=(root/"data/pokemon/evos_moves.asm").read_text()
m=re.search(r"^PikachuEvosMoves:(.*?)(?=^RaichuEvosMoves:)",ev,re.M|re.S)
assert m
assert re.search(r"db\s+6,\s*TAIL_WHIP",m.group(1))
assert re.search(r"db\s+8,\s*THUNDER_WAVE",m.group(1))
# Extra R0.8 checks: original move effects and accuracy stage ratios.
effects={
 "SAND_ATTACK":"ACCURACY_DOWN1_EFFECT",
 "THUNDERSHOCK":"PARALYZE_SIDE_EFFECT1",
 "TAIL_WHIP":"DEFENSE_DOWN1_EFFECT",
 "GROWL":"ATTACK_DOWN1_EFFECT",
}
for name,effect in effects.items():
 assert re.search(r"\bmove\s+"+name+r",\s*"+effect+r",",source),(name,effect)
table=(root/"data/battle/stat_modifiers.asm").read_text()
pairs=re.findall(r"^\s*db\s+(\d+),\s*(\d+)",table,re.M)
assert [(int(a),int(b)) for a,b in pairs[:13]]==[
 (25,100),(28,100),(33,100),(40,100),(50,100),(66,100),
 (1,1),(15,10),(2,1),(25,10),(3,1),(35,10),(4,1)]
side=(root/"engine/battle/effects.asm").read_text()
assert "10 percent + 1" in side
core=(root/"engine/battle/core.asm").read_text()
assert "cp 25 percent" in core
capture=(root/"engine/items/item_effects.asm").read_text()
assert "Status = 12" in capture
macro=(root/"macros/data.asm").read_text()
assert 'DEF percent EQUS "* $ff / 100"' in macro
assert (25*255//100)==63
assert (50*255//100+1)==128
assert "cp 50 percent + 1" in core
assert "call CriticalHitTest" in core
crit=(root/"engine/battle/core.asm").read_text()
assert "CriticalHitTest:" in crit and "rlc a" in crit
assert "HighCriticalMoves:" in (root/"data/battle/critical_hit_moves.asm").read_text()
assert "cp QUICK_ATTACK" in core
print("R09_UPSTREAM_PRIORITIES_CRIT_AND_PERCENT_MACRO_PASS")
print("R08_UPSTREAM_STATUS_ACCURACY_CAPTURE_CONSTANTS_PASS")
# R10: lock new move metadata and learnsets to the exact checked-out Yellow.
more={
 "THUNDER_WAVE":("PARALYZE_EFFECT",0,"ELECTRIC",100,20),
 "DOUBLE_TEAM":("EVASION_UP1_EFFECT",0,"NORMAL",100,15),
 "SLAM":("NO_ADDITIONAL_EFFECT",80,"NORMAL",75,20),
 "THUNDERBOLT":("PARALYZE_SIDE_EFFECT1",95,"ELECTRIC",100,15),
 "HYPER_FANG":("FLINCH_SIDE_EFFECT1",80,"NORMAL",90,15),
}
for name,(effect,power,typ,acc,pp) in more.items():
 pattern=r"^\s*move\s+"+name+r",\s*"+effect+r",\s*"+str(power)+r",\s*"+typ+r",\s*"+str(acc)+r",\s*"+str(pp)+r"\s*$"
 assert re.search(pattern,source,re.M),(name,pattern)
learnsets={
 "Pikachu":{6:"TAIL_WHIP",8:"THUNDER_WAVE",11:"QUICK_ATTACK",15:"DOUBLE_TEAM",20:"SLAM",26:"THUNDERBOLT"},
 "Pidgey":{5:"SAND_ATTACK",12:"QUICK_ATTACK"},
 "Rattata":{7:"QUICK_ATTACK",14:"HYPER_FANG"},
}
for species,requirements in learnsets.items():
 matches=re.search(r"^"+species+r"EvosMoves:(.*?)(?=^[A-Za-z0-9_]+EvosMoves:)",ev,re.M|re.S)
 assert matches,species
 for level,move in requirements.items():
  assert re.search(r"^\s*db\s+"+str(level)+r",\s*"+move+r"\b",matches.group(1),re.M),(species,level,move)
print("R10_UPSTREAM_MOVE_METADATA_AND_LEARNSETS_PASS")
print("R05_UPSTREAM_SPECIES_MOVE_LEARNSET_PASS")

# R0.12-C: residual poison/burn mechanics from original Gen1 assembly.
residual=(root/"engine/battle/core.asm").read_text()
assert "HandlePoisonBurnLeechSeed:" in residual
assert "and (1 << BRN) | (1 << PSN)" in residual
assert "max HP/16" in residual
assert "HalveAttackDueToBurn:" in residual
assert "25 percent" in residual
items=(root/"engine/items/item_effects.asm").read_text()
assert "Burn/Paralysis/Poison: Status = 12" in items
print("R12C_UPSTREAM_BURN_POISON_RESIDUAL_AND_CAPTURE_PASS")

# R12C2: validate sleep decrement, Gen1 freeze gating and Toxic counter path.
core=(root/"engine/battle/core.asm").read_text()
assert "CheckPlayerStatusConditions:" in core and "CheckEnemyStatusConditions:" in core
assert "ld [wBattleMonStatus], a ; decrement number of turns left" in core
assert "bit FRZ, [hl] ; frozen?" in core
assert "ld de, wPlayerToxicCounter" in core
assert "ld de, wEnemyToxicCounter" in core
assert "bit BADLY_POISONED, [hl]" in core
items=(root/"engine/items/item_effects.asm").read_text()
assert "Freeze/Sleep:          Status = 25" in items
print("R12C2_UPSTREAM_SLEEP_FREEZE_TOXIC_STATUS_PASS")
