#pragma once

#include <stddef.h>
#include <IntervalTimer.h>

#include "Config.h"

typedef enum
{
  BEAT1,
  BEAT2,
  BEAT3,
  OFFBEAT
} Beat;

typedef struct
{
  int *beats;
  int length;
} beatpattern;

typedef struct seqadv_s
{
  bool seq1adv;
  bool seq2adv;
} seqadv;

typedef seqadv (*rhythmcallbacktype)(int index, seqadv seqadvance);
typedef void (*oneshotfntype)(void);

void extClockFire();

class Polyrhythm
{
  private:
    int i = 0;
    beatpattern pattern[3];
    int delay;
    bool onestep = false;
    oneshotfntype oneshotfn = NULL;
    IntervalTimer timer;
    rhythmcallbacktype beatcallback;
    rhythmcallbacktype offbeatcallback;

    void cleanupPatterns();
    
  public:
    bool running;
    bool pause;

    Polyrhythm();
    ~Polyrhythm();
 
    void generate(rhythmCfg *rs);
    void run(rhythmcallbacktype beatcallback, rhythmcallbacktype offbeatcallback, int tempo);
    void handleCallbacks();
    void setoneshotfn(oneshotfntype fn);
    void stop();
    void doonestep();
    void regenerate(rhythmCfg *rs);
    void setTempo(int tempo);
    int tempoToDelay(int tempo);
    int getBPM();
    int getDelayMillis();
    beatpattern getPattern(int r);
    int *getNext(int steps);
    char *getRhythmString(int r);
};

class Sequencer
{
  private:
    int seqnum;
    bool seq1;
    int step; 
    int loop; 
    int curstepval;
    bool onestep;
    bool permute;
    int mysteps[4];
    
    void permuteSteps();
    
  public:
    bool pause;
    sequencerCfg seqcfg;
    
    Sequencer(int seqnum);

    void update(bool resetpermute);
    void allLedsOff();
    void ledOn(int led);
    int next();
    int getCurStepIndex();
    int getCurStep();
    void doonestep();
    void reset();
    bool isActive();
    void setPermute(bool onoff);
    void togglePermute();
    bool isPermuting();
};
