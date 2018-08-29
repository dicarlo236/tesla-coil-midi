cmake_minimum_required(VERSION 2.8)
project(teslaCoilMidi)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(CheckIncludeFiles)

#  Build midifile library
set(midifile-srcs
    midifile/src-library/Options.cpp
    midifile/src-library/Binasc.cpp
    midifile/src-library/MidiEvent.cpp
    midifile/src-library/MidiEventList.cpp
    midifile/src-library/MidiFile.cpp
    midifile/src-library/MidiMessage.cpp
    )
set(midifile-hdrs
    midifile/include/Binasc.h
    midifile/include/MidiEvent.h
    midifile/include/MidiEventList.h
    midifile/include/MidiFile.h
    midifile/include/MidiMessage.h
    midifile/include/Options.h
    )
include_directories(midifile/include)
add_library(midifile STATIC ${midifile-srcs} ${midifile-hdrs})

# Build actual application
add_executable(${PROJECT_NAME} "main.cpp")
target_link_libraries(${PROJECT_NAME} midifile)
