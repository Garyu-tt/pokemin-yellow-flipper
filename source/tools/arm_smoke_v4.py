#!/usr/bin/env python3
"""Read-only version-aware ARM SD smoke checker for Yellow's v4 save.

Checks the actual selected two-slot parent and the mandatory v4 Antidote
and paired wallet+stock records. This is a WIRE integrity smoke check, not
an independent proof of game semantics, hardware SD durability or release.
Legacy v1/v3 records are explicitly reported unsupported, never v4 PASS.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import struct
from pathlib import Path

SNAP_LEN = 608
SNAP_MAGIC = 0x41545933
SNAP_SEAL = 0x434D4954
PARENT_MAGIC = 0x59464C50
PARENT_VERSION = 13
SNAP_PAYLOAD = 600
FNV_SEED = 2166136261
FNV_PRIME = 16777619


def u32(b: bytes, offset: int) -> int:
    return struct.unpack_from('<I', b, offset)[0]


def fnv(b: bytes) -> int:
    h = FNV_SEED
    for octet in b:
        h = ((h ^ octet) * FNV_PRIME) & 0xFFFFFFFF
    return h


def read_exact(path: Path, size: int) -> bytes:
    # Never follow an emulated SD symlink, even when the target is a file.
    if path.is_symlink() or path.parent.is_symlink() or not path.is_file():
        raise ValueError(f'missing_or_unsafe:{path.name}')
    data = path.read_bytes()
    if len(data) != size:
        raise ValueError(f'wrong_length:{path.name}:{len(data)}!= {size}')
    return data


def parent(path: Path) -> dict:
    b = read_exact(path, SNAP_LEN)
    magic, version, generation, payload = struct.unpack_from('<IIII', b)
    if magic != SNAP_MAGIC or version not in (1, 3, 4) or not generation or payload != SNAP_PAYLOAD:
        raise ValueError(f'parent_header:{path.name}:v{version}')
    if u32(b, 604) != SNAP_SEAL or u32(b, 600) != fnv(b[:600]):
        raise ValueError(f'parent_checksum_or_seal:{path.name}')
    if u32(b, 16) != PARENT_MAGIC or struct.unpack_from('<H', b, 20)[0] != PARENT_VERSION:
        raise ValueError(f'native_v13_header:{path.name}')
    binding = u32(b, 144)
    if not binding or any(u32(b, offset) != binding for offset in (156, 172, 188, 204, 256, 272)):
        raise ValueError(f'embedded_sidecar_parent_binding:{path.name}')
    return {'version': version, 'generation': generation, 'parent_checksum': binding,
            'sha256': hashlib.sha256(b).hexdigest(), 'path': str(path)}


def check_sidecars(folder: Path, slot: str, selected: dict) -> dict:
    if selected['version'] != 4:
        raise ValueError(f'unsupported_atomic_version:{selected["version"]}:require_v4')
    checksum = selected['parent_checksum']
    generation = selected['generation']
    antidote = read_exact(folder / f'antidote_{slot}.v1', 24)
    if (u32(antidote, 0) != 0x59414931 or antidote[4:6] != b'\x01\x00' or
        antidote[7] != 0 or antidote[6] > 99 or u32(antidote, 8) != checksum or
        u32(antidote, 12) != generation or u32(antidote, 16) != fnv(antidote[:16]) or
        u32(antidote, 20) != 0x59414953):
        raise ValueError(f'antidote_integrity:{slot}')
    pair = read_exact(folder / f'money_stock_{slot}.v1', 28)
    if (u32(pair, 0) != 0x31534D59 or pair[4:6] != b'\x01\x00' or
        pair[10:12] != b'\x00\x00' or pair[9] > 99 or
        any(nibble > 9 for octet in pair[6:9] for nibble in (octet >> 4, octet & 15)) or
        u32(pair, 12) != checksum or u32(pair, 16) != generation or
        u32(pair, 20) != fnv(pair[:20]) or u32(pair, 24) != 0x53534D59):
        raise ValueError(f'money_stock_integrity:{slot}')
    if pair[9] != antidote[6]:
        raise ValueError(f'stock_mismatch:{slot}:antidote={antidote[6]}:pair={pair[9]}')
    return {'antidote_stock': antidote[6],
            'money_bcd_hex': pair[6:9].hex(),
            'antidote_sha256': hashlib.sha256(antidote).hexdigest(),
            'money_stock_sha256': hashlib.sha256(pair).hexdigest()}


def inspect_sd(sd_root: Path) -> dict:
    folder = sd_root / 'apps_data/yellow_native'
    if sd_root.is_symlink() or (sd_root / 'apps_data').is_symlink() or folder.is_symlink():
        raise ValueError('unsafe_sd_directory_symlink')
    slots = {}
    rejected = {}
    for slot in ('a', 'b'):
        path = folder / f'snapshot_gen_{slot}_v01.dat'
        try:
            slots[slot] = parent(path)
        except ValueError as exc:
            rejected[slot] = str(exc)
    if not slots:
        raise ValueError(f'no_valid_atomic_parent:{rejected}')
    # Match yellow_atomic_select: signed 32-bit generation delta; ties favor a.
    winner = 'a' if 'a' in slots else 'b'
    if 'a' in slots and 'b' in slots:
        delta = (slots['b']['generation'] - slots['a']['generation']) & 0xFFFFFFFF
        winner = 'b' if 0 < delta < 0x80000000 else 'a'
    selected = slots[winner]
    # Crucial: never fall back to the older parent when the NEWEST parent's
    # paired wallet/stock is corrupt. The native runtime locks that state.
    details = check_sidecars(folder, winner, selected)
    return {'verified': True, 'save_kind': 'atomic-v4', 'selected_slot': winner,
            'selected_parent': selected, 'sidecars': details,
            'ignored_invalid_parent_slots': rejected}


def check_emulator(report: Path, fap: Path) -> dict:
    if report.is_symlink() or not report.is_file() or fap.is_symlink() or not fap.is_file():
        raise ValueError('missing_or_unsafe_arm_evidence')
    data = json.loads(report.read_text(encoding='utf-8'))
    sha = hashlib.sha256(fap.read_bytes()).hexdigest()
    if data.get('fap_sha256') != sha or data.get('exit_code') != 0:
        raise ValueError('arm_fap_hash_or_exit_mismatch')
    required = ('real_arm_fap_loaded', 'all_firmware_imports_implemented',
                'viewport_draws', 'scripted_buttons_delivered',
                'screen_changes_after_menu', 'screen_changes_after_save',
                'no_emulation_fault', 'no_unsupported_calls')
    if not all(data.get('checks', {}).get(k) is True for k in required):
        raise ValueError('arm_emulator_checks_failed')
    return {'verified': True, 'fap_sha256': sha, 'checks': list(required),
            'legacy_save_file_created_ignored': True}


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--sd-root', type=Path, required=True)
    ap.add_argument('--report', type=Path, help='write JSON outside SD root only')
    ap.add_argument('--emulator-report', type=Path)
    ap.add_argument('--fap', type=Path)
    args = ap.parse_args()
    if bool(args.emulator_report) != bool(args.fap):
        ap.error('--emulator-report and --fap must be provided together')
    if args.emulator_report and args.sd_root.resolve() != (args.emulator_report.resolve().parent / 'sdcard'):
        ap.error('ARM report and SD must come from the SAME probe run')
    if args.report and (args.report.resolve() == args.sd_root.resolve() or
                        args.report.resolve().is_relative_to(args.sd_root.resolve())):
        ap.error('report cannot be written to emulated SD')
    try:
        result = inspect_sd(args.sd_root)
        if args.emulator_report:
            result['arm_emulator'] = check_emulator(args.emulator_report, args.fap)
        result['smoke_verified'] = bool(args.emulator_report)
        status = 0
    except (ValueError, OSError, json.JSONDecodeError) as exc:
        result = {'verified': False, 'smoke_verified': False, 'error': str(exc)}
        status = 2
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps(result, indent=2, sort_keys=True) + '\n', encoding='utf-8')
    print(json.dumps(result, sort_keys=True))
    return status


if __name__ == '__main__':
    raise SystemExit(main())
