#include "player.h"

#include <string.h>
#include <math.h>

Note* song = nullptr;
uint32_t songLength = 0;

// TODO add min available oscillator
// TODO reduce duty cycle
// TODO fast calculation of nNotes playing

struct osc {
    uint32_t duration_remaining;
    uint16_t period;
    uint16_t on_time;
    uint16_t on_offset;
    // maybe pad? uint16_t _;
};

static osc oscs[MAX_NOTES];
static uint32_t now = 0;
static uint32_t songPosition = 0;
bool songDone = false;
static uint32_t nextNoteTick = 0;
static uint16_t frequencyTable[256];
static uint16_t periodTable[256];

void initialize_play() {
    memset((void*)oscs,0,MAX_NOTES*sizeof(osc));
    now = 0;
    songPosition = 0;
    songDone = false;
    nextNoteTick = song[0]._start_tick;
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
}


uint16_t play() {
    if(songDone) return false;
    uint16_t isOn = 0;

    // count number of playing notes
    uint8_t nPlaying = 0;
    for(uint8_t i = 0; i < MAX_NOTES; i++)
        if(oscs[i].duration_remaining) nPlaying++;

    // play/end notes
    for(uint8_t i = 0; i < MAX_NOTES; i++) {
        osc* o = oscs + i;
        if(!o->duration_remaining) continue;

        // note is playing
        uint16_t progress = now % o->period;
        uint16_t effectiveOnTime = o->on_time / nPlaying;
        uint16_t unwrappedEnd = o->on_offset + effectiveOnTime;
        if(unwrappedEnd > o->period) {
            if(progress > o->on_offset || progress < (o->on_offset + effectiveOnTime - o->period)) {
                isOn++;
            }
        } else {
            if(progress > o->on_offset && progress < (o->on_offset + effectiveOnTime )) {
                isOn++;
            }
        }

        o->duration_remaining--;
    }

    // is there a new note?
    if(nextNoteTick <= now) {
        // there is note to load right now, find an oscillator
        osc* o = nullptr;
        for(uint8_t i = 0; i < MAX_NOTES; i++) {
            if(!oscs[i].duration_remaining) {
                o = oscs + i;
                break;
            }
        }

        if(!o) {
            printf("Too many notes! skipping...\n");
        } else {
            Note* note = song + songPosition;
            o->duration_remaining = note->_duration + (note->_extra_duration << 16); // wrong
            o->period = periodTable[note->_note];
            o->on_time = o->period / 2;
            o->on_offset = (o->period * (o - oscs)) / MAX_NOTES; // meh
        }

        // prepare for next note
        songPosition++;
        if(songPosition >= songLength) songDone = true;
        nextNoteTick = song[songPosition]._start_tick;
    }

    now++;
    return isOn;
}
