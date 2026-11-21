#ifndef YELLOW_PALLET_AUDIO_H
#define YELLOW_PALLET_AUDIO_H
#include <stdbool.h>
#include <stdint.h>
#define YELLOW_PALLET_TEMPO_BPM 160u
#define YELLOW_PALLET_NOTE_COUNT 48u
typedef struct {
    uint16_t hz;
    uint8_t units; /* Gen1 note length: 1 = 1/16 whole */
} YellowPalletTone;
typedef struct {
    uint32_t next_change_ms;
    uint8_t cursor;
    bool started;
} YellowPalletMusic;
/* Deterministic fixed-point duration from source tempo and note_type 12. */
uint16_t yellow_pallet_note_duration_ms(uint8_t note_units);
bool yellow_pallet_tone_at(unsigned i,YellowPalletTone* out);
void yellow_pallet_music_reset(YellowPalletMusic* music);
bool yellow_pallet_music_poll(YellowPalletMusic* music,uint32_t now_ms,
                              YellowPalletTone* changed);
#endif
