# Pokemon Yellow Native for Flipper Zero - R0.3

Status: private **partial** native port, not full Pokemon Yellow and not a Game Boy emulator.

## Built and tested
- ARM Cortex-M4 external Flipper application (.fap), built against Momentum mntm-012 API 87.1.
- 4 original maps: Pallet Town (10x9 blocks), Red's House 1F, Blue's House, Oak's Lab. Exact block data from pret/pokeyellow, no hand-drawn replacements.
- Native, non-scaled 128x64 viewport, source sprites and four-level tile graphics converted to 1 bit with ordered Bayer dithering.
- Intro starts in Red's house; north-town event triggers Oak's warning, a simplified Oak catches Pikachu scene, Oak's lab, rival takes Eevee, Pikachu as starter and a simplified single battle.
- Pikachu follows player tile by tile, follows through warps, supports face + OK interaction; first D-pad press toward follower turns, second can exchange positions.
- Validated saves in /ext/apps_data/yellow_native/save_v03.dat (R0.2 save file remains intact).
- Sprite data linked once (yellow_assets.c), not duplicated into all C source objects; app around 27KB.
- Native state-machine test and negative test, including recovery after rival battle loss and rejecting invalid save files.

## Known limitations
- **Not** the canonical full Oak cutscene, dialogue, battle formula, music, 151-mon roster, all maps and encounters. Opening scenario is a reduced playable adaptation; rival fight uses simplified deterministic damage.
- NPCs are static, actor sprite sideways mirror and full scene pacing are pending.
- Wild encounters and full world progression are not implemented yet.
- Device execution **not yet verified**. FAP build and API import validation pass; live FPS, memory and actual input need testing on Flipper.
- Third-party Pokémon art/game data copyrighted; use privately and do not distribute as an independent public game without appropriate rights.

## Project separation
- Application ID: yellow_native, not noosphere_pika. Installing this .fap alongside Noosphere Pika does not replace its files.
- The separate digital companion **HTask1k0** has an immutable identity. This native Pokémon Yellow port's in-game Pokémon is not an alias of that digital entity.

## Build
With uFBT set to Momentum mntm-012, use the application manifest and C files in the project root. Included files: yellow_core.c/h, yellow_flipper.c, yellow_assets.c/h, application.fam.

## Core tests
The host tests include both C implementation modules into a single test executable:

    cc -std=c11 -I. tests/test_core.c -o /tmp/yn-test-core && /tmp/yn-test-core
    cc -std=c11 -I. tests/test_negative.c -o /tmp/yn-test-negative && /tmp/yn-test-negative

The tests were executed with the local portable TinyCC against a temporary extracted glibc-dev system header/toolset on mirbi-linux; tests and FAP build both passed.

## Asset provenance
Source: https://github.com/pret/pokeyellow . Pin exact commit from source_manifest.json. Assets generated using tools/generate_assets.py from a local checkout of the corresponding commit at /tmp/pokeyellow. Original in-game dialogue has been selectively shortened/reformatted for 128x64 display.
