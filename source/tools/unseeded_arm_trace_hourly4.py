#!/usr/bin/env python3
"""Replay actual YellowKey events from a fresh C trace against a clean ARM FAP.
Never write to emulated game RAM; all changes are genuine input callbacks.
"""
import argparse,hashlib,json,sys,time,os
from pathlib import Path
SOURCE=Path(__file__).resolve().parents[1]
if "POKEMON_ARM_WORKDIR" not in os.environ: raise SystemExit("POKEMON_ARM_WORKDIR required: isolated evidence directory")
ROOT=Path(os.environ["POKEMON_ARM_WORKDIR"]).resolve()
PROJECT=Path(os.environ.get("POKEMON_BRIDGE","/home/mirbi/projects/mikoshi-chatgpt-bridge"))
sys.path.insert(0,str(PROJECT/"emulation/Flipper-Zero-Emulator"))
from fapemu.runner import Runner
from fapemu.api import InputKeyUp,InputKeyDown,InputKeyLeft,InputKeyRight,InputKeyOk,InputKeyBack,InputTypeShort
KEYS=[InputKeyUp,InputKeyDown,InputKeyLeft,InputKeyRight,InputKeyOk,InputKeyBack]
def parse():
    trace=SOURCE/"tests/unseeded_oak_arm_trace_hourly4.tsv"
    assert hashlib.sha256(trace.read_bytes()).hexdigest()=="1ddc19225bac3fe7e764bf93f260bdcf189d5d9187772225833790f0c2ce6cf2"
    core_test=SOURCE/"tests/test_fresh_oak_parcel_hourly3.c"
    assert hashlib.sha256(core_test.read_bytes()).hexdigest()=="d7c930c7321057f25f3b0075e0df2a55adc241bc775cdc6821fe77478df9988e"
    rows=[]
    for line in (SOURCE/"tests/unseeded_oak_arm_trace_hourly4.tsv").read_text().splitlines():
        a=line.split("\t")
        assert a[0]=="TRACE" and len(a)==17,(len(a),a)
        q=list(map(int,a[1:]))
        rows.append(dict(index=q[0],key=q[1],pre=q[2:7],post=q[7:12],parcel=q[12],stock=q[13],money=q[14],pending=q[15]))
    assert all(row["index"]==i for i,row in enumerate(rows))
    return rows
class Replay(Runner):
    def __init__(self,limit,post_actions=None):
        self.fap_path=Path(os.environ.get("POKEMON_ARM_FAP",str(SOURCE/"dist/yellow_native.fap")))
        self.sd=ROOT/"unseeded-sd"
        self.sd.mkdir(exist_ok=True)
        assert not any(self.sd.iterdir()),"must start with empty SD"
        super().__init__(self.fap_path,sd_root=self.sd,headless=True)
        self.rows=parse()[:limit]
        self.i=0
        self.wait_until=0
        self.waiting=False
        self.sent_at=0
        self.last_progress=0
        self.samples=[]
        self.errors=[]
        self.complete=False
        self.post_actions=post_actions
        self.post_index=0
        self.post_log=[]
    def state(self):
        v=[v for v in self.env.viewports if v.enabled and v.input_ctx]
        if len(v)!=1:return None
        d=self.machine.read(v[0].input_ctx,704)
        return dict(map=d[2],x=int.from_bytes(d[0:1],"little",signed=True),
            y=int.from_bytes(d[1:2],"little",signed=True),screen=d[16],story=d[20],
            parcel=d[692],stock=d[696],money=d[699],pending=d[17],saved=d[19],
            status=d[121],hp=d[4],steps=int.from_bytes(d[6:8],"little"),frames=self.frames)
    def same(self,s,expect):
        return s and [s[k] for k in ("map","x","y","screen","story")]==expect
    def _service(self):
        t=self.env.now_ms()
        s=self.state()
        if s and self.i<len(self.rows):
            row=self.rows[self.i]
            if self.waiting:
                if self.same(s,row["post"]) and s["parcel"]==row["parcel"] and s["stock"]==row["stock"] and s["money"]==row["money"] and t>=self.sent_at+90:
                    self.samples.append(dict(i=self.i,key=row["key"],state=s,ms=t))
                    self.i+=1;self.waiting=False
                    self.wait_until=t+(1800 if row["pre"][3]==7 and row["key"]==4 else 1100 if row["post"][3]==6 and row["pre"][3] not in (6,7) else 320 if row["key"]<4 else 110)
                    self.last_progress=t
                elif t>self.sent_at+5000:
                    raise RuntimeError(f"post mismatch i={self.i} expected={row['post']} actual={s}")
            elif t>=self.wait_until:
                if self.same(s,row["pre"]):
                    self._deliver_input([(KEYS[row["key"]],InputTypeShort)])
                    self.sent_at=t;self.waiting=True
                elif t>self.last_progress+8000:
                    raise RuntimeError(f"pre mismatch i={self.i} expected={row['pre']} actual={s}")
        elif s and self.i>=len(self.rows):
            if self.post_actions is None:
                self.complete=True
                return False
            if self.post_index<len(self.post_actions):
                if t>=self.wait_until:
                    key=self.post_actions[self.post_index]
                    self._deliver_input([(key,InputTypeShort)])
                    self.post_log.append(dict(index=self.post_index,key=key,ms=t,before=s))
                    self.post_index+=1
                    self.wait_until=t+250
            elif t>=self.wait_until and s["saved"]==1 and s["parcel"]==3 and s["stock"]==1 and s["money"]==41:
                self.complete=True
                return False
            elif t>self.wait_until+6000:
                raise RuntimeError(f"post-save failed {s}")
        return super()._service()
def main():
    p=argparse.ArgumentParser();p.add_argument("--limit",type=int,default=30);p.add_argument("--seconds",type=float,default=40);a=p.parse_args()
    ROOT.mkdir(parents=True,exist_ok=True)
    c=Replay(a.limit);code=None;error=None
    try:code=c.run(max_seconds=a.seconds)
    except Exception as e:error=f"{type(e).__name__}: {e}"
    out=dict(complete=c.complete,matched=c.i,total=len(c.rows),code=code,error=error,
        elapsed_ms=c.env.now_ms(),last=c.state(),samples=c.samples,missing_imports=c.missing_api,
        fap_sha256=hashlib.sha256(c.fap_path.read_bytes()).hexdigest(),
        sd_files=sorted(str(p.relative_to(c.sd)) for p in c.sd.rglob("*") if p.is_file()))
    (ROOT/"unseeded-probe.json").write_text(json.dumps(out,indent=2)+"\n")
    print(json.dumps({k:v for k,v in out.items() if k!="samples"},indent=2))
    print("LAST SAMPLES",json.dumps(out["samples"][-8:],indent=2))
    sys.exit(0 if c.complete and c.i==len(c.rows) and not error and not c.missing_api else 1)
if __name__=="__main__":main()
