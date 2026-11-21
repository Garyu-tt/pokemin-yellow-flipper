#!/usr/bin/env python3
"""Mikoshi Flipper EmuLab: deterministic ARM FAP visual/button smoke test.

Actual .fap runs inside fapemu (Unicorn Cortex-M4), no physical Flipper.
Actuator: genuine Flipper input callbacks. Sensors: original 128x64 Canvas,
SHA256 and pixel differences, PNG frames, and reproducible JSON evidence.
No firmware flash, no outbound network, no access to production save data.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import os
import sys
import time
from datetime import datetime, timezone
from pathlib import Path
from PIL import Image, ImageChops, ImageStat
from vision import classify

BASE=Path("/home/mirbi/projects/mikoshi-flipper-emulab")
UPSTREAM=BASE/"upstream"
if str(UPSTREAM) not in sys.path:sys.path.insert(0,str(UPSTREAM))
os.environ.setdefault("SDL_VIDEODRIVER","dummy")
os.environ.setdefault("SDL_AUDIODRIVER","dummy")

from fapemu.api import InputTypeLong
from fapemu.runner import Runner
from fapemu.env import Env
from fapemu.api import api
from fapemu.api.core import HostFile

# Safe app-local SD mapping: a FAP may only read/write its disposable SD tree.
_ORIGINAL_HOST_PATH=Env.host_path
def _confined_sd_path(env,flipper_path):
    candidate=_ORIGINAL_HOST_PATH(env,flipper_path).resolve()
    root=env.sd_root.resolve()
    if candidate!=root and root not in candidate.parents:
        raise PermissionError("FAP path escaped isolated emulator SD card")
    return candidate
Env.host_path=_confined_sd_path

@api("storage_common_mkdir",2)
def _storage_mkdir(env,storage,path_ptr):
    try:
        env.host_path(env.machine.cstring(path_ptr)).mkdir(parents=True,exist_ok=True)
        return 0   # FS_Error: FSE_OK
    except OSError:return 5

@api("storage_file_is_open",1)
def _storage_is_open(env,handle):
    f=env.machine.handle(handle)
    return 1 if isinstance(f,HostFile) and f.fh is not None and not f.fh.closed else 0

from fapemu.api import (InputKeyUp,InputKeyDown,InputKeyRight,InputKeyLeft,
                        InputKeyOk,InputKeyBack,InputTypePress,InputTypeShort,
                        InputTypeRelease)
import pygame

KEYS={"HOLD":InputKeyOk,"UP":InputKeyUp,"DOWN":InputKeyDown,"RIGHT":InputKeyRight,
      "LEFT":InputKeyLeft,"OK":InputKeyOk,"BACK":InputKeyBack}
VISUAL_POLICY={"intro_pallet":"OK","map_art":"OK",
               "controls_dialog":"BACK","overworld":"RIGHT"}

def fingerprint(image):
    bw=image.convert("L")
    small=bw.resize((8,8),Image.Resampling.BILINEAR)
    vals=list(small.get_flattened_data());avg=sum(vals)/len(vals)
    phash="".join("1" if x>=avg else "0" for x in vals)
    hist=bw.histogram()
    return {"sha256":hashlib.sha256(bw.tobytes()).hexdigest(),
            "perceptual_hash":f"{int(phash,2):016x}",
            "active_pixels":sum(n for k,n in enumerate(hist) if k<128),
            "width":bw.width,"height":bw.height}

def diff_image(a,b):
    if a is None:return {"changed_pixels":None,"fraction":None,"bbox":None}
    ia=a.convert("L");ib=b.convert("L")
    diff=ImageChops.difference(ia,ib)
    # 1 pixel different can be antialiasing: treat >20 intensity as change.
    mask=diff.point(lambda x:255 if x>20 else 0)
    n=sum(1 for val in mask.get_flattened_data() if val>0)
    return {"changed_pixels":n,"fraction":round(n/(ia.width*ia.height),5),
            "bbox":list(mask.getbbox()) if mask.getbbox() else None}

class VisualRunner(Runner):
    def __init__(self, fap, artifact_dir, actions, seconds=16, autoplay=False):
        super().__init__(fap,sd_root=artifact_dir/"sdcard",headless=True,verbose=True)
        self.artifact_dir=artifact_dir
        self.actions=actions
        self.autoplay=autoplay
        self.seconds=seconds
        self.snapshots=[]
        self.inputs=[]
        self.last_image=None
        self._next=0
        self._last_action_frame=0
        self._first_frame=False
        self._last_screen_change_frame=0
        self._last_hash=None

    def _save_frame(self,label):
        surface=self.env.canvas.surface
        rgb=pygame.image.tobytes(surface,"RGB")
        img=Image.frombytes("RGB",surface.get_size(),rgb).convert("L")
        safe=label.replace("/","_")
        path=self.artifact_dir/"frames"/(f"{len(self.snapshots):03d}_{safe}.png")
        path.parent.mkdir(exist_ok=True,parents=True)
        img.save(path,optimize=True)
        fp=fingerprint(img);diff=diff_image(self.last_image,img)
        record={"frame":self.frames,"label":label,"file":str(path.relative_to(self.artifact_dir)),
                "fingerprint":fp,"diff_from_previous":diff,"scene":classify(img)}
        self.snapshots.append(record);self.last_image=img
        if self._last_hash!=fp["sha256"]:
            self._last_hash=fp["sha256"];self._last_screen_change_frame=self.frames
        return record

    def _service(self):
        keep=super()._service()
        if not keep:return keep
        if not self._first_frame and self.frames>=5:
            self._save_frame("boot")
            self._first_frame=True
        if self._first_frame and self._next<len(self.actions):
            act=self.actions[self._next]
            # At least 20 frames after prior input; enough for screen to settle.
            target=20+self._next*31
            if self.frames>=target:
                observed=classify(self.last_image) if self.last_image is not None else {"scene":"unknown"}
                key=(VISUAL_POLICY.get(observed["scene"]) if self.autoplay else act["key"])
                if key is None:
                    self.inputs.append({"key":None,"blocked":True,"reason":"unknown_scene",
                                        "scene":observed["scene"],"frame":self.frames})
                    self._next+=1
                    return keep
                self.actions[self._next]["key"]=key
                previous=self.snapshots[-1] if self.snapshots else None
                kind=InputTypeLong if key=="HOLD" else InputTypeShort
                self._deliver_input([(KEYS[key],InputTypePress),
                                     (KEYS[key],kind),
                                     (KEYS[key],InputTypeRelease)])
                self.inputs.append({"key":key,"frame":self.frames,
                                    "vision_decision":observed if self.autoplay else None,
                                    "action_index":self._next,
                                    "previous_screen":previous["fingerprint"]["sha256"] if previous else None})
                self._last_action_frame=self.frames
                self._next+=1
        if self._last_action_frame and self.frames in [20+ix*31+16 for ix in range(len(self.actions))]:
            self._save_frame("after_"+self.actions[self._next-1]["key"].lower())
        if self.frames==5+len(self.actions)*31+18:
            self._save_frame("final")
        return keep

    def execute(self):
        start=time.monotonic()
        error=None;code=None
        try:
            code=self.run(max_seconds=self.seconds)
        except Exception as e:
            error={"class":type(e).__name__,"message":str(e)[:700]}
        finally:
            if self.display:
                try:self._save_frame("last")
                except Exception as e:error=error or {"class":"screenshot_error","message":str(e)[:500]}
                self.display.quit()
        changes=[x["diff_from_previous"]["changed_pixels"] for x in self.snapshots[1:]]
        distinct=len({x["fingerprint"]["sha256"] for x in self.snapshots})
        result={"tool":"mikoshi-flipper-emulab","engine":"fapemu:Unicorn-Cortex-M4",
                "real_arm_execution":True,"passed":error is None and
                 len(self.inputs)==len(self.actions) and not any(x.get("blocked") for x in self.inputs)
                 and distinct>=2 and self.frames>=6,
                "no_device_used":True,"semantic_vision_verified":False,
                "sdk_appchk_verified":False,"missing_api":self.missing_api,
                "unsupported_calls":self.env.unsupported,
                "frames":self.frames,"exit_code":code,"error":error,
                "elapsed_s":round(time.monotonic()-start,2),
                "fap":str(self.path),"fap_sha256":hashlib.sha256(self.path.read_bytes()).hexdigest(),
                "actions":self.inputs,"snapshots":self.snapshots,
                "distinct_frames":distinct,
                "recognized_scenes":sorted({x["scene"]["scene"] for x in self.snapshots if x["scene"]["scene"]!="unknown"}),
                "vision_mode":"deterministic_template_plus_pixel_diff_no_vlm",
                "visual_feedback_drives_actions":self.autoplay,
                "screen_changed":any((i or 0)>0 for i in changes),
                "screen_checks_passed":sum((i or 0)>0 for i in changes),
                "created_at":datetime.now(timezone.utc).isoformat()}
        path=self.artifact_dir/"report.json"
        path.write_text(json.dumps(result,ensure_ascii=False,indent=2)+"\n")
        return result

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument("--fap",required=True,type=Path)
    parser.add_argument("--out",required=True,type=Path)
    parser.add_argument("--seconds",type=float,default=16)
    parser.add_argument("--keys",default="OK,DOWN,OK,BACK")
    parser.add_argument("--autoplay",action="store_true",help="select buttons from classified screenshots")
    args=parser.parse_args()
    actions=([{"key":None} for _ in range(4)] if args.autoplay else
             [{"key":x.strip().upper()} for x in args.keys.split(",") if x.strip()])
    if len(actions)>15 or not all(x["key"] in KEYS or (args.autoplay and x["key"] is None) for x in actions):
        parser.error("keys must be UP DOWN LEFT RIGHT OK BACK HOLD (max 15)")
    if not args.fap.is_file():parser.error("FAP missing")
    args.out.mkdir(parents=True,exist_ok=True)
    emu=VisualRunner(args.fap,args.out,actions,seconds=min(args.seconds,120),autoplay=args.autoplay)
    report=emu.execute()
    print(json.dumps({k:v for k,v in report.items() if k!="snapshots"},ensure_ascii=False,indent=2))
    print("REPORT",args.out/"report.json")
    return 0 if report["passed"] else 2

if __name__=="__main__":raise SystemExit(main())
