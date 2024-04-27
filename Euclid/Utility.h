#pragma once

#define MSG(v) {Serial.println(v);}
#define DBG(v) {Serial.print(#v " = ");Serial.println(v);}
#define DBG2(s,v) {Serial.print(#s " = ");Serial.println(v);}
#define DBG3(s,v) {Serial.print(#s ", " #v " = ");Serial.println(v);}
#define MSGn(v) {Serial.print(v);}
#define DBGn(v) {Serial.print(#v " = ");Serial.print(v);Serial.print(" ");}
#define DBG2n(s,v) {Serial.print(#s " = ");Serial.print(v);Serial.print(" ");}
#define DBG3n(s,v) {Serial.print(#s ", " #v " = ");Serial.print(v);Serial.print(" ");}
#define DBGLINE {Serial.println("----------------");}

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
