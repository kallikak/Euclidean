#pragma once

#include "Config.h"

float quantise(float f, tuningCfg *cfg, bool transpose);

float freqToNote(float f);

float noteToFreq(float n);

float freqToMIDINote(float f);

float offsetToFreq(int k, float f0);

int roundnotenumber(float n);

const char *notestring(float n);

float detuneCents(float f, int c);

int16_t movingaverage(int16_t newvalue, int16_t avg, int n);

float movingaveragefloat(float newvalue, float avg, int n);
