from pathlib import Path
import os

upstream=Path(os.environ["PKYELLOW_SOURCE"])
files={
    "engine/events/black_out.asm": ("ResetStatusAndHalveMoneyOnBlackout", "DivideBCDPredef3", "HealParty"),
    "engine/events/set_blackout_map.asm": ("SetLastBlackoutMap:", "wLastBlackoutMap", "wLastMap"),
    "engine/events/poison.asm": ("ApplyOutOfBattlePoisonDamage:", "TEXT_BLACKED_OUT", "wOutOfBattleBlackout"),
    "constants/map_constants.asm": ("map_const VIRIDIAN_CITY", "map_const VIRIDIAN_POKECENTER"),
}
for relative,needles in files.items():
    text=(upstream/relative).read_text()
    for needle in needles:
        assert needle in text, (relative,needle)
print("WHITEOUT_PINNED_UPSTREAM_SOURCE_PATHS_AND_BCD_RULE_PASS")
