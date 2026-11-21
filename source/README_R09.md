# Pokémon Yellow Native — R0.9

Private native Pokémon Yellow partial port for Flipper Zero, compiled into an ARM Cortex-M4 `.fap` against Momentum mntm-012 SDK API 87.1. **Not a Game Boy emulator and not the completed original game.**

## Changes from R0.8

- Battle attack turns now choose an opponent move and resolve move priority, effective Speed (including paralysis) and Speed ties before execution. Quick Attack (+1) takes precedence over normal moves regardless of Speed, based on the original `pret/pokeyellow/engine/battle/core.asm`.
- Selected attacking Pokémon acts first if its move priority is higher, otherwise if it is faster; exact ties use the Gen1 `50 percent + 1` threshold with the upstream `percent` macro.
- If the first action ends the battle or forces a party replacement, the second action is cancelled. A fainted Pokémon does not spend PP on a pending, never-executed attack.
- Pidgey/Rattata/Eevee choose from a **simplified** move policy; Lv7+ Rattata may use Quick Attack as the higher-priority enemy move.
- Added base-Speed-dependent critical-hit probability for the currently available moves. The original three-byte-RLC equivalent is emulated for the crit roll, with critical level doubling and stage-ignoring physical damage formula.
- Battle viewport can show `CRIT` without overlapping `PAR`; `last_enemy_move`, `last_player_first` and `last_critical` are debug-friendly transient fields.
- Verified `macros/data.asm` uses `percent = * $ff / 100`: `25 percent` is **63**, not 64; `50 percent + 1` is **128**, not 129. Earlier R0.8 approximations were corrected and re-tested.
- Save version 9 stored in `/ext/apps_data/yellow_native/save_v09.dat`. Prior save files remain intact, without automatic migration.

## Verified host / SDK tests

All PASS:
- test_core, test_negative, test_route04, test_battle05, test_capture06, test_party07, test_effects08
- NEW test_turns09: priority, Speed ties, paralysis Speed, critical chance, damage formula bypassing stat stages, no response after player KO, no PP consumed on cancelled action, Quick Attack overriding higher Speed, battle integration of criticals
- test_original_data.py compares source move/base stats and battle constants with pinned pret/pokeyellow version
- Momentum ARM build/link and APPCHK for API 87.1

## Limitations / safety

- Only 7 original map areas and a reduced Pokémon Yellow intro; three currently controlled species (Pikachu, Pidgey, Rattata). No full 151 species, full Gen1 AI, full move catalog or status set, critical text pacing, priority for unported moves (such as Counter), accurate enemy PP, complete Gen1 damage quirks, music, evolution, PC boxes or full storyline.
- The companion digital Pikachu **HTask1k0** in Noosphere Pika is a separate persistent identity and was not modified.
- No firmware flashing, and no claim of physical Flipper execution, stable FPS, or hardware UI verification. The app binary was verified by ARM SDK compilation and host-side tests only.
- Original Pokémon Yellow data and graphics sourced from pinned https://github.com/pret/pokeyellow (commit in source_manifest.json), under rights belonging to the Pokémon rights holders. Private experimentation only; redistribution needs suitable rights.

## Files and deployment

External FAP ID: `yellow_native`, target `/ext/apps/Games/yellow_native.fap`. This release is staged as a file on a connected laptop; not installed into the device in this step. Persistent project lives in `/home/mirbi/projects/pokemon-yellow-native`.
