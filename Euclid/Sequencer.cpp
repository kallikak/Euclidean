#include <stddef.h>
#include <math.h>
#include "Arduino.h"

#include "Config.h"
#include "Sequencer.h"
#include "Synth.h"
#include "Controls.h"
#include "Drums.h"

#define PERMUTE_LOOP_COUNT 8

static volatile bool fire = false;
void callback()
{
  fire = true;
}

void extClockFire()
{
  fire = true;
}

Polyrhythm::Polyrhythm()
{
  running = false;
  pause = false;
  i = 0;
  for (int j = 0; j < 3; ++j)
  {
    pattern[j].beats = NULL;
    pattern[j].length = 0;
  }
  onestep = false;
  oneshotfn = NULL;
}

Polyrhythm::~Polyrhythm()
{
  cleanupPatterns();
}

void Polyrhythm::cleanupPatterns()
{
  for (int j = 0; j < 3; ++j)
  {
    if (pattern[j].beats)
      delete pattern[j].beats;
    pattern[j].beats = NULL;
    pattern[j].length = 0;
  }
}
 
void Polyrhythm::generate(rhythmCfg *rs)
{
  // euclid = (m, n) => new Array(n).fill().map((e,i) => (m * i) % n < m ? 'x' : '.').join(" ")
  // euclid(3, 8)
  // "x . . x . . x ."
  int ms[3] = {rs->r1[0], rs->r2[0], rs->r3[0]};
  int ns[3] = {rs->r1[1], rs->r2[1], rs->r3[1]};
  int os[3] = {rs->r1[2], rs->r2[2], rs->r3[2]};
  int j, k;
  int m, n;
  cleanupPatterns();
  for (j = 0; j < 3; ++j)
  {
    m = ms[j];
    n = ns[j];
    pattern[j].beats = new int[n];
    pattern[j].length = n;
    for (k = 0; k < n; ++k)
    {
      pattern[j].beats[(k + os[j]) % n] = (m * k) % n < m ? 1 : 0;
    }
#if DEBUG_SERIAL
    Serial.println(poly->getRhythmString(0));
    Serial.println(poly->getRhythmString(1));
    Serial.println(poly->getRhythmString(2));
#endif    
  }
}

char *Polyrhythm::getRhythmString(int r)
{
  static char tempStr[100] = {0};
  int n = pattern[r].length;
  sprintf(tempStr, "R%d:", r);
  for (int k = 0; k < n; ++k)
    sprintf(tempStr, "%s%c", tempStr, pattern[r].beats[k] ? 'X' : '-');
  return tempStr;
}

beatpattern Polyrhythm::getPattern(int r)
{
  return pattern[r];
}

int next[3 * 32] = {0};
int *Polyrhythm::getNext(int steps)
{  
  steps = min(32, steps);
  int m = 0;
  for (int j = 0; j < 3; ++j)
  {
    for (int n = 0; n < steps; ++n)
    {
      int k = (i + n) % pattern[j].length;
      next[m] = pattern[j].beats[k];
      if (next[m] && k == 0)
        next[m]++;
      else if (k == 0)
        next[m]--;  
      m++;
    }
  }
  return next;
}

int Polyrhythm::tempoToDelay(int tempo)
{
//  ?+ [0:127:(127/15)]{x~_;]Int x,"\t",Int (2000 * E^(-x * 3.52 / 100))}
// ?+ [0:127:(127/15)]{x~_;]Int x,"\t",Int (1000 * E^(-x * 2.933 / 100))}
//    var d = 1095 * Math.exp(-0.02097 * tempo) - 50;
//    int d = max(30, min(2000, 2050 * exp(-tempo * 4.4067 / 128)));
    int d = max(50, min(2000, 2050 * exp(-tempo * 3.754 / 128)));
#if DEBUG_SERIAL
    Serial.print("Tempo is ");Serial.print(tempo);Serial.print(", Delay is ");Serial.print(d);Serial.print(", bpm is ");
      Serial.println((int)round(60000.0 / d));
#endif      
    return d;
}

void Polyrhythm::run(rhythmcallbacktype onbeat, rhythmcallbacktype onoffbeat, int tempo)
{
  stop();
  timer.end();

  pause = false;
  running = true;    
  onestep = false;
  i = 0;
  delay = tempoToDelay(tempo);
  beatcallback = onbeat;
  offbeatcallback = onoffbeat;
  if (!useMIDIClock)
    timer.begin(callback, delay * 1000);
#if DEBUG_SERIAL
  Serial.print("Setting timer delay ");
  Serial.print(delay);
  Serial.println("ms");
#endif
  fire = true;
//  handleCallbacks();
}

void Polyrhythm::setoneshotfn(oneshotfntype fn)
{
  oneshotfn = fn;
}

//static unsigned long last = 0;
void Polyrhythm::handleCallbacks()
{ 
  if (!running || !fire)
    return;
//  noInterrupts();
  fire = false;
//  interrupts();

  if (oneshotfn)
  {
    // protect against recursion
    oneshotfntype savefn = oneshotfn;
    oneshotfn = NULL;
    savefn();
  }

  seqadv dooneseqadv = {false, false};
  bool beat = false;
  int beats[] = {0, 0, 0};
  for (int j = 0; j < 3; ++j)
  {
    int k = i % pattern[j].length;
    beats[j] = pattern[j].beats[k];
    if (beats[j] && k == 0)
      beats[j]++;
    if (beats[j])
    {
//      if (!beat)
      {
        dooneseqadv = beatcallback(j, dooneseqadv);
        beat = dooneseqadv.seq1adv || dooneseqadv.seq2adv;
      }
    } 
  }
  if (!beat)
  {
    dooneseqadv = offbeatcallback(0, dooneseqadv);
  }
  
  notifybeats(beats[0], beats[1], beats[2], !beat);
  
  bool didadv = dooneseqadv.seq1adv || dooneseqadv.seq2adv;
  if (onestep && !didadv) 
    i++;
  else if (!pause) 
    ++i;
  onestep = !(dooneseqadv.seq1adv || dooneseqadv.seq2adv);
}

void Polyrhythm::stop()
{
  timer.end();
  running = false;  
  pause = false;
}

void Polyrhythm::doonestep()
{
  onestep = true;
  i++;
  timer.update(1000000);
}

void Polyrhythm::regenerate(rhythmCfg *rs)
{
  generate(rs);
  setTempo(rs->tempo);
#if DEBUG_SERIAL
  Serial.print("tempo ");
  Serial.print(rs->tempo);
  Serial.print(" to delay ");
  Serial.println(delay);
#endif  
}

void Polyrhythm::setTempo(int tempo)
{
  delay = tempoToDelay(tempo);  
  timer.update(delay * 1000);
  if (config->eff.active == DELAY || config->eff.active == CHORUS_DELAY)
  {
    if (config->eff.delay != BYPASS)
      synth->loadEffectsConfig(&config->eff);
  }
}

int Polyrhythm::getDelayMillis()
{
  return delay; 
}
    
int Polyrhythm::getBPM()
{
  // delay in ms => bpm = 60000/delay
  return round(60000.0 / delay);
}

Sequencer::Sequencer(int n)
{
  seqnum = n;
  pause = false;
  seq1 = seqnum == 1;
  step = -1;
  loop = seq1 ? 0 : PERMUTE_LOOP_COUNT / 2;
  permute = false;
  onestep = false;
  curstepval = 0;
  memset(mysteps, 0, 4 * sizeof(int));
}

void Sequencer::setPermute(bool onoff)
{
  permute = onoff;
}

void Sequencer::togglePermute()
{
  permute = !permute;
}

bool Sequencer::isPermuting()
{
  return permute;
}

void Sequencer::update(bool resetpermute)
{
  if (resetpermute)
    permute = false;
  seqcfg = config->seq[seqnum - 1];
  memcpy(mysteps, seqcfg.steps, 4 * sizeof(int));
  updateOneSeqStepsDisplay(seqnum - 1, mysteps);
#if DEBUG_SERIAL
  int k;
  Serial.print("Update sequencer ");Serial.println(seqnum);
  for (k = 0; k < 4; ++k)
  {
    Serial.print("\tstep ");Serial.print(k + 1);Serial.print(": ");Serial.print(mysteps[k]);
  }
  Serial.println();
  Serial.print("\tapplyOsc ");Serial.print(seqcfg.applyOsc);
  Serial.print("\tapplySub ");Serial.print(seqcfg.applySub);
  Serial.println();
  for (k = 0; k < 3; ++k)
  {
    Serial.print("\tadvBeat ");Serial.print(k + 1);Serial.print(": ");Serial.print(seqcfg.advBeat[k]);
  }
  Serial.print("\tadvOffbeat ");Serial.print(seqcfg.advOffbeat);
  Serial.println();
#endif  
}

void Sequencer::allLedsOff()
{
  for (int k = 0; k < 4; ++k)
    setSequenceLed(seqnum, k, false);
}

void Sequencer::ledOn(int led)
{
  allLedsOff();
  if (isActive() || synth->droning || poly->pause)
    setSequenceLed(seqnum, led, true);
}

int Sequencer::next()
{
  if (onestep || !pause || step < 0) 
  {
    ++step;
  }
  onestep = false;
  if (step >= 4)
  {
    step %= 4;
    if (permute)
    {
      loop++;
      if (loop == PERMUTE_LOOP_COUNT)
      {
        loop = 0;
        permuteSteps();
        updateOneSeqStepsDisplay(seqnum - 1, mysteps);
      }
    }
  }
  ledOn(step);
  curstepval = mysteps[step];
#if DEBUG_SERIAL
  Serial.print("Sequencer ");
  Serial.print(seqnum);
  Serial.print(" loop ");
  Serial.print(loop + 1);
  Serial.print(" step ");
  Serial.print(step + 1);
  Serial.print(": ");
  Serial.println(curstepval);
#endif  
  return curstepval;
}

int Sequencer::getCurStep()
{
  return curstepval;
}

int Sequencer::getCurStepIndex()
{
  return step;
}

void Sequencer::doonestep()
{
  onestep = true;
}

bool Sequencer::isActive()
{
  return seqcfg.applyOsc || seqcfg.applySub;
}

void Sequencer::reset()
{
  step = -1;
  loop = 0;
  onestep = false;
  pause = false;
}

void Sequencer::permuteSteps()
{
  int i = 3;
  int j = 3;
  int k = 3;
  int temp;
  int *a = mysteps;
  
  while (i > 0 && a[i - 1] >= a[i]) 
    i--;

  if (i > 0) 
  {
    while (a[j] <= a[i - 1]) 
      j--;

    temp = a[i - 1];
    a[i - 1] = a[j];
    a[j] = temp;
  }

  while (i < k) 
  {
    temp = a[i];
    a[i++] = a[k];
    a[k--] = temp;
  }
#if DEBUG_SERIAL
  Serial.print("New permutation for sequencer ");
  Serial.print(seqnum);
  Serial.print(": ");
  for (i = 0; i < 4; ++i)
  {
    Serial.print(a[i]);
    Serial.print(" ");
  }
  Serial.println();
#endif  
}
