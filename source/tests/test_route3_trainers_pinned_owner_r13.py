#!/usr/bin/env python3
"""Independent pin against original Route3 objects, scripts and trainer parties."""
from pathlib import Path
import os,re
source=Path(__file__).resolve().parents[1]
up=Path(os.environ.get("PKYELLOW_SOURCE",source.parent.parent/"upstream/pokeyellow"))
objects=(up/"data/maps/objects/Route3.asm").read_text()
scripts=(up/"scripts/Route3.asm").read_text()
parties=(up/"data/trainers/parties.asm").read_text()
native=(source/"yellow_route3_trainers.c").read_text()
expected=[
  (2,16,9,"LEFT",2,["PIDGEY","PIDGEY"],9,"COOLTRAINER_F1","LASS",1),
  (4,23,4,"LEFT",4,["RATTATA","NIDORAN_M"],10,"COOLTRAINER_F2","LASS",2),
  (6,24,6,"RIGHT",3,["CATERPIE","METAPOD"],11,"YOUNGSTER5","BUG_CATCHER",6)
]
headers=re.findall(r"Route3TrainerHeader(\d+):\s*\n\s*trainer\s+EVENT_BEAT_ROUTE_3_TRAINER_(\d+),\s*(\d+)",scripts)
assert len(headers)==8 and headers==[(str(i),str(i),str(r)) for i,r in enumerate([2,3,2,1,4,3,3,2])]
def trainer_party(kind,number):
    block=parties.split(kind.title().replace("_","")+"Data:",1)
    # Source supports exact section names, including CamelCase.
    name={"LASS":"LassData:","BUG_CATCHER":"BugCatcherData:"}[kind]
    text=parties.split(name,1)[1]
    # Party numbers are numbered across the *entire trainer class*.
    text=re.split(r"(?m)^\w+Data:",text,maxsplit=1)[0]
    assert "; Route 3" in text
    rows=re.findall(r"^\s*db\s+([^\n;]+)",text,re.M)
    row=rows[number-1].split(",")
    level=int(row[0].strip())
    species=[x.strip() for x in row[1:] if x.strip()!="0"]
    return level,species
for idx,x,y,direction,rng,kinds,lvl,obj,klass,n in expected:
    line=re.search(r"^\s*object_event\s+"+str(x)+r",\s*"+str(y)+
         r",\s*SPRITE_\w+,\s*STAY,\s*"+direction+
         r",\s*TEXT_ROUTE3_"+obj+r",\s*OPP_"+klass+r",\s*"+str(n)+r"\s*$",objects,re.M)
    assert line,(idx,obj)
    assert trainer_party(klass,n)==(lvl,kinds),(klass,n)
    mapping=("{" + str(idx)+","+str(x)+","+str(y)+",Y"+direction.title()+","+
             str(rng)+",2,{"+",".join("YK_"+k for k in kinds)+"},{"+
             str(lvl)+","+str(lvl)+"}}")
    assert mapping in native,(idx,kinds)
assert len(re.findall(r"^\s*object_event\s+\d+,\s*\d+,.*OPP_",objects,re.M))==8
assert len(re.findall(r"\{\d+,\d+,\d+,Y(?:Left|Right),\d+,2,\{",native))==3
print("ROUTE3_PINNED_3_OF_8_OBJECTS_PARTIES_SIGHT_RANGES_TRAINER_HEADERS_PASS")
