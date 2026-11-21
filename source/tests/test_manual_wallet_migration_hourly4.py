#!/usr/bin/env python3
"""Independent host-only v1/v3 wallet migration fixtures and fail-closed tests."""
from pathlib import Path
import hashlib
import os
import re
import shutil
import struct
import subprocess
import tempfile

S = Path(__file__).resolve().parents[1]
ROOT = Path("/home/mirbi/projects/mikoshi-chatgpt-bridge/toolchain/root/usr")
TCC = ROOT / "bin/tcc"
DEPS = [
    "yellow_assets.c", "yellow_battle.c", "yellow_core.c",
    "yellow_pewter_progress.c", "yellow_antidote_inventory_slot.c",
    "yellow_viridian_mart_purchase.c", "yellow_mart_runtime.c",
    "yellow_antidote_cure.c", "yellow_antidote_game.c",
]
def compile_host(out, entry, extra=()):
    cmd = [
        str(TCC), "-nostdlib",
        "-B" + str(ROOT / "lib/x86_64-linux-gnu/tcc"),
        "-I" + str(ROOT / "include"),
        "-I" + str(ROOT / "include/x86_64-linux-gnu"),
        "-I" + str(S),
        "-L" + str(ROOT / "lib/x86_64-linux-gnu"),
        "-L/lib/x86_64-linux-gnu",
        str(ROOT / "lib/x86_64-linux-gnu/crt1.o"),
        str(ROOT / "lib/x86_64-linux-gnu/crti.o"),
        str(entry), *(str(S / d) for d in DEPS),
        *(str(S / d) for d in extra),
        str(ROOT / "lib/x86_64-linux-gnu/crtn.o"), "-lc", "-o", str(out),
    ]
    subprocess.run(cmd, check=True, capture_output=True, text=True, timeout=12)

def run(cmd, ok=True):
    r = subprocess.run([str(x) for x in cmd], capture_output=True, text=True, timeout=8)
    if ok:
        assert r.returncode == 0, (cmd, r.stdout, r.stderr)
    else:
        assert r.returncode != 0 and "MIGRATION_REFUSED" in r.stderr, (cmd, r.stdout, r.stderr)
    return r

def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def fnv(b):
    h = 2166136261
    for v in b:
        h = ((h ^ v) * 16777619) & 0xffffffff
    return h

def verify_pair(path, balance, stock, generation=41):
    b = path.read_bytes()
    assert len(b) == 28
    assert struct.unpack_from("<I", b)[0] == 0x31534d59
    assert b[4] == 1 and b[5] == 0 and b[9] == stock
    assert b[10:12] == bytes(2)
    assert struct.unpack_from("<I", b, 16)[0] == generation
    assert struct.unpack_from("<I", b, 20)[0] == fnv(b[:20])
    assert struct.unpack_from("<I", b, 24)[0] == 0x53534d59
    pairs = [balance // 10000, balance // 100 % 100, balance % 100]
    assert b[6:9] == bytes((v // 10 << 4) | (v % 10) for v in pairs)
    return struct.unpack_from("<I", b, 12)[0]

with tempfile.TemporaryDirectory(prefix="wallet-v1-v3-offline-") as td:
    root = Path(td)
    tool = root / "manual_money_migration"
    fixture = root / "fixture"
    compile_host(tool, S / "tests/manual_money_migration_hourly4.c", ("yellow_money_stock_slot.c",))
    compile_host(fixture, S / "tests/wallet_migration_fixture_hourly4.c")

    def make(label, version, item, slot=0):
        folder = root / label
        folder.mkdir()
        run([fixture, folder, version, int(item), slot])
        letter = "b" if slot else "a"
        parent = folder / f"snapshot_gen_{letter}_v01.dat"
        return folder, parent, letter

    def migrate(parent, label, balance=1275, stock=None, ack=True, ok=True):
        out = root / label
        cmd = [tool, "--snapshot", parent, "--balance", str(balance),
               "--out-dir", out]
        if stock is not None:
            cmd += ["--stock", str(stock)]
        if ack:
            cmd.append("--ack-unknown-history")
        result = run(cmd, ok=ok)
        if not ok:
            assert not out.exists(), f"failed migration left partial output: {out}"
        return out, result

    v3, v3parent, _ = make("v3-good", 3, True)
    before = {p.name: digest(p) for p in v3.iterdir()}
    out, result = migrate(v3parent, "v3-destination")
    assert "OFFLINE_MIGRATION_PREPARED_PASS" in result.stdout
    parent_checksum = verify_pair(out / "money_stock_a.v1", 1275, 2)
    notes = (out / "MANUAL_INSTALL_ONLY.txt").read_text()
    assert f"Parent checksum: {parent_checksum:08x}" in notes
    assert "NOT derivable from snapshot" in notes
    assert not (out / "antidote_a.v1").exists()
    assert before == {p.name: digest(p) for p in v3.iterdir()}
    migrate(v3parent, "v3-ack-missing", ack=False, ok=False)
    migrate(v3parent, "v3-invalid-balance", balance=1000000, ok=False)
    migrate(v3parent, "v3-negative-balance", balance=-1, ok=False)
    migrate(v3parent, "v3-stock-override", stock=2, ok=False)
    run([tool, "--snapshot", v3parent, "--balance", "1275",
         "--ack-unknown-history", "--out-dir", out], ok=False)

    v1, v1parent, _ = make("v1-no-stock", 1, False, 1)
    v1hash = digest(v1parent)
    migrate(v1parent, "v1-stock-unspecified", ok=False)
    v1out, _ = migrate(v1parent, "v1-explicit", balance=0, stock=0)
    v1checksum = verify_pair(v1out / "money_stock_b.v1", 0, 0)
    item = (v1out / "antidote_b.v1").read_bytes()
    assert len(item) == 24 and item[6] == 0
    assert struct.unpack_from("<I", item, 8)[0] == v1checksum
    assert struct.unpack_from("<I", item, 12)[0] == 41
    assert struct.unpack_from("<I", item, 16)[0] == fnv(item[:16])
    assert struct.unpack_from("<I", item, 20)[0] == 0x59414953
    assert digest(v1parent) == v1hash
    v1stock, v1stockparent, _ = make("v1-existing-stock", 1, True)
    v1stockout, _ = migrate(v1stockparent, "v1-stock-valid", balance=999999)
    verify_pair(v1stockout / "money_stock_a.v1", 999999, 2)
    assert not (v1stockout / "antidote_a.v1").exists()

    v3empty, v3emptyparent, _ = make("v3-missing-stock", 3, False)
    migrate(v3emptyparent, "v3-no-stock", stock=0, ok=False)
    broken, brokenparent, _ = make("v3-bad-parent", 3, True)
    raw = bytearray(brokenparent.read_bytes()); raw[19] ^= 1
    brokenparent.write_bytes(raw)
    migrate(brokenparent, "bad-parent-refused", ok=False)
    truncated, truncatedparent, _ = make("v3-short-parent", 3, True)
    truncatedparent.write_bytes(truncatedparent.read_bytes()[:-1])
    migrate(truncatedparent, "short-parent-refused", ok=False)
    v4, v4parent, _ = make("v4-refused", 4, True)
    migrate(v4parent, "v4-not-legacy", ok=False)

    mismatch, mismatchparent, _ = make("v3-bad-stock", 3, True)
    stockpath = mismatch / "antidote_a.v1"
    stockraw = bytearray(stockpath.read_bytes()); stockraw[6] ^= 1
    stockpath.write_bytes(stockraw)
    migrate(mismatchparent, "bad-stock-refused", ok=False)
    existing, existingparent, _ = make("v3-existing-money", 3, True)
    (existing / "money_stock_a.v1").write_bytes(bytes(28))
    migrate(existingparent, "existing-money-refused", ok=False)
    symlink, symlinkparent, _ = make("v3-symlink", 3, True)
    symlinkparent.unlink()
    symlinkparent.symlink_to(v3parent)
    migrate(symlinkparent, "symlink-refused", ok=False)

    # Independently pin source-level fail-closed runtime guards: old v1/v3
    # have no stored balance, and all three money actions check integrity.
    fl = (S / "yellow_flipper.c").read_text()
    assert "v1 AND v3 lack historical wallet data" in fl
    assert "g->money_integrity_error=true;" in fl
    assert "g->antidote_integrity_error=true;" in fl
    assert "g->money_integrity_error)return false;" in fl
    assert "a->g.antidote_integrity_error || a->g.money_integrity_error" in fl
    mart = (S / "yellow_mart_runtime.c").read_text()
    assert "g->money_integrity_error || g->antidote_integrity_error" in mart

print("OFFLINE_LEGACY_V1_V3_MANUAL_WALLET_MIGRATION_NEGATIVES_AND_READONLY_PASS")
