from pathlib import Path
p=Path(__file__).resolve().parents[1]
fl=(p/'yellow_flipper.c').read_text()
core=(p/'yellow_core.c').read_text()
header=(p/'yellow_core.h').read_text()
assert 'WHITEOUT_SAVE_A' in fl and 'WHITEOUT_SAVE_B' in fl
assert 'last_blackout_original_map' in header and 'last_blackout_integrity_error' in header
assert 'g->map==YMapViridianCenter' in core
assert 'if(!g->last_blackout_integrity_error)g->last_blackout_original_map=1u;' in core
assert fl.index('whiteout_slot_write(storage,path,&anchor)') < fl.index('atomic_write_inactive(storage,dest,next)')
assert 'y_whiteout_slot_restore(&g->last_blackout_original_map' in fl
assert 'g->last_blackout_integrity_error=true;' in fl
assert 'g->last_blackout_integrity_error || g->last_blackout_original_map>1u' in fl
assert 'yellow_whiteout_slot.c' in (p/'application.fam').read_text()
print('R13_NATIVE_SD_SLOT_WIRING_STATIC_CONTRACT_PASS')
