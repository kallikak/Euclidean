#pragma once

#define PROTOTYPE 0

#define MAX_STEP 16
#define MAX_SAMPLESET_NAME_LENGTH 20

#define DEBUG_SERIAL 0

#include <Audio.h>

/*
#define WAVEFORM_SINE              0
#define WAVEFORM_SAWTOOTH          1
#define WAVEFORM_SQUARE            2
#define WAVEFORM_TRIANGLE          3
#define WAVEFORM_ARBITRARY         4
#define WAVEFORM_PULSE             5
#define WAVEFORM_SAWTOOTH_REVERSE  6
#define WAVEFORM_SAMPLE_HOLD       7
#define WAVEFORM_TRIANGLE_VARIABLE 8
*/

typedef enum 
{
  SINE = WAVEFORM_SINE,
  TRIANGLE = WAVEFORM_TRIANGLE,
  VAR_TRIANGLE = WAVEFORM_TRIANGLE_VARIABLE,
  SQUARE = WAVEFORM_SQUARE,
  SAWTOOTH = WAVEFORM_SAWTOOTH,
  REV_SAWTOOTH = WAVEFORM_SAWTOOTH_REVERSE,
  PULSE = WAVEFORM_PULSE,
  S_AND_H = WAVEFORM_SAMPLE_HOLD
} waveshape;
  
typedef int ccInt;  // for 0 to 127 control values

typedef struct
{
  float freq; // this is a fractional MIDI note value
  waveshape shape;
  ccInt pwamt;
  bool pwm;
  ccInt modamt;
  int subfreq;  // divisor from 2 to 16
  waveshape subshape;
  ccInt detune; // support +/- semitone
} oscillatorCfg;

typedef struct
{
  ccInt attack;
  ccInt decay;
} envelopeCfg;

typedef struct
{
  waveshape shape;
  ccInt rate;
} modulatorCfg;

typedef enum
{
  FREE,
  CHROMATIC,
  DIATONIC,
  DIATONIC_M,
  PENTATONIC,
  PENTATONIC_M,
  PYTHAGOREAN,
  PYTHAGOREAN_M,
  JUST,
  JUST_M
} quantisation;

typedef struct
{
  quantisation scale;
  int transpose;  // -12 to +12
} tuningCfg;

typedef struct
{
  ccInt osclevel[2];
  bool oscmute[2];
  ccInt sublevel[2];
  bool submute[2];
} mixerCfg;

typedef struct
{
  ccInt cutoff;
  ccInt resonance;
  ccInt envamt;   // from -64 to 63
  ccInt modamt;
} filterCfg;

typedef struct
{
   int steps[4]; // -MAX_STEP to MAX_STEP
   bool applyOsc;
   bool applySub;
   bool advBeat[3];
   bool advOffbeat;
} sequencerCfg;

typedef struct
{
  ccInt tempo;
  int r1[3];
  int r2[3];
  int r3[3];
} rhythmCfg;

typedef enum
{
  NONE,
  CHORUS,
  FLANGE,
  DELAY,
  CHORUS_DELAY
} effect;

typedef enum
{
  BYPASS,
  SLOW,
  MEDIUM,
  FAST
} effect_preset;

typedef struct
{
  effect active;
  effect_preset chorus; // use preset index - 0 means off
  effect_preset flange;
  effect_preset delay;
} effectsCfg;

typedef enum
{
  BT_OFF,
  BT_SEL,
  BT_ONE,
  BT_ALL,
  BT_NONE
} beatState;

typedef struct
{
  beatState beatstate;
  beatState offbeatstate;
  int beats[4];
  char sampleset[MAX_SAMPLESET_NAME_LENGTH + 1];
} drumCfg;

typedef struct
{
  oscillatorCfg osc[2]; 
  mixerCfg      mix;
  envelopeCfg   env;
  modulatorCfg  lfo;
  filterCfg     filter;
  tuningCfg     tuning;
  effectsCfg    eff;
  sequencerCfg  seq[2];
  rhythmCfg     rhythm;
  drumCfg       drum;
} euclid_config, *config_ptr;

extern config_ptr config;

extern bool useMIDIClock;

extern bool needsKitUpdate;
extern bool needsOffbeatUpdate;

void setDefaultConfig();

waveshape getNextWaveshape(bool osc, bool sub, bool lfo, int i, bool back);
const char *waveshapeStr(waveshape w, bool full);
const char *quantisationStr(quantisation q, bool full);
const char *effectStr(effect e);
const char *effectPresetStr(effect_preset p);
const char *beatstateStr(beatState state);

void printOscConfig(int i, oscillatorCfg *cfg);
void printMixConfig(mixerCfg *cfg);
void printEnvelopeConfig(envelopeCfg *cfg);
void printModulatorConfig(modulatorCfg *cfg);
void printFilterConfig(filterCfg *cfg);
void printTuningConfig(tuningCfg *cfg);
void printSequencerConfig(int i, sequencerCfg *cfg);
void printRhythmConfig(rhythmCfg *cfg);
void printEffectsConfig(effectsCfg *cfg);
void printDrumConfig(drumCfg *cfg);
void printConfig(config_ptr cfg);
