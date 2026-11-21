"""Pinned pret/pokeyellow evidence for the bounded R0.14 lab-exit guard."""
from pathlib import Path
import os
upstream=Path(os.environ.get("PKYELLOW_SOURCE", Path(__file__).resolve().parents[3]/"upstream/pokeyellow"))
script=(upstream/"scripts/OaksLab.asm").read_text()
text=(upstream/"text/OaksLab.asm").read_text()
assert "OaksLabPlayerDontGoAwayScript:" in script
assert "OaksLabPlayerForcedToWalkBackScript:" in script
assert "TEXT_OAKSLAB_OAK_DONT_GO_AWAY_YET" in script
assert "_OaksLabOakDontGoAwayYetText::" in text
assert "Don't go" in text and "away yet!" in text
print("R14_OAK_EXIT_PINNED_UPSTREAM_PASS")
