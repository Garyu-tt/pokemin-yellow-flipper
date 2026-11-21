#!/usr/bin/env python3
"""Read-only downgrade risk preflight for Pokémon Yellow R08 -> v064.

Does not convert or alter a save. This tool prevents accidentally treating
atomic-v3 snapshots as backwards-compatible atomic-v1 records. Reads a copied
SD image directory and uses source-pinned R08 on-disk offsets.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import struct
from pathlib import Path

MAGIC=0x41545933       # YELLOW_ATOMIC_MAGIC
SEAL=0x434d4954        # YELLOW_ATOMIC_SEAL
ITEM_MAGIC=0x59414931  # antidote YAI1
ITEM_SEAL=0x59414953   # antidote YAIS
SIZE=608
PAYLOAD=600
PARENT_CSUM_OFFSET=144  # measured from native C offsetof()
HDR="<IIII"

def fnv(data:bytes):
    value=2166136261
    for byte in data:
        value=((value^byte)*16777619)&0xffffffff
    return value

def parse_snapshot(path:Path):
    if not path.exists():
        return dict(exists=False,valid=False,path=path.name,reason="missing")
    raw=path.read_bytes()
    if len(raw)!=SIZE:
        return dict(exists=True,valid=False,path=path.name,reason="truncated_or_oversized",bytes=len(raw))
    magic,version,generation,payload=struct.unpack_from(HDR,raw,0)
    checksum,seal=struct.unpack_from("<II",raw,PAYLOAD)
    valid=(magic==MAGIC and version in (1,3) and 0<generation<2**32
           and payload==PAYLOAD and seal==SEAL and fnv(raw[:PAYLOAD])==checksum)
    return dict(exists=True,valid=valid,path=path.name,version=version,
                generation=generation,parent_checksum=struct.unpack_from("<I",raw,PARENT_CSUM_OFFSET)[0],
                sha256=hashlib.sha256(raw).hexdigest(),
                reason="valid" if valid else "invalid_header_checksum_or_seal")

def check_sidecar(path:Path,snapshot:dict):
    if not path.exists():return dict(exists=False,valid=False,reason="missing")
    raw=path.read_bytes()
    valid=(len(raw)==24 and struct.unpack_from("<I",raw,0)[0]==ITEM_MAGIC
           and raw[4]==1 and raw[5]==0 and raw[6]<=99 and raw[7]==0
           and struct.unpack_from("<I",raw,8)[0]==snapshot["parent_checksum"]
           and struct.unpack_from("<I",raw,12)[0]==snapshot["generation"]
           and struct.unpack_from("<I",raw,16)[0]==fnv(raw[:16])
           and struct.unpack_from("<I",raw,20)[0]==ITEM_SEAL)
    return dict(exists=True,valid=valid,stock=raw[6] if len(raw)>6 else None,
                sha256=hashlib.sha256(raw).hexdigest(),
                reason="valid" if valid else "corrupt_or_wrong_generation")

def newer(a,b):
    delta=(a-b)&0xffffffff
    return 0<delta<0x80000000

def choose(slots):
    candidates=[(key,rec) for key,rec in slots.items() if rec["valid"]]
    if not candidates:return None
    win=candidates[0]
    for challenger in candidates[1:]:
        if newer(challenger[1]["generation"],win[1]["generation"]):win=challenger
    return win[0]

def audit(folder:Path):
    slots={name:parse_snapshot(folder/("snapshot_gen_"+name+"_v01.dat")) for name in "ab"}
    for name in "ab":
        rec=slots[name]
        if rec["valid"] and rec["version"]==3:
            rec["antidote_sidecar"]=check_sidecar(folder/("antidote_"+name+".v1"),rec)
    modern=choose(slots)
    legacy=choose({k:{**v,"valid":v["valid"] and v.get("version")==1} for k,v in slots.items()})
    codes=[]
    if modern is None:codes.append("NO_VALID_ATOMIC_PARENT")
    else:
        win=slots[modern]
        if win["version"]==3:
            codes.append("V3_UNSUPPORTED_BY_V064")
            if not win["antidote_sidecar"]["valid"]:
                codes.append("MISSING_OR_CORRUPT_REQUIRED_V3_ANTIDOTE")
        if legacy is None: codes.append("V064_HAS_NO_READABLE_ATOMIC_PARENT")
        elif legacy!=modern:
            codes.append("V064_WOULD_LOAD_OLDER_SLOT")
        for name,rec in slots.items():
            if rec["valid"] and rec.get("version")==3 and not rec["antidote_sidecar"]["valid"]:
                codes.append("INVALID_V3_SIDECAR_"+name)
    # A corrupt v3 envelope still proves newer-format state existed.
    # Do not bless an older v1 slot when a damaged v3 slot is present.
    if any(row.get("version")==3 for row in slots.values()):
        codes.append("V3_FORMAT_PRESENT_ANYWHERE")
    compatible=(modern is not None and legacy==modern and
                slots[modern]["version"]==1 and
                not any(row.get("version")==3 for row in slots.values()))
    return {
        "schema":"pokemon.yellow.downgrade-preflight/v1",
        "folder":str(folder),"slots":slots,
        "current_reader_selects":modern,"v064_reader_selects":legacy,
        "legacy_atomic_envelope_compatible":compatible,
        "downgrade_approved":False,
        "block_downgrade":not compatible,
        "issues":sorted(set(codes)),
        "note":"Compatible envelope != safe full-game downgrade; never write old saves without an audited export."
    }

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("sd_folder",type=Path)
    ap.add_argument("--json",action="store_true")
    args=ap.parse_args()
    folder=args.sd_folder
    if not folder.is_dir():ap.error("SD folder does not exist")
    result=audit(folder)
    print(json.dumps(result,ensure_ascii=False,indent=2 if args.json else None))
    return 2 if result["block_downgrade"] else 0
if __name__=="__main__":
    raise SystemExit(main())
