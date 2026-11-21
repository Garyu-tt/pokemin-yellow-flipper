# Pokémon Yellow Native — R0.6 for Flipper Zero

Status: **partial native game port**, ARM Cortex-M4 FAP, no Game Boy instruction emulation. Not the complete original game.

## New in this release

- Reimplemented the original Gen1 *basic Poké Ball* two-random-number capture test and integer HP/catch-rate threshold from `pret/pokeyellow/engine/items/item_effects.asm`, for wild Route 1 Pidgey and Rattata. No status effects, other balls, or box storage yet.
- Initial 5 Poké Balls granted only as a **temporary development starter kit** after the introductory rival battle, because Oak's original parcel/Poké Ball quest is not yet ported.
- Party capacity six: Pikachu (first, fixed) plus up to five captured Pidgey/Rattata. Full party blocks additional captures without consuming Poké Balls; trainer Pokémon cannot be captured.
- Original low-level Pidgey moves (Gust, Sand Attack from Lv5) and Rattata moves (Tackle, Tail Whip, Quick Attack from Lv7) stored for caught Pokémon, up to four move slots.
- Party display scrolls by three entries; each Pokémon shows level, current/max HP. Bag shows Poké Ball count. Fight menu scrolls four rows; options are learned Pikachu moves, Ball and Run. Pikachu still cannot be swapped for another active Pokémon in battle.
- Nurse heals all party HP/PP, including newly caught Pokémon.
- New separate versioned save `/ext/apps_data/yellow_native/save_v06.dat` retains entire caught party, balls and PP. R0.5 and older saves are left untouched; migration is **not implemented**.
- No firmware flash, no modification of unrelated Noosphere Pika app or HTask1k0 immutable digital identity.

## Tests

- `test_core.c` / `test_negative.c`: R0.3 dialogue, battle and save regressions (PASS).
- `test_route04.c`: 7-map Route 1/Viridian exploration and Center regressions (PASS).
- `test_battle05.c`: calculated stats, base move data, PP and EXP progression (PASS).
- `test_capture06.c`: Gen1 capture boundary cases, wild capture, 6-slot capacity, trainer capture blocked, full-party protection, bag/party navigation, nurse party healing, R0.6 save validation (PASS).
- `test_original_data.py`: checks specific original base stats, move metadata and learnsets (PASS).
- Momentum mntm-012 SDK API 87.1 native ARM compilation, linking and APPCHK (PASS).

## Incomplete systems

This is **not full Pokémon Yellow**. Only 7 original map areas and opening scripts are adapted. Other 149 species, PC/box transfers, capture status effects, party switching in combat, full battle move system/AI, status ailments, critical hits, item economy, Pokémon evolution, complete dialogue, inventory, music and more are pending. Gameplay is not yet validated on a physical Flipper; FPS and input behavior on device remain unknown. Four move *storage* slots exist, but only the moves currently learned by Pikachu are usable.

## Assets and rights

Original maps and sprites are from the pinned https://github.com/pret/pokeyellow disassembly, SHA commit in source_manifest.json. Game and visual rights remain with their owners; this is a private development/testing project, not a license for public redistribution.
