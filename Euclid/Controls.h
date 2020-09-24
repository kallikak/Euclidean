#pragma once

#include "sequencer.h"
#include "Synth.h"

#define NUM_SELECT  6
class BounceMcp;

typedef struct
{
  int expander;
  int pinA;
  int pinB;
  int pinSW;
  uint16_t mask;
  bool halfstate;
  uint8_t _state;
  int minval[NUM_SELECT];
  int maxval[NUM_SELECT];
  BounceMcp *debouncer;
} encoder;

enum loadadjust
{
  NO_ADJUST = 0,
  BELOW,
  ABOVE
};

typedef struct
{
  int pin;
  long value;
  long avgrawvalue;
  int loadedvalue;
  bool signedrange;
  enum loadadjust adjust;
  unsigned long lastadjustmillis;
  bool lastadjustup;
} pot;

#define STEP 10
#define HOLD 11
#define STOP 12
#define RUN 13

void setSequenceLed(int seq, int led, bool onoff);

void polyRestart();
void polyResume();
void polySetup();

void setInitialVolumes();
void setupControls();
void checkControls(int loopcount);
bool isDemoMode();

extern Polyrhythm *poly;
extern Sequencer *seq1;
extern Sequencer *seq2;
extern Synth *synth;

void setSelectLed(int led);
void showSelectSummary(int selected);
void showSequencerState();
void updateStepsDisplay();
void showDroningFrequencies();
void showPauseInfo();
void buttonPressed(int j, int e);
void handleButton(int i);
void handleEncoderTurn(int i, int d);
void handleEncoderPress(int i);
void handlePot(int i, int value);
void setStepValue(int seqnum, int i, int value);
void updateActiveMeter(int d);
bool loadPreset(int preset);
void manageLoadedValue(pot *p, int v, bool isstep);
void updateStepsDisplay();
void updateOneSeqStepsDisplay(int i, int *steps);

extern int lastupdatemillis;
extern int selected;

seqadv beatcallback(int n, seqadv doneseqadv);
seqadv offbeatcallback(int ignored, seqadv doneseqadv);
void notifybeats(int beat1, int beat2, int beat3, bool offbeat);

void switchOn();
bool isOff();
void switchOff();

bool checkForBackupPrevent();

void checkSerialControl();
