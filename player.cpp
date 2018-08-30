#include "player.h"

#include <string.h>
#include <math.h>
#include <stdlib.h>

Note* song = nullptr;
uint32_t songLength = 0;


struct osc {
    uint32_t duration_remaining;
    uint16_t period;
    uint16_t on_time;
};

static osc oscs[MAX_NOTES];
static osc* oscPool[MAX_NOTES];
static osc** oscPoolEnd;
static osc** oscPoolPtr;
static uint32_t now = 0;
static uint32_t songPosition = 0;
bool songDone = false;
static uint32_t nextNoteTick = 0;
static uint16_t frequencyTable[256];
static uint16_t periodTable[256];
static uint8_t nPlaying = 0;

void initialize_play() {
    memset((void*)oscs,0,MAX_NOTES*sizeof(osc));
    now = 0;
    songPosition = 0;
    songDone = false;
    nextNoteTick = song[0]._start_tick;
    nPlaying = 0;
    for(int i = 0; i < 256; i++)
    {
        float offset = 69;
        float f = 440.f * pow(2.f, (i - offset)/12.0f);
        float f2 = ((float)SAMPLE_FREQEUENCY) / f;
        frequencyTable[i] = 440*pow(2,(i - offset)/12.0f);
        periodTable[i] = ((uint32_t)f2) > 1 ? f2 : 1;
        //printf("period %d %d (%d Hz)\n", i, periodTable[i],frequencyTable[i]);
    }
    printf("Done initialize\n");

    for(int i = 0; i < MAX_NOTES; i++) {
        oscPool[i] = oscs + i;
    }
    oscPoolEnd = oscPool + MAX_NOTES;
    oscPoolPtr = oscPool;
}


uint16_t play() {
    if(songDone) return 0;
    uint16_t isOn = 0;

    // play/end notes
    for(uint8_t i = 0; i < MAX_NOTES; i++) {
        osc* o = oscs + i;
        if(!o->duration_remaining) continue;

        // note is playing
        uint16_t progress = now % o->period;
        uint16_t effectiveOnTime = o->on_time / nPlaying;

        if(progress < effectiveOnTime) {
            isOn += 1;
        }

        if(o->duration_remaining == 1) {
            nPlaying--;
            *(--oscPoolPtr) = o;
        }
        o->duration_remaining--;
    }

    // avoids mod.
    if(!(now & (1 << CHEATER_FREQUENCY_MULT_POWER))) {
        // is there a new note?
        if(nextNoteTick <= (now >> CHEATER_FREQUENCY_MULT_POWER) && oscPoolPtr < oscPoolEnd) {
            // there is note to load right now, find an oscillator
            osc* o = *(oscPoolPtr++);

            if(!o) {
                printf("Too many notes! skipping...\n");
            } else {
                Note* note = song + songPosition;
                o->duration_remaining = ((uint32_t)note->_duration) << CHEATER_FREQUENCY_MULT_POWER;
                o->period = periodTable[note->_note];
                o->on_time = o->period / 2;
                nPlaying++;
            }

            // prepare for next note
            songPosition++;
            if(songPosition >= songLength) songDone = true;
            nextNoteTick = song[songPosition]._start_tick;
        }
    }


    now++;
    return isOn;
}
