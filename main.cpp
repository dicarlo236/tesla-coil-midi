#include <iostream>
#include <stdio.h>
#include <string>
#include <stdint.h>
#include <vector>
#include <algorithm>
#include <fstream>

#include "midifile/include/MidiFile.h"
#include "player.h"
#include <iostream>
#include <chrono>

class Timer
{
public:
    Timer() : beg_(clock_::now()) {}
    void reset() { beg_ = clock_::now(); }
    double elapsed() const {
        return std::chrono::duration_cast<second_>
            (clock_::now() - beg_).count(); }

private:
    typedef std::chrono::high_resolution_clock clock_;
    typedef std::chrono::duration<double, std::ratio<1> > second_;
    std::chrono::time_point<clock_> beg_;
};

// resolution of notes stopping and starting

using namespace smf;
using std::string;
using std::vector;



uint32_t secondsToTicks(double seconds) {
    if(seconds < .001) printf("!note with duration %f\n",seconds);
    return seconds * SAMPLE_FREQEUENCY / (1 << CHEATER_FREQUENCY_MULT_POWER);
}

void songToData(std::vector<Note>& notes, std::string fileName) {
    std::ofstream fs(fileName);
    fs << "const uint32_t SONG[" << notes.size() <<"] = {";
    for(int i = 0; i < notes.size(); i++) {
        fs << *(uint32_t*)(notes.data() + i);
        if(i < notes.size() - 1)
            fs << ", ";
    }
    fs << "};";
    fs.close();
}

void boundPitches(int max, vector<Note>& notes) {
    int minFound = 256;
    int maxFound = 0;

    for(auto& note : notes) {
        if(note._note > maxFound) maxFound = note._note;
        if(note._note < minFound) minFound = note._note;
    }

    printf("bound pitches to %d on (%d, %d)\n", max, minFound, maxFound);

    if(max > maxFound) {
        printf("!pitch %d out of bounds\n", maxFound);
    }

}


void processMidiFile(const char* filename) {
    std::string fileString(filename);
    printf("Processing file...\n\tname: %s\n", filename);
    MidiFile midifile;
    midifile.read(fileString);
    printf("\tTracks: %d\n", midifile.getTrackCount());
    printf("\tTicks per Quarter Note: %d\n", midifile.getTicksPerQuarterNote());

    midifile.doTimeAnalysis();
    midifile.linkNotePairs();

    vector<Note> notes;

    for(int track = 0; track < midifile.size(); track++) {
        int nEvents = 0;
        double maxDuration = 0.;
        for(int event = 0; event < midifile[track].size(); event++) {
            if(midifile[track][event].isNoteOn()) {
                double duration = midifile[track][event].getDurationInSeconds();
                if(duration > maxDuration) maxDuration = duration;
                notes.emplace_back(secondsToTicks(midifile[track][event].seconds),
                                   secondsToTicks(duration),
                                   midifile[track][event][1]);
//                for(int i = 0; i < midifile[track][event].size(); i++)
//                    printf("%x ", midifile[track][event][i]);
//                printf("\n");
                nEvents++;
            }
        }
        printf("Track %d had %d notes, max length %.3f seconds\n",track,nEvents,maxDuration);
    }

    std::sort(notes.begin(), notes.end(),[](const Note& a, const Note& b) -> bool {
        return a._start_tick < b._start_tick;
    });

    printf("Size of song: %lu bytes\n", sizeof(Note) * notes.size());

    boundPitches(128,notes);

    song = notes.data();
    songLength = notes.size();
    initialize_play();

    vector<uint8_t> music;
    music.reserve(secondsToTicks(midifile.getFileDurationInSeconds()));

    Timer t;
    int i = 0;
    while(!songDone) {
        music.push_back(play()>0?30:-30);
        //music.push_back( play() * 5 );
    }
    float secs = t.elapsed();
    printf("Done in %.3f ms\n", secs * 1000.f);
    printf("%.3fx faster than real time\n", midifile.getFileDurationInSeconds() / secs);

    printf("Got %lu samples\n", music.size());

    string rawName = fileString + ".raw";
    FILE* fptr = fopen(rawName.c_str(), "wb");
    fwrite(music.data(), music.size(), 1,fptr);
    fclose(fptr);

    songToData(notes, fileString + ".txt");

}

int main(int argc, char** argv)
{
    printf("Size of note: %lu bytes\n", sizeof(Note));
    if(argc < 2) {
        printf("Usage: midi-tool [MIDI-FILES]...\n");
        return EXIT_FAILURE;
    }

    for(char** arg = argv + 1; arg < argv + argc; arg++)
        processMidiFile(*arg);
}
