#!/usr/bin/env python3
from pathlib import Path
p=Path(__file__).resolve().parents[1]
f=(p/'yellow_flipper.c').read_text()
h=(p/'yellow_core.h').read_text()
assert '#define ANTIDOTE_SAVE_A' in f and '#define ANTIDOTE_SAVE_B' in f
assert 'yellow_antidote_inventory_create(g->antidotes,next->parent.checksum' in f
assert 'if(ok)ok=atomic_write_inactive(storage,dest,next);' in f
assert f.index('yellow_antidote_inventory_create(g->antidotes,next->parent.checksum') < f.index('if(ok)ok=atomic_write_inactive(storage,dest,next);')
assert 'antidote_slot_read(storage,item_path,&item)' in f
assert 'parent->parent.checksum,parent->generation,&g->antidotes' in f
assert 'g->antidote_integrity_error=true' in f
assert 'g->antidote_integrity_error || g->antidotes>99u' in f
assert 'yellow_game_apply_antidote(&a->g,a->g.active_party,' in f
assert 'apply_key=false; /* never accidentally use Potion too */' in f
assert 'uint8_t antidotes;' in h and 'bool antidote_integrity_error;' in h
assert 's->antidotes' not in (p/'yellow_core.c').read_text()
print('R20_BAG_INPUT_SD_TWO_GENERATION_PARENT_SEAL_FAILCLOSED_STATIC_CONTRACT_PASS_PARTIAL')
