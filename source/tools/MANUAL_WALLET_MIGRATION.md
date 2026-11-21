# Legacy v1/v3 atomic wallet — OFFLINE manual recovery only

This is **not an automatic migration**, release, or instruction to modify a live SD card. Legacy atomic v1 and v3 parents do **not** serialize historical money. The 3000 coins created by `yellow_init` are **not evidence of the old balance**. The Flipper runtime must refuse purchases, Antidote consumption and resaving until the matching money+stock pair is supplied.

## Safe workflow

1. Copy the **entire** `/ext/apps_data/yellow_native/` directory from the SD to a trusted offline host. Keep a read-only backup and record SHA-256 of every file. Do not run this utility against the mounted Flipper SD.
2. Independently establish the historical balance (0–999999) from a trusted external record or explicitly choose a manual adjustment. No amount can be inferred from v1/v3 bytes. The CLI requires `--ack-unknown-history` and an explicit `--balance`; this is **acknowledgement, not proof**.
3. Build the host-only C source `tests/manual_money_migration_hourly4.c` against the project's actual `yellow_atomic_snapshot.c`, `yellow_antidote_inventory_slot.c`, `yellow_money_stock_slot.c` and their game dependencies. For a reproducible build and 16+ positive/negative fixture scenarios on `mirbi-linux`, run `python3 tests/test_manual_wallet_migration_hourly4.py`. The test builds the utility in a temporary directory using the local TinyCC toolchain. It never accesses the real SD.
4. Example **offline** invocation of the compiled utility (replace with a path to your isolated host copy):
   `manual_money_migration --snapshot /offline/save-copy/snapshot_gen_a_v01.dat --balance 1275 --ack-unknown-history --out-dir /offline/staged-money-a`
   For a v1 snapshot **without** an Antidote sidecar, also provide an explicitly confirmed `--stock 0` (or other 0–99 count). For v3 a valid matching Antidote sidecar is **mandatory**; its count is used and cannot be overridden.
5. The tool validates the complete sealed parent with the real game codec, the optional exact-size Antidote sidecar, its parent checksum and generation. It refuses v4, corrupt/truncated parents, existing wallet sidecars (even corrupt), inconsistent stock, and an existing output directory. It writes only to a brand-new offline output directory **outside** the input save tree and reads back the resulting bytes. The original parent is never rewritten or version-bumped. Output names are slot-specific `money_stock_a.v1` / `money_stock_b.v1`, and, only for v1 without stock sidecar, `antidote_a.v1` / `antidote_b.v1`.
6. **Stop before copying to SD.** Have an independent reviewer verify original hashes, the exact selected slot, generation, parent checksum, supplied balance/stock, rollback and all other sidecars. A second slot or a newer generation can supersede this snapshot. Physical SD/FAT power-cut durability and real Flipper execution are unverified.

The tool deliberately does not install files, modify snapshots, promote a development HEAD, or issue a release. Successful fixture tests are not evidence of historical account balance.
