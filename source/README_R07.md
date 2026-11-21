# Pokémon Yellow Native — R0.7 (Flipper Zero)

**Private partial native port, not an emulator and not a completed Pokémon Yellow game.**

## New

- Battle with captured Pidgey or Rattata, not just Pikachu. Original 32×32 back sprites converted from pinned pret/pokeyellow into 1-bit resources.
- Party list supports selecting the active monster in overworld. Fight menu has SWITCH; pick a healthy member with OK. Voluntary switching spends a turn and allows an opponent attack.
- On fainting, the player must choose another conscious team member. Back cannot evade mandatory selection. Forced switch does not trigger a second enemy attack.
- If the whole team faints, trainer returns home and all team members recover; Pikachu is restored as active.
- Species-specific HP, PP, level, experience, moves and battle art; surviving battle participants split experience and advance independently.
- Party healing restores the caught Pokémon as well as Pikachu.
- New separate save_v07.dat stores party active index and progress, validates integrity. No migration from prior saves.
- Old maps, scenes and catches remain; Noosphere Pika immutable digital entity HTask1k0 stays separate and untouched.

## Known limitations

- Sand Attack currently consumes PP but has no accuracy-stage effect, and battles do not yet implement complete Gen1 mechanics, criticals, status, speed priority, party AI, full Pokédex or evolution.
- The experience-sharing approximation is not a complete Gen1 simulation.
- Only original 7 maps, Pidgey/Rattata + Pikachu, reduced intro and simplified battles.
- Does not automatically install to a physical Flipper or modify firmware; real device FPS, input and UI have not yet been verified.
- No Poké Ball refill except the noncanonical temporary intro developer kit.

## Verified

- test_core, test_negative, test_route04, test_battle05, test_capture06, test_party07, test_original_data: all PASS on host.
- Momentum mntm-012 API 87.1 ARM compile/link/APPCHK: PASS.
- Release includes all C sources, tests, source asset generator, original source manifest, FAP and verification evidence.

## Paths

Application ID: yellow_native
Proposed app path: /ext/apps/Games/yellow_native.fap
Save path: /ext/apps_data/yellow_native/save_v07.dat
Upstream: https://github.com/pret/pokeyellow pinned commit in source_manifest.json

Pokémon art/code rights belong to their respective owners. Private testing only; no unlicensed public redistribution.
