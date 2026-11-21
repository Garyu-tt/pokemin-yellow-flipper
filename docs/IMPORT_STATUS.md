# Import manifest — 2026-11-21

Source host: `mirbi-linux`, project tree `/home/mirbi/projects/pokemon-yellow-native`, experimental branch tree `/home/mirbi/projects/pokemon-yellow-glitchlab`.

This repository imports **the current native source**, source tests and utilities, GlitchDex standalone experiment, available graph views and compressed node/edge data, milestone reports, and the historical R02–R12C2 zip bundles and R03–R12C2 FAP binaries plus GlitchLab test FAP.

Source timeline: the working directory was last observed around 2026-11-08. Historical accepted v064 and current development source are not equivalent. Builds in `releases/` are historical artifacts, **not** claims that the newest source is release-ready or tested on physical hardware.

Deliberate exclusions:
- 3.5+ GB of duplicate `source.before-development-*` snapshots, generated research/build trees, compiler outputs and repeated copies. The working files and test sources are preserved.
- `upstream/pokeyellow` clone. Use upstream [pret/pokeyellow](https://github.com/pret/pokeyellow) at the revision recorded in `source/source_manifest.json`; the original ROM is not included.
- 20 MB generated SQLite graph database, and the redundant combined graph archive. Node/edge `.jsonl.gz` inputs, HTML/DOT views and generating utilities are included.
- Compiled `.o` files, firmware checkout/vendor dependencies and local caches/credential stores.

**Independent validation still required:** repeat a clean build for current Flipper/Momentum SDK, check API compatibility, physical display/input/audio, SD save/load and power-loss recovery, and complete game progression. A historic FAP passing an emulator is not device acceptance.

No software license is asserted for third-party assets, ROM-derived material or trademarks.
