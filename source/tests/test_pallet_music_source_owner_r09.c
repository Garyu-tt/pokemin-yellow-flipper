#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "../yellow_pallet_audio.c"
static void source_excerpt_and_limits(void) {
    YellowPalletTone t;
    assert(YELLOW_PALLET_TEMPO_BPM==160 && YELLOW_PALLET_NOTE_COUNT==48);
    assert(!yellow_pallet_tone_at(0,NULL));
    assert(!yellow_pallet_tone_at(YELLOW_PALLET_NOTE_COUNT,&t));
    static const uint16_t first[]={247,262,294,392,294,262,247};
    static const uint8_t units[]={4,2,4,2,2,2,4};
    for(unsigned i=0;i<7;i++) {
        assert(yellow_pallet_tone_at(i,&t));
        assert(t.hz==first[i] && t.units==units[i]);
    }
    unsigned rests=0,ms=0;
    for(unsigned i=0;i<YELLOW_PALLET_NOTE_COUNT;i++) {
        assert(yellow_pallet_tone_at(i,&t));
        assert(t.units>=1 && t.units<=8);
        if(!t.hz)rests++;
        else assert(t.hz>=100 && t.hz<=1000);
        ms+=yellow_pallet_note_duration_ms(t.units);
    }
    assert(rests==2 && ms==11643);
    assert(yellow_pallet_note_duration_ms(0)==0);
    assert(yellow_pallet_note_duration_ms(17)==0);
    assert(yellow_pallet_note_duration_ms(1)==94);
    assert(yellow_pallet_note_duration_ms(2)==188);
    assert(yellow_pallet_note_duration_ms(4)==375);
}
static void playback_without_blocking(void) {
    YellowPalletMusic music={0};
    YellowPalletTone tone={0};
    assert(!yellow_pallet_music_poll(NULL,0,&tone));
    assert(!yellow_pallet_music_poll(&music,0,NULL));
    assert(yellow_pallet_music_poll(&music,1000,&tone));
    assert(tone.hz==247 && tone.units==4);
    assert(music.next_change_ms==1375 && music.cursor==1);
    assert(!yellow_pallet_music_poll(&music,1374,&tone));
    assert(yellow_pallet_music_poll(&music,1375,&tone));
    assert(tone.hz==262 && tone.units==2);
    assert(music.next_change_ms==1563);
    assert(!yellow_pallet_music_poll(&music,1562,&tone));
    uint32_t ms=music.next_change_ms;
    for(unsigned i=2;i<YELLOW_PALLET_NOTE_COUNT;i++) {
        assert(yellow_pallet_music_poll(&music,ms,&tone));
        ms=music.next_change_ms;
    }
    assert(music.cursor==0);
    assert(yellow_pallet_music_poll(&music,ms,&tone));
    assert(music.cursor==1 && tone.hz==247);
    yellow_pallet_music_reset(&music);
    assert(!music.started && music.cursor==0);
    assert(yellow_pallet_music_poll(&music,UINT32_MAX-10u,&tone));
    assert(!yellow_pallet_music_poll(&music,UINT32_MAX-9u,&tone));
    assert(yellow_pallet_music_poll(&music,364u,&tone));
    assert(tone.hz==262);
}
int main(void) {
    source_excerpt_and_limits();
    playback_without_blocking();
    puts("PALLET_TOWN_48_SOURCE_CH1_NOTES_NONBLOCKING_RESTS_WRAP_PASS");
    return 0;
}
