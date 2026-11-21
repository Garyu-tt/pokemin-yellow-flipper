#!/usr/bin/env python3
"""Independent static wiring oracle for Flipper-only UI + runtime safety."""
from pathlib import Path
src=Path(__file__).resolve().parents[1]
ui=(src/'yellow_flipper.c').read_text()
core=(src/'yellow_core.h').read_text()
slot=(src/'yellow_oak_parcel_slot.c').read_text()
assert 'bool parcel_warning_acknowledged;' in ui
assert 'if(a->g.oak_parcel_integrity_error && !a->parcel_warning_acknowledged)' in ui
assert 'if(event.type==InputTypeShort && (key==YOk||key==YBack))' in ui
assert 'a->parcel_warning_acknowledged=true;' in ui
assert 'PARCEL RECORD BAD' in ui and 'SAVING DISABLED' in ui
assert 'SD DATA UNCHANGED' in ui and 'SAVE LOCKED' in ui
assert 'g->oak_parcel_integrity_error=true;' in ui
assert 'y_oak_parcel_runtime_safe(g->oak_parcel_flags,g->oak_parcel_integrity_error)' in ui
assert 'return !integrity_error && y_oak_parcel_valid(flags);' in slot
assert 'bool oak_parcel_integrity_error;' in core
assert 'bool parcel_warning_acknowledged;' not in core
assert 'parcel_warning_acknowledged=false;' not in ui
print('R014_UI_WARNING_VISIBLE_AND_SAVE_GUARD_UNMODIFIED_STATIC_PASS')
