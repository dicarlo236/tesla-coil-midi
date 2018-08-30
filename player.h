#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>
#include <stdio.h>

// Midi player for tesla coils
// - adjusts duty cycle to prevent coil from exploding when many notes play simultaneously
// - messes with phase offset to work around 1-bit output resolution of tesla coil when possible

// set this to audio sample frequency
#define SAMPLE_FREQEUENCY 48000

// maximum number of notes that can be played
// if this is too large, the code will be slow
// it this is too small, notes might be left out
#define MAX_NOTES 10

// 64-bit struct representing a single note to be played
// each tick is 1/sample_frequency long
// notes contain:
// - starting time (up to 2^32 ticks, ~1 day at 50 kHz)
// - duration (up to 2^24 ticks, ~6 minutes at 50 kHz)
//   * the lower 16 bits of duration are stored in _duration
// - note (midi notes are 0-255, this is looked up in the frequency table)
struct Note {
    Note(uint32_t start, uint32_t duration, uint8_t note) :
        _start_tick(start),
        _note(note) {
        if(duration >= (1 << 24))
            printf("! Duration %d is too long\n", duration);
        _duration = duration & 0xffff;
        _extra_duration = 0xff & (duration >> 16);
    }
    uint32_t _start_tick;
    uint16_t _duration;
    uint8_t  _note;
    uint8_t  _extra_duration;
};


// set this to the Note array for the song
extern Note* song;

// set this to the length of the Note array
extern uint32_t songLength;

// this becomes true when the song is done playing
extern bool songDone;


// call this at SAMPLE_FREQUENCY to determine if the output should be enabled or not
uint16_t play();

// call this once BEFORE calling play the first time and every time after switching songs
void initialize_play();

#endif // PLAYER_H
