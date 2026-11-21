#!/usr/bin/env python3
"""Independent source pin for R21; not a gameplay-completion claim."""
import os
import pathlib
import re

root=pathlib.Path(os.environ.get("PKYELLOW_SOURCE",
    "/home/mirbi/projects/pokemon-yellow-native/upstream/pokeyellow"))
marts=(root/"data/items/marts.asm").read_text()
prices=(root/"data/items/prices.asm").read_text()
script=(root/"scripts/ViridianMart.asm").read_text()
assert re.search(r"ViridianMartClerkText::\s*\n\s*script_mart\s+POKE_BALL,\s*POTION,\s*ANTIDOTE\b",marts)
assert re.search(r"bcd3\s+100\s*;\s*ANTIDOTE\b",prices)
assert "EVENT_OAK_GOT_PARCEL" in script
assert "EVENT_GOT_OAKS_PARCEL" in script
assert script.index("EVENT_OAK_GOT_PARCEL")!=script.index("EVENT_GOT_OAKS_PARCEL")
print("R21_ORIGINAL_VIRIDIAN_MART_SOURCE_PRICE_AND_GATE_PASS")
