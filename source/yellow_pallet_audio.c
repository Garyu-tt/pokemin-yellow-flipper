/* A source-verified, MONOPHONIC 48-event excerpt from pinned pret/pokeyellow
 * audio/music/pallettown.asm Music_PalletTown_Ch1 .mainloop.
 * NOT equivalent to the whole 3-channel original Game Boy sound engine.
 * Pure bounded sequencer, independently testable on host without speaker. */
#include "yellow_pallet_audio.h"
#include <string.h>
static const YellowPalletTone pallet_ch1[YELLOW_PALLET_NOTE_COUNT]={
    {247,4},
    {262,2},
    {294,4},
    {392,2},
    {294,2},
    {262,2},
    {247,4},
    {196,2},
    {294,4},
    {294,2},
    {262,2},
    {247,2},
    {0,2},
    {247,2},
    {262,2},
    {247,2},
    {262,8},
    {0,2},
    {247,2},
    {262,2},
    {220,2},
    {247,2},
    {196,2},
    {220,2},
    {185,2},
    {247,4},
    {262,2},
    {294,4},
    {392,2},
    {294,2},
    {262,2},
    {247,4},
    {196,2},
    {294,4},
    {294,2},
    {392,2},
    {370,2},
    {330,4},
    {294,2},
    {262,4},
    {220,2},
    {247,2},
    {262,2},
    {294,2},
    {262,2},
    {247,2},
    {220,2},
    {196,4},
};
uint16_t yellow_pallet_note_duration_ms(uint8_t units) {
    if(units==0||units>16)return 0;
    /* one unit = 60000/(160 BPM * 4) = 93.75 milliseconds */
    return (uint16_t)((60000u*units+320u)/(160u*4u));
}
bool yellow_pallet_tone_at(unsigned i,YellowPalletTone* out) {
    if(!out||i>=YELLOW_PALLET_NOTE_COUNT)return false;
    *out=pallet_ch1[i];return true;
}
void yellow_pallet_music_reset(YellowPalletMusic* music) {
    if(music)memset(music,0,sizeof(*music));
}
bool yellow_pallet_music_poll(YellowPalletMusic* music,uint32_t now,
                              YellowPalletTone* changed) {
    if(!music||!changed)return false;
    if(!music->started) {
        music->started=true;
        music->next_change_ms=now;
    }
    if((int32_t)(now-music->next_change_ms)<0)return false;
    *changed=pallet_ch1[music->cursor];
    music->cursor=(uint8_t)((music->cursor+1u)%YELLOW_PALLET_NOTE_COUNT);
    music->next_change_ms=now+yellow_pallet_note_duration_ms(changed->units);
    return true;
}
