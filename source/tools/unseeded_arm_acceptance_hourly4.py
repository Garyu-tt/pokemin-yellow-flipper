#!/usr/bin/env python3
"""Unseeded ARM native-input Oak Parcel -> paid Antidote -> Bag -> SD restart.
The only RAM fixture is poison on phase 2, AFTER the entire natural quest.
"""
import hashlib,json,sys,os
from pathlib import Path
from unseeded_arm_trace_hourly4 import Replay,ROOT,PROJECT,KEYS,SOURCE
sys.path.insert(0,str(PROJECT/"emulation/Flipper-Zero-Emulator"))
from fapemu.runner import Runner
from fapemu.api import InputKeyBack,InputKeyDown,InputKeyOk,InputTypeShort
FAP=Path(os.environ.get("POKEMON_ARM_FAP",str(SOURCE/"dist/yellow_native.fap")))
SD=ROOT/"unseeded-sd"
REPORT=ROOT/"unseeded-arm-acceptance.json"
# Genuine input; no RAM changes to map/story/parcel/money/stock.
MENU_BAG_USE_SAVE=[InputKeyBack,InputKeyDown,InputKeyOk,InputKeyOk,
                   InputKeyDown,InputKeyOk,InputKeyBack,InputKeyBack,
                   InputKeyDown,InputKeyOk]
def state_of(r):
    v=[v for v in r.env.viewports if v.enabled and v.input_ctx]
    if len(v)!=1:return None,None
    ctx=v[0].input_ctx
    d=r.machine.read(ctx,704)
    s=dict(map=d[2],x=int.from_bytes(d[0:1],"little",signed=True),
       y=int.from_bytes(d[1:2],"little",signed=True),screen=d[16],
       story=d[20],parcel=d[692],stock=d[696],money=d[698:701].hex(),
       status=d[121],saved=d[19],integrity=d[693],
       item_error=d[697],money_error=d[701],
       steps=int.from_bytes(d[6:8],"little"),frame=r.frames)
    return s,ctx
class Reopen(Runner):
    def __init__(self,phase):
        super().__init__(FAP,sd_root=SD,headless=True)
        self.phase=phase
        self.checked=False
        self.poison_fixture=False
        self.actions=MENU_BAG_USE_SAVE if phase==1 else []
        self.idx=0
        self.next_ms=0
        self.log=[]
        self.initial=None
        self.final=None
        self.complete=False
    def _service(self):
        t=self.env.now_ms()
        s,ctx=state_of(self)
        if s and not self.checked and t>=1600:
            self.initial=s
            if not (s["map"]==21 and (s["x"],s["y"])==(2,5) and
                s["story"]==4 and s["parcel"]==3 and s["integrity"]==0 and
                s["money"]=="002900" and s["stock"]==(1 if self.phase==1 else 0) and
                s["money_error"]==0 and s["item_error"]==0):
                raise RuntimeError("reopen persisted state incorrect: "+repr(s))
            self.checked=True
            if self.phase==1:
                # Explicitly labeled status fixture, not natural poison gameplay.
                self.machine.write(ctx+121,bytes([3]))
                self.env.dirty=True
                self.poison_fixture=True
            self.next_ms=t+220
        if self.checked and s and self.phase==1:
            if self.idx<len(self.actions) and t>=self.next_ms:
                key=self.actions[self.idx]
                self._deliver_input([(key,InputTypeShort)])
                self.log.append(dict(i=self.idx,key=key,ms=t,before=s))
                self.idx+=1
                self.next_ms=t+260
            elif self.idx==len(self.actions) and t>=self.next_ms:
                if s["saved"]==1 and s["stock"]==0 and s["status"]==0 and s["money"]=="002900":
                    self.complete=True;self.final=s
                    return False
                if t>self.next_ms+7000:
                    raise RuntimeError("cure/save incomplete: "+repr(s))
        elif self.checked and s and self.phase==2 and t>=self.next_ms+400:
            self.complete=True;self.final=s
            return False
        return super()._service()
def run(case,seconds):
    error=None;code=None
    try:code=case.run(max_seconds=seconds)
    except Exception as e:error=type(e).__name__+": "+str(e)
    return dict(complete=case.complete,code=code,error=error,
        missing_imports=case.missing_api,elapsed_ms=case.env.now_ms(),
        initial=getattr(case,"initial",None),final=getattr(case,"final",None),
        actions=getattr(case,"log",getattr(case,"post_log",[])),
        matched=getattr(case,"i",None),
        milestones=getattr(case,"samples",[]))
def sd_files():
    return {str(p.relative_to(SD)):dict(size=p.stat().st_size,sha256=hashlib.sha256(p.read_bytes()).hexdigest())
       for p in sorted(SD.rglob("*")) if p.is_file()}
def main():
    assert FAP.is_file()
    ROOT.mkdir(parents=True,exist_ok=True)
    if SD.exists():assert not any(SD.iterdir()),"refuse to use nonempty SD"
    phase0=Replay(406,post_actions=MENU_BAG_USE_SAVE)
    r0=run(phase0,300)
    out=dict(schema="unseeded_arm_oak_parcel_antidote_hourly4",
        base=os.environ.get("POKEMON_ARM_BASE","unknown"),manifest_sha256=os.environ.get("POKEMON_ARM_BASE_SHA",""),
        fap_sha256=hashlib.sha256(FAP.read_bytes()).hexdigest(),
        clean_sd=True,seeded_story=False,seeded_location=False,
        seeded_parcel=False,seeded_stock=False,seeded_money=False,
        poison_status_fixture_after_first_restart=True,
        phase0=r0,phase1=None,phase2=None,physical_flipper=False)
    if r0["complete"] and r0["error"] is None and not r0["missing_imports"]:
        c1=Reopen(1);out["phase1"]=run(c1,40)
        if out["phase1"]["complete"] and out["phase1"]["error"] is None and not out["phase1"]["missing_imports"]:
            c2=Reopen(2);out["phase2"]=run(c2,10)
    out["sd_records"]=sd_files()
    a=out["phase0"].get("milestones",[])
    # Confirm the exact genuine story flags progressed 0 -> got -> delivered,
    # no early paid item, and full quest precedes the paid transaction.
    phases=[(x["i"],x["state"]["parcel"],x["state"]["stock"],x["state"]["money"]) for x in a]
    first_got=next((i for i,p,_,_ in phases if p==1),None)
    first_delivered=next((i for i,p,_,_ in phases if p==3),None)
    first_paid=next((i for i,_,stock,_ in phases if stock==1),None)
    out["milestones"]=dict(parcel_got=first_got,parcel_delivered=first_delivered,paid_antidote=first_paid,
        before_delivery_stock_unchanged=all(stock==0 and money==48 for i,p,stock,money in phases if i<first_delivered),
        after_purchase_money=41)
    p1=out["phase1"];p2=out["phase2"]
    out["success"]=bool(r0["complete"] and r0["matched"]==406 and not r0["error"] and
       first_got is not None and first_delivered is not None and first_paid is not None and
       first_got<first_delivered<first_paid and out["milestones"]["before_delivery_stock_unchanged"] and
       p1 and p1["complete"] and p1["initial"]["stock"]==1 and p1["final"]["stock"]==0 and
       p2 and p2["complete"] and p2["initial"]["stock"]==0 and
       len(out["sd_records"])>=8 and
       not any(x["error"] or x["missing_imports"] for x in (r0,p1,p2)))
    REPORT.write_text(json.dumps(out,indent=2)+"\n")
    compact={k:v for k,v in out.items() if k not in ("phase0","phase1","phase2","sd_records")}
    compact["phase0"]=dict(complete=r0["complete"],matched=r0["matched"],error=r0["error"],elapsed_ms=r0["elapsed_ms"])
    for k in ("phase1","phase2"):
        q=out[k]
        compact[k]=None if q is None else dict(complete=q["complete"],error=q["error"],initial=q["initial"],final=q["final"])
    compact["sd_files"]=list(out["sd_records"])
    print(json.dumps(compact,indent=2))
    sys.exit(0 if out["success"] else 1)
if __name__=="__main__":main()
