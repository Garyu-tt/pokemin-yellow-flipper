#!/usr/bin/env python3
from pathlib import Path
import re
base=Path(__file__).resolve().parents[1]
up=Path('/home/mirbi/projects/pokemon-yellow-native/upstream/pokeyellow')
mart=(up/'data/maps/objects/ViridianMart.asm').read_text()
prices=(up/'data/items/prices.asm').read_text()
items=(up/'data/items/marts.asm').read_text()
script=(up/'scripts/ViridianMart.asm').read_text()
assert re.search(r'object_event\s+0,\s+5,\s+SPRITE_CLERK,\s+STAY,\s+RIGHT',mart)
assert re.search(r'bcd3\s+100\s*;\s*ANTIDOTE\b',prices)
assert re.search(r'ViridianMartClerkText::\s*\n\s*script_mart\s+POKE_BALL,\s*POTION,\s*ANTIDOTE,',items)
assert 'EVENT_OAK_GOT_PARCEL' in script
c=(base/'yellow_core.c').read_text()
fl=(base/'yellow_flipper.c').read_text()
assert 'g->x==2 && g->y==5 && g->facing==YLeft' in c
assert 'x==1 && y==5 && g->x==2 && g->y==5' in c
assert 'key==YBack && g->pending==YPendingBuyAntidote' in c
assert 'yellow_game_buy_antidote(g,1u)' in c
assert 'g->map==YMapViridianMart' in fl and '5*16-cam_y,YActorReceptionist' in fl
assert 'yellow_game_apply_antidote(&a->g' in fl
assert 'money_stock_slot_write' in fl and 'money_stock_slot_read' in fl
assert 'g->antidote_integrity_error=true;' in fl
assert 'parent->version>=3u' in fl
# Strengthened migration guard: unknown v3 balances must also fail closed.
assert 'parent->version>=3u' in fl and 'g->money_integrity_error=true;' in fl
assert '#define YELLOW_ATOMIC_ALLOW_V4 1' in fl
print('SOURCE_PIN_CLERK_COUNTER_PRICE_GATE_UI_SAVE_AND_BAG_WIRING_PASS')
