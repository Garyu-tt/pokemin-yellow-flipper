#!/usr/bin/env python3
"""Deterministic wire-fixture tests; NOT a substitute for real ARM evidence."""
from pathlib import Path
import importlib.util
import struct
import tempfile
import unittest

p = Path(__file__).resolve().parents[1] / 'tools/arm_smoke_v4.py'
spec = importlib.util.spec_from_file_location('arm_smoke_v4', p)
smoke = importlib.util.module_from_spec(spec)
spec.loader.exec_module(smoke)


def put(b, offset, value):
    struct.pack_into('<I', b, offset, value)


def make_parent(generation=1, version=4, checksum=0x1234ABCD):
    b = bytearray(608)
    for off, value in ((0, smoke.SNAP_MAGIC), (4, version), (8, generation),
                       (12, 600), (16, smoke.PARENT_MAGIC), (144, checksum),
                       (156, checksum), (172, checksum), (188, checksum),
                       (204, checksum), (256, checksum), (272, checksum),
                       (604, smoke.SNAP_SEAL)):
        put(b, off, value)
    struct.pack_into('<H', b, 20, 13)
    put(b, 600, smoke.fnv(b[:600]))
    return b


def make_antidote(generation=1, checksum=0x1234ABCD, stock=3):
    b = bytearray(24)
    for off, value in ((0, 0x59414931), (8, checksum), (12, generation), (20, 0x59414953)):
        put(b, off, value)
    b[4] = 1
    b[6] = stock
    put(b, 16, smoke.fnv(b[:16]))
    return b


def make_pair(generation=1, checksum=0x1234ABCD, stock=3):
    b = bytearray(28)
    for off, value in ((0, 0x31534D59), (12, checksum), (16, generation), (24, 0x53534D59)):
        put(b, off, value)
    b[4] = 1
    b[6:9] = bytes.fromhex('003000')
    b[9] = stock
    put(b, 20, smoke.fnv(b[:20]))
    return b


class TestSmoke(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.folder = self.root / 'apps_data/yellow_native'
        self.folder.mkdir(parents=True)
        self.write_slot('a')

    def write_slot(self, slot, generation=1, version=4):
        (self.folder / f'snapshot_gen_{slot}_v01.dat').write_bytes(make_parent(generation, version))
        (self.folder / f'antidote_{slot}.v1').write_bytes(make_antidote(generation))
        (self.folder / f'money_stock_{slot}.v1').write_bytes(make_pair(generation))

    def change(self, filename, offset, value, rehash=None):
        path = self.folder / filename
        b = bytearray(path.read_bytes())
        b[offset] = value
        if rehash is not None:
            put(b, rehash, smoke.fnv(b[:rehash]))
        path.write_bytes(b)

    def reject(self, reason):
        with self.assertRaisesRegex(ValueError, reason):
            smoke.inspect_sd(self.root)

    def test_valid_v4(self):
        result = smoke.inspect_sd(self.root)
        self.assertTrue(result['verified'])
        self.assertEqual(result['sidecars']['antidote_stock'], 3)
        self.assertEqual(result['selected_slot'], 'a')

    def test_two_slots_choose_newer(self):
        self.write_slot('b', generation=2)
        self.assertEqual(smoke.inspect_sd(self.root)['selected_slot'], 'b')

    def test_equal_generation_prefers_a(self):
        self.write_slot('b')
        self.assertEqual(smoke.inspect_sd(self.root)['selected_slot'], 'a')

    def test_generation_wrap(self):
        self.write_slot('a', generation=0xFFFFFFFF)
        self.write_slot('b', generation=1)
        self.assertEqual(smoke.inspect_sd(self.root)['selected_slot'], 'b')

    def test_invalid_newest_parent_falls_back(self):
        self.write_slot('b', generation=2)
        self.change('snapshot_gen_b_v01.dat', 600, 0)
        self.assertEqual(smoke.inspect_sd(self.root)['selected_slot'], 'a')

    def test_invalid_newest_sidecar_must_not_fall_back(self):
        self.write_slot('b', generation=2)
        self.change('money_stock_b.v1', 20, 0)
        self.reject('money_stock_integrity')

    def test_v1_not_mislabeled_v4(self):
        self.write_slot('a', version=1)
        self.reject('unsupported_atomic_version:1')

    def test_v3_not_mislabeled_v4(self):
        self.write_slot('a', version=3)
        self.reject('unsupported_atomic_version:3')

    def test_missing_parent(self):
        (self.folder / 'snapshot_gen_a_v01.dat').unlink()
        self.reject('no_valid_atomic_parent')

    def test_parent_truncated(self):
        p = self.folder / 'snapshot_gen_a_v01.dat'
        p.write_bytes(p.read_bytes()[:-1])
        self.reject('no_valid_atomic_parent')

    def test_parent_checksum(self):
        self.change('snapshot_gen_a_v01.dat', 48, 99)
        self.reject('no_valid_atomic_parent')

    def test_parent_seal(self):
        self.change('snapshot_gen_a_v01.dat', 604, 0)
        self.reject('no_valid_atomic_parent')

    def test_embedded_binding(self):
        self.change('snapshot_gen_a_v01.dat', 156, 0, rehash=600)
        self.reject('no_valid_atomic_parent')

    def test_missing_antidote(self):
        (self.folder / 'antidote_a.v1').unlink()
        self.reject('missing_or_unsafe:antidote_a')

    def test_missing_wallet(self):
        (self.folder / 'money_stock_a.v1').unlink()
        self.reject('missing_or_unsafe:money_stock_a')

    def test_antidote_truncated(self):
        p = self.folder / 'antidote_a.v1'
        p.write_bytes(p.read_bytes()[:-1])
        self.reject('wrong_length:antidote')

    def test_wallet_extra_bytes(self):
        p = self.folder / 'money_stock_a.v1'
        p.write_bytes(p.read_bytes() + b'\0')
        self.reject('wrong_length:money_stock')

    def test_antidote_checksum(self):
        self.change('antidote_a.v1', 16, 0)
        self.reject('antidote_integrity')

    def test_wallet_checksum(self):
        self.change('money_stock_a.v1', 20, 0)
        self.reject('money_stock_integrity')

    def test_wallet_generation_binding(self):
        self.change('money_stock_a.v1', 16, 2, rehash=20)
        self.reject('money_stock_integrity')

    def test_wallet_parent_binding(self):
        self.change('money_stock_a.v1', 12, 2, rehash=20)
        self.reject('money_stock_integrity')

    def test_antidote_generation_binding(self):
        self.change('antidote_a.v1', 12, 2, rehash=16)
        self.reject('antidote_integrity')

    def test_stock_mismatch(self):
        self.change('money_stock_a.v1', 9, 2, rehash=20)
        self.reject('stock_mismatch')

    def test_invalid_bcd_even_with_valid_checksum(self):
        self.change('money_stock_a.v1', 6, 0xFA, rehash=20)
        self.reject('money_stock_integrity')

    def test_reserved_byte_even_with_valid_checksum(self):
        self.change('money_stock_a.v1', 10, 1, rehash=20)
        self.reject('money_stock_integrity')

    def test_symlink_is_rejected(self):
        p = self.folder / 'money_stock_a.v1'
        p.unlink()
        p.symlink_to('/etc/passwd')
        self.reject('missing_or_unsafe:money_stock')

    def test_root_symlink_is_rejected(self):
        link = self.root / 'linked'
        link.symlink_to(self.root, target_is_directory=True)
        with self.assertRaisesRegex(ValueError, 'unsafe_sd_directory_symlink'):
            smoke.inspect_sd(link)


if __name__ == '__main__':
    unittest.main(verbosity=2)
