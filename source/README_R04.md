# Pokémon Yellow Native for Flipper Zero — R0.4

**Status: partial native port / early playable prototype.** Not a Game Boy emulator, not the complete Pokémon Yellow game.

## Ported in this revision

- The original Route 1 (10×18 map blocks) and Viridian City (20×18 map blocks) plus Viridian Pokémon Center (7×4 blocks) from `pret/pokeyellow`.
- North/south map connections in both directions, using the original Viridian-to-Route 1 horizontal offset.
- Original Gen1 bottom-left collision tile rule (corrects the initial R0.2/R0.3 approximation).
- Original Route 1 encounter rate 25/256 for grass steps and 10 weighted monster slots (weight distribution: 51,51,39,25,25,25,13,13,11,3 out of 256). Pokémon: Pidgey and Rattata Lv2–7.
- Original black-and-white adapted 2bpp sprite art for Pidgey, Rattata, plus original town NPC sprites.
- Simple wild battles: ThunderShock/Growl/Run. HP approximation from species base HP; not exact Gen1 battle engine; no catch mechanics or EXP yet.
- Pokémon Center room, receptionist, nurse, and restoration of Pikachu HP by talking across the counter. Other Viridian interiors are not playable yet.
- `save_v04.dat` stores the extra wild encounter fields; previous versioned saves remain untouched.
- Previous R0.3 Oak/Pikachu intro and companion remain functional.

## Tests

- `tests/test_core.c` — R0.3 Oak capture, rival battle, companion, save compatibility of current structure.
- `tests/test_negative.c` — loss, inability to run from trainer, warp, rejection of corrupted/outdated saves.
- `tests/test_route04.c` — Pallet ↔ Route 1 ↔ Viridian, grass encounter rate table, wild combat and Pokémon Center healing/exit, versioned save.
- ARM SDK compile and API check using Momentum `mntm-012`, API `87.1`.

Tests are host-side state-machine and build/link checks, **not evidence of executing on physical Flipper Zero**.

## Installation safety and project identity

- External app id: `yellow_native`; it does **not** modify firmware or Noosphere Pika.
- Saved file: `/ext/apps_data/yellow_native/save_v04.dat`.
- The standalone digital Pikachu HTask1k0 is a separate immutable identity; it must not be conflated with the game's Pokémon data or renamed from a changed Flipper label.
- Preserve all previous releases and saves. Verify SDK version before installing to hardware.

## Sources and restrictions

- Original Game Boy data and graphics: https://github.com/pret/pokeyellow (pinned commit in `source_manifest.json`).
- Original wild slot weights: `data/wild/probabilities.asm`; Route 1 table: `data/wild/maps/Route1.asm`.
- Original collision tile rule: `engine/overworld/player_state.asm` and `data/tilesets/collision_tile_ids.asm`.
- Game copyrights remain with their rights holders. For private modding/testing; redistribution requires appropriate rights.

## Still missing

Full Kanto, full event scripts, encounter/capture system, Pokédex/party/inventory, actual Gen1 battle formula, level/EXP progression, music/sound, native save conversion from original Game Boy cartridges, playtesting on real hardware. No claims these are done.
