#pragma once

#include <Audio.h>

#include "Config.h"
#include "Sequencer.h"

void initDrums(AudioMixer4 *mix, AudioPlayMemory *s1, AudioPlayMemory *s2, AudioPlayMemory *s3, AudioPlayMemory *s4);
void setDrumsConfig(drumCfg *cfg);
float adjustDrumsVolume(int d);
bool drumsEnabled();
bool isDrum(Beat beat);
void managebeatstate(bool back);
void manageoffbeatstate(bool back);
void managedrumclick(bool back);
void manageoffbeatclick(bool back);
void playDrumSample(Beat beat, bool accent);
const char *getBeatString(Beat beat, bool shortDesc);
const char *getKitString(bool shortDesc);
