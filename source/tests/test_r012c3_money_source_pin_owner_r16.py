import os
from pathlib import Path

root=Path(os.environ["PKYELLOW_SOURCE"])
source=(root/"engine/events/black_out.asm").read_text()
assert "ResetStatusAndHalveMoneyOnBlackout::" in source
assert "wPlayerMoney" in source
assert "predef DivideBCDPredef3" in source
assert "ld a, 2" in source
assert "predef_jump HealParty" in source
print("R012C3_WHITEOUT_BCD_SOURCE_PIN_PASS_NOT_RUNTIME_INTEGRATION")
