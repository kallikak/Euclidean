#pragma once

#define MIDI_CLOCKS_PER_BEAT 12

void managePlayState();

void setupMIDI();

void checkMIDI();

void resetMIDI();

void sendMIDINoteOn(byte note, byte vel, byte chan);

void sendMIDINoteOff(byte note, byte chan);

void stopAllMIDI();

void sendMIDIClock();

void sendMIDIClockStart();

void sendMIDIClockContinue();

void sendMIDIClockStop();
