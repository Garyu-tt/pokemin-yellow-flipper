# R20 partial integrated staging (NOT FULL GAME RELEASE)
- Antidote native bag selection and poison cure connected to party state.
- Two-generation 24-byte Antidote inventory SD sidecars with parent checksum,
  generation, readback, exact length and seal-last writes.
- Missing legacy sidecar means stock zero. Present corrupt sidecar blocks
  further saves and item use; previous valid parent remains usable.
- No Antidote acquisition path implemented yet. No natural-input gameplay
  evidence, no physical Flipper test. YellowSave v13 unchanged.
