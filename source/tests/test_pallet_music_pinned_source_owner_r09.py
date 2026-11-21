#!/usr/bin/env python3
"""Full 48-event transcription independently recomputed from original pret."""
from pathlib import Path
import math,os,re
s=Path(__file__).resolve().parents[1]
up=Path(os.environ.get("PKYELLOW_SOURCE",s.parent.parent/"upstream/pokeyellow"))
original=(up/"audio/music/pallettown.asm").read_text()
assert original.startswith("Music_PalletTown_Ch1::")
ch=original.split("Music_PalletTown_Ch2::",1)[0]
assert re.search(r"(?m)^\s*tempo\s+160$",ch)
chunk=ch.split(".mainloop:",1)[1]
frequencies={"C_":261.63,"C#":277.18,"D_":293.66,"D#":311.13,"E_":329.63,
"F_":349.23,"F#":369.99,"G_":392.,"G#":415.30,"A_":440.,"A#":466.16,"B_":493.88}
actual=[];octave=0
for line in chunk.splitlines():
    line=line.split(";",1)[0].strip()
    if m:=re.fullmatch(r"octave\s+(\d+)",line):
        octave=int(m.group(1))
    elif m:=re.fullmatch(r"note\s+([A-G][#_]),\s*(\d+)",line):
        tone,length=m.groups()
        actual.append((round(frequencies[tone]*2**(octave-4)),int(length)))
    elif m:=re.fullmatch(r"rest\s+(\d+)",line):
        actual.append((0,int(m.group(1))))
    if len(actual)==48:break
assert len(actual)==48
generated=(s/"yellow_pallet_audio.c").read_text()
observed=[tuple(map(int,x)) for x in re.findall(r"^\s*\{(\d+),(\d+)\},\s*$",generated,re.M)]
assert observed==actual,(observed[:10],actual[:10],len(observed))
assert sum(n==0 for n,_ in observed)==2
print("PALLET_PINNED_SOURCE_EXACT_48_NOTE_REST_EVENTS_PASS")
