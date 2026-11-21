# Pokémon Yellow Native for Flipper Zero

An experimental **native C/ARM port** of Pokémon Yellow for Flipper Zero's 128×64 monochrome screen. This is a work-in-progress, not a complete Game Boy emulator or a verified full-game release.

## Repository layout

- `source/` — current native application source, build manifest, utilities, and tests from `pokemon-yellow-native/source`.
- `experiments/glitchlab/` — standalone GlitchDex experiment and its tests/scripts; separate from the main game.
- `graph/` and `graph_tools/` — world graph sources, reports, and graph-generation utilities.
- `docs/` — project progress, test notes, and historical development context.
- `releases/` — selected experimental FAP builds and their associated test reports (not hardware-certified).

## Status

The latest working source snapshot was taken from the owner's `mirbi-linux` development workspace. Prior development reports describe host tests, ARM checks, emulated runs and SD persistence checks; **physical Flipper Zero testing and complete playthrough are not confirmed**. A passing test does not make a release hardware-ready.

Native architecture targets a direct ARM app and a redesigned monochrome UI rather than enlarging the Game Boy display. GlitchLab's on-device summon menu is experimental and does not reproduce original ROM memory corruption.

## Build and use

The primary Flipper application manifest is `source/application.fam`. Build against a matching Flipper/Momentum SDK (API compatibility may vary); see the imported README_R*.md and tests for historical build checks. Place a compatible experimental `.fap` under `/ext/apps/Games/` to try on a device. Back up saves before experiments.

## Source and rights

The original Pokémon Yellow game, artwork, names and music belong to their respective rights holders. The upstream disassembly/source research is credited to [pret/pokeyellow](https://github.com/pret/pokeyellow); this repository does not include a copyrighted ROM image. Distribution/use of original-game-derived data and images may require separate rights. This is an unofficial, non-commercial experimental project.

## Roadmap

Finish end-to-end gameplay coverage, verify native Flipper startup, SD/power-loss recovery, input/render/audio, and full-game progression on actual hardware. Keep PikaCore/tamagotchi and firmware shell experiments separated from the primary game until independently validated.
