# Pokémon Yellow — CURRENT DEVELOPMENT BASE (2026-10-30)

**Development source:** /home/mirbi/projects/pokemon-yellow-native/source

**Immutable agent snapshot:** /home/mirbi/projects/mikoshi-chatgpt-bridge/runs/continuous/development-r10/source

**Baseline label:** development-r10 — **312 hash-verified files**, R07 gameplay/status patches, Antidote inventory, Mart/Bag interactions and atomic version-4 paired money+stock sidecar. Old v3 without a verifiable wallet is fail-closed.

**Historical accepted checkpoint:** v064 is retained for archival/release comparison ONLY. It is NOT the default source for NEW implementation tasks.

**Manifest:** /home/mirbi/projects/mikoshi-chatgpt-bridge/runs/continuous/development-base.json
**Manifest SHA256:** 0d8e501c4f48d9362fde08513a4603cb07a7f99d38453f35548539eb2d577f35

**Historical project source backup:** /home/mirbi/projects/pokemon-yellow-native/source.before-development-r10-20261030

New coders and reviewers must start from development-r10 unless reproducing a specifically historical issue, check all source hashes and review active worktree conflicts, develop only in isolated worktrees, add independent C/ARM/Unicorn proof, and never rewrite accepted v064 or release without separate acceptance.

Verified pre-integration: 163/163 C/Python, ARM APPCHK Target 7/API87.1, 8/8 save/SD emulator cases, 2/2 Antidote use/restart, and 5/5 negative-result mutations. **No physical Flipper validation or release.**

Outstanding: independent acceptance, natural story/item acquisition, v3-save downgrade compatibility with old firmware, physical SD/power-cut tests, end-to-end R0.12-C3/D completion, FAP bit-for-bit rebuild reproducibility.

**Promotion discipline:** Agents work on private copies of current development HEAD, then submit exact patches with source hash, pass C/Python/ARM checks; integration is serialized and reviewed. A successful hourly `finish` is not itself a merge.

## Shared-head engineering protocol
All five hourly agents and the coordinator use the single manifest:
`/home/mirbi/projects/mikoshi-chatgpt-bridge/runs/continuous/development-base.json`.
Before a new task, check the current revision and SHA with:
`python3 /home/mirbi/projects/mikoshi-chatgpt-bridge/pokemon_dev_integrator.py head`.
Never hardcode v064, R08, or R09 in new code. The source snapshot is immutable per task, and all code patches are reviewed/validated then merged through the serialized integrator.

To integrate a tested patch (NOT a release), use:
`python3 pokemon_dev_integrator.py integrate --patch /absolute/worker.patch --base-sha <SHA_from_claim> --worker poke-hourly-N --report /absolute/evidence.md`.
The command fails closed if the HEAD changed, reruns the complete C/Python suite, Cortex-M4 checks, offline ARM APPCHK and Unicorn when native files changed, then updates the common development HEAD with a new revision. If stale, rebase on the newest HEAD and rerun tests. No direct edits in the common tree and no automatic device flashing or official release.

**R10 integration:** source-backed Viridian Antidote purchase by native keys; buy/use/save/restart PASS in ARM Unicorn with story/initial map seeded. V3 and v4 money-sidecar deletion fail-closed. Shared test suite: 172/172 C/Python PASS. Source-backed R09 stand-alone money helper remains for research and is NOT the active game wallet serialization; live game uses paired stock+money v4 sidecar. A real Flipper release still requires full independent acceptance and hardware tests.
