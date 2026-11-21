# Pokémon Yellow Native for Flipper Zero — R0.5

**Private early native port. Not a complete Pokémon Yellow game, and no Game Boy CPU emulation.**

## What was added over R0.4

- Dedicated C battle module `yellow_battle.c/h` with real source-derived Gen1 base stats for Pikachu, Eevee, Pidgey and Rattata (HP, Attack, Defense, Speed, Special, exp yield and growth category).
- Fixed DV=15 and Stat Exp=0 for initial stat formulas; medium-fast and medium-slow experience growth.
- Key original move properties (type, power, accuracy, PP) for ThunderShock, Growl, Tail Whip, Tackle, Gust and Quick Attack, plus unused Sand Attack metadata.
- Basic Gen1-like damage calculation factoring Attack/Defense or Special, stage modifiers, STAB, Electric-vs-Flying matchup, accuracy and 217–255 random damage multiplier. **Not fully Gen1-accurate** (see limitations).
- Actual PP depletion and restoration when talking to nurse. Empty PP for a move prevents using that move.
- Experience awards after Pokémon defeat. Trainer Eevee uses an approximated trainer multiplier. Pika starts at Lv5, gains levels; learns Tail Whip at Lv6.
- New 128×64-native battle move list and PP, Party shows current level, HP, EXP.
- Versioned new save `save_v05.dat`: EXP, PP and level persist and validate on restore. R0.4 saves are not overwritten.
- Upstream table checks in `tests/test_original_data.py` against pinned pret/pokeyellow disassembly.

## Passed verifications

- Host scenario test: Oak encounter, initial rival battle, companion, saves.
- Host negative tests: forced rival loss, escape lock, corrupted save rejection.
- Host route tests: Pallet ↔ Route 1 ↔ Viridian City, wild spawn table, Center healing.
- R0.5 battle tests: real base data, type matchup/STAB, attack stages, PP, EXP, level-up, Lv6 Tail Whip, heal, save/restore.
- Momentum mntm-012 firmware SDK `API 87.1`: GCC ARM compile/link and APPCHK PASS.

## Limitations / important honesty

The following are **not** implemented or fully faithful: original battle state machine, type immunities beyond included matchup, criticals, stat experience, variable DVs, status effects, four full move slots, experience participation distribution, evolution, other 147 Pokémon, catching and party management, inventory, full maps/scripts/audio, full Original Game Boy save conversion. Wild battles and intro remain simplified. Execution on real Flipper and FPS are **not yet verified**.

## Build and safety

Sources in this archive. App ID `yellow_native`, external Momentum FAP (not firmware flash). Uses only its own paths, default path `/ext/apps/Games/yellow_native.fap`, storage `/ext/apps_data/yellow_native/save_v05.dat`. Existing `save_v03.dat`, `save_v04.dat`, and unrelated Noosphere Pika are left untouched. Digital entity HTask1k0 has a separate immutable identity; it is not the in-game Pokémon.

Source-derived data: https://github.com/pret/pokeyellow, fixed upstream commit in `source_manifest.json`.
Pokémon names, art and game data remain copyrighted by their rights holders. Private research/testing; no unlicensed public redistribution.
