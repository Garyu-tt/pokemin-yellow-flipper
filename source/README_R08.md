# Pokémon Yellow Native — R0.8 (Flipper Zero)

**Private partial native port, not an emulator and not the completed original game.**

## Changes in R0.8

- Sand Attack now applies actual accuracy stage reduction (up to -6), and hit checks use Gen1 accuracy/evasion ratio tables. The 255/256 cap for a nominal 100% accuracy move is respected.
- ThunderShock uses the original 10%-plus-one-byte threshold (26/256) for secondary paralysis on successful, non-KO hits against a target without a persistent status.
- Fully paralyzed Pokémon have a 25% chance to miss their action. PP consumption and battle turn ordering are currently approximations.
- Pokémon paralysis status is individually stored for Pikachu and all caught monsters, restored from a new versioned save, and cured by the Pokémon Center. Opponent paralysis applies during the encounter and is inherited if captured.
- Basic Poké Ball capture adjusts Gen1 Rand1 by 12 for paralyzed targets. Other status bonuses (sleep/freeze) aren't implemented.
- Party and battle HUDs display PAR.
- Includes an approximate quarter-speed function for paralysis, **not yet used to schedule battle action order**.
- Save data version 8 uses `/ext/apps_data/yellow_native/save_v08.dat`. Prior save files and FAP releases remain untouched.

## Test evidence

- Seven host C suites PASS: test_core, test_negative, test_route04, test_battle05, test_capture06, test_party07, test_effects08.
- Pinned upstream data verification PASS: test_original_data.py verifies moves, accuracy stages, 10%-status threshold, 25%-skip threshold, 12-point Poké Ball modifier.
- Momentum mntm-012 SDK API 87.1 native ARM compile/link/APPCHK PASS.
- No physical device gameplay/graphics/performance validation yet.

## Known limitations

Only seven map areas, initial intro, Route 1 encounters and first Pokémon Center. Gen1 combat and experience formulas are incomplete: no crits, complete status set, true priority/speed initiative, all species, full evolution or detailed opponent AI. Fully paralyzed action uses simplified timing. Wild monsters are Pidgey/Rattata, and the initial Poké Ball allowance remains a temporary dev shortcut. No save migration or full cart-save conversion.

No changes to Flipper firmware or the independent Noosphere Pika entity HTask1k0. Copyrighted Pokémon game materials originate with rights holders; private development/testing only.

Source: https://github.com/pret/pokeyellow (exact revision in source_manifest.json).
