#include <math.h>
#include <stddef.h>

#include "MIDIManager.h"

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include "Config.h"
#include "Utility.h"
#include "Sequencer.h"
#include "Synth.h"
#include "Presets.h"
#include "Controls.h"
#include "LCD.h"
#include "Drums.h"

// GUItool: begin automatically generated code
AudioSynthWaveform       LFO;            //xy=1012,692
AudioSynthWaveformDc     pwmamt2;        //xy=1047,892
AudioSynthWaveformDc     pwmamt1;        //xy=1059,509
AudioSynthWaveformDc     modamt1;        //xy=1125,416
AudioEffectMultiply      multiply3;      //xy=1156,796
AudioEffectMultiply      multiply1;      //xy=1191,589
AudioSynthWaveformDc     modamt2;        //xy=1201,654
AudioEffectMultiply      multiply2;      //xy=1306,432
AudioAnalyzePeak         peak2;          //xy=1308,794
AudioEffectMultiply      multiply4;      //xy=1312,706
AudioMixer4              pwmix2;         //xy=1312,867
AudioAnalyzePeak         peak1;          //xy=1351,551
AudioMixer4              pwmix1;         //xy=1379,631
AudioSynthWaveformDc     fenvamt;        //xy=1420,994
AudioSynthWaveformDc     fmodamt;        //xy=1424,934
AudioSynthWaveform       sub2a;          //xy=1477,683
AudioSynthWaveform       sub2b;          //xy=1478,722
AudioSynthWaveformModulated osc2b;          //xy=1482,796
AudioSynthWaveformModulated osc2a;          //xy=1485,757
AudioSynthWaveform       sub1b;          //xy=1503,499
AudioSynthWaveform       sub1a;          //xy=1505,451
AudioSynthWaveformModulated osc1a;          //xy=1510,552
AudioSynthWaveformModulated osc1b;          //xy=1525,599
AudioEffectEnvelope      fenv;           //xy=1624,927
AudioEffectMultiply      multiply5;      //xy=1627,872
AudioMixer4              o2mix;          //xy=1631,753
AudioMixer4              o1mix;          //xy=1684,590
AudioEffectEnvelope      o2env;          //xy=1784,751
AudioMixer4              fmix;           //xy=1798,882
AudioPlayMemory          sound1;         //xy=1801,378
AudioPlayMemory          sound2;         //xy=1805,418
AudioPlayMemory          sound3;         //xy=1805,458
AudioPlayMemory          sound4;         //xy=1807,496
AudioEffectEnvelope      o1env;          //xy=1814,602
AudioMixer4              delaymix;       //xy=1825,1002
AudioEffectDelay         delayeff;       //xy=1834,1161
AudioMixer4              oscmix;         //xy=1945,753
AudioFilterStateVariable filter;         //xy=1953,872
AudioMixer4              drummix;        //xy=2015,450
AudioEffectFlange        flanger;        //xy=2083,1083
AudioEffectEnsemble      chorus;         //xy=2092,973
AudioAmplifier           filterpreamp;   //xy=2104,766
AudioAmplifier           drumfilterpreamp; //xy=2180.555419921875,687.7777709960938
AudioMixer4              effectmixL;     //xy=2201,883
AudioMixer4              effectmixR;     //xy=2288,1060
AudioFilterStateVariable drumfilter; //xy=2324.777587890625,767.1110229492188
AudioMixer4              finalmixL; //xy=2406,887
AudioMixer4              finalmixR; //xy=2431,981
AudioAmplifier           levelL;         //xy=2505,812
AudioAmplifier           levelR;         //xy=2619,920
AudioOutputUSB           usb1;           //xy=2640,757
AudioOutputI2S           i2s1;           //xy=2658,800
AudioConnection          patchCord1(LFO, 0, multiply2, 1);
AudioConnection          patchCord2(LFO, 0, multiply1, 1);
AudioConnection          patchCord3(LFO, 0, multiply3, 0);
AudioConnection          patchCord4(LFO, 0, multiply4, 1);
AudioConnection          patchCord5(LFO, 0, multiply5, 0);
AudioConnection          patchCord6(pwmamt2, 0, pwmix2, 1);
AudioConnection          patchCord7(pwmamt2, 0, multiply3, 1);
AudioConnection          patchCord8(pwmamt1, 0, multiply1, 0);
AudioConnection          patchCord9(pwmamt1, 0, pwmix1, 0);
AudioConnection          patchCord10(modamt1, 0, multiply2, 0);
AudioConnection          patchCord11(multiply3, 0, pwmix2, 0);
AudioConnection          patchCord12(multiply3, peak2);
AudioConnection          patchCord13(multiply1, 0, pwmix1, 1);
AudioConnection          patchCord14(multiply1, peak1);
AudioConnection          patchCord15(modamt2, 0, multiply4, 0);
AudioConnection          patchCord16(multiply2, 0, osc1a, 0);
AudioConnection          patchCord17(multiply2, 0, osc1b, 0);
AudioConnection          patchCord18(multiply4, 0, osc2a, 0);
AudioConnection          patchCord19(multiply4, 0, osc2b, 0);
AudioConnection          patchCord20(pwmix2, 0, osc2a, 1);
AudioConnection          patchCord21(pwmix2, 0, osc2b, 1);
AudioConnection          patchCord22(pwmix1, 0, osc1a, 1);
AudioConnection          patchCord23(pwmix1, 0, osc1b, 1);
AudioConnection          patchCord24(fenvamt, fenv);
AudioConnection          patchCord25(fmodamt, 0, multiply5, 1);
AudioConnection          patchCord26(sub2a, 0, o2mix, 0);
AudioConnection          patchCord27(sub2b, 0, o2mix, 2);
AudioConnection          patchCord28(osc2b, 0, o2mix, 3);
AudioConnection          patchCord29(osc2a, 0, o2mix, 1);
AudioConnection          patchCord30(sub1b, 0, o1mix, 2);
AudioConnection          patchCord31(sub1a, 0, o1mix, 0);
AudioConnection          patchCord32(osc1a, 0, o1mix, 1);
AudioConnection          patchCord33(osc1b, 0, o1mix, 3);
AudioConnection          patchCord34(fenv, 0, fmix, 1);
AudioConnection          patchCord35(multiply5, 0, fmix, 0);
AudioConnection          patchCord36(o2mix, o2env);
AudioConnection          patchCord37(o1mix, o1env);
AudioConnection          patchCord38(o2env, 0, oscmix, 1);
AudioConnection          patchCord39(fmix, 0, filter, 1);
AudioConnection          patchCord40(fmix, 0, drumfilter, 1);
AudioConnection          patchCord41(sound1, 0, drummix, 0);
AudioConnection          patchCord42(sound2, 0, drummix, 1);
AudioConnection          patchCord43(sound3, 0, drummix, 2);
AudioConnection          patchCord44(sound4, 0, drummix, 3);
AudioConnection          patchCord45(o1env, 0, oscmix, 0);
AudioConnection          patchCord46(delaymix, delayeff);
AudioConnection          patchCord47(delaymix, 0, effectmixL, 2);
AudioConnection          patchCord48(delaymix, 0, effectmixR, 2);
AudioConnection          patchCord49(delayeff, 0, delaymix, 1);
AudioConnection          patchCord50(oscmix, filterpreamp);
AudioConnection          patchCord51(filter, 0, chorus, 0);
AudioConnection          patchCord52(filter, 0, delaymix, 0);
AudioConnection          patchCord53(filter, 0, effectmixL, 0);
AudioConnection          patchCord54(filter, 0, effectmixR, 0);
AudioConnection          patchCord55(filter, 0, flanger, 0);
AudioConnection          patchCord56(drummix, drumfilterpreamp);
AudioConnection          patchCord57(flanger, 0, effectmixL, 3);
AudioConnection          patchCord58(flanger, 0, effectmixR, 3);
AudioConnection          patchCord59(chorus, 0, effectmixL, 1);
AudioConnection          patchCord60(chorus, 1, effectmixR, 1);
AudioConnection          patchCord61(filterpreamp, 0, filter, 0);
AudioConnection          patchCord62(drumfilterpreamp, 0, drumfilter, 0);
AudioConnection          patchCord63(effectmixL, 0, finalmixL, 0);
AudioConnection          patchCord64(effectmixR, 0, finalmixR, 0);
AudioConnection          patchCord65(drumfilter, 0, finalmixL, 1);
AudioConnection          patchCord66(drumfilter, 0, finalmixR, 1);
AudioConnection          patchCord67(finalmixL, levelL);
AudioConnection          patchCord68(finalmixR, levelR);
AudioConnection          patchCord69(levelL, 0, i2s1, 0);
AudioConnection          patchCord70(levelL, 0, usb1, 0);
AudioConnection          patchCord71(levelR, 0, i2s1, 1);
AudioConnection          patchCord72(levelR, 0, usb1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=2566,384
// GUItool: end automatically generated code

bool off = true;

void setupSynth() 
{
  // the final osc mixer
  oscmix.gain(0, 0.2);  
  oscmix.gain(1, 0.2);  
  oscmix.gain(2, 0.5);  
  
  // and the filter values
  // and volume
  levelL.gain(0);
  levelR.gain(0);

  // effects
  effectmixL.gain(0, 1);
  effectmixL.gain(1, 0); // chorusL
  effectmixL.gain(2, 0); // delay
  
  effectmixR.gain(0, 1);
  effectmixR.gain(1, 0); // chorusR
  effectmixR.gain(2, 0); // delay
  
  AudioMemory(500);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.7);
  sgtl5000_1.muteLineout();
  sgtl5000_1.enhanceBassEnable();
  sgtl5000_1.autoVolumeControl(0,2,0,-18.0,200,2000);  //maxGain,response,hard limit,threshold,attack, decay
}

Polyrhythm *poly = new Polyrhythm();
Sequencer *seq1 = new Sequencer(1);
Sequencer *seq2 = new Sequencer(2);
Synth *synth = NULL;

seqadv beatcallback(int index, seqadv seqadvance);
seqadv offbeatcallback(int index, seqadv seqadvance);

int loopcount = 0;

Synth *makeSynth()
{
  Filter *f = new Filter(&filter, &drumfilter, &fenvamt, &fmodamt, &fenv, &fmix, &filterpreamp, &drumfilterpreamp);
  Envelope *e1 = new Envelope(&o1env);
  Envelope *e2 = new Envelope(&o2env);
  Oscillator *o1 = new Oscillator(0, &osc1a, &sub1a, &osc1b, &sub1b, &o1mix, &modamt1, &pwmamt1, &pwmix1);
  Oscillator *o2 = new Oscillator(1, &osc2a, &sub2a, &osc2b, &sub2b, &o2mix, &modamt2, &pwmamt2, &pwmix2);
  Modulator *m = new Modulator(&LFO);
  Effects *e = new Effects(&delayeff, &chorus, &flanger, &delaymix, &effectmixL, &effectmixR);
  
  return new Synth(f, e1, e2, o1, o2, m, e, &levelL, &levelR);
}

void switchOff()
{
  if (off)
    return;
  // minimise transients
  for (int i = 5; i >= 0; --i)
  {
    finalmixL.gain(0, i / 100.0);
    finalmixL.gain(1, i / 100.0);
    finalmixR.gain(0, i / 100.0);
    finalmixR.gain(1, i / 100.0);
    delay(2);
  }
  off = true;
}

void switchOn()
{
  if (off)
  {
    finalmixL.gain(0, 1);
    finalmixL.gain(1, 1);
    finalmixR.gain(0, 1);
    finalmixR.gain(1, 1);
    off = false;
  }
}

bool isOff()
{
  return off;
}

seqadv advsequencer(bool seq1adv, bool seq2adv, seqadv doneseqadv, bool wasbeat)
{
  bool o1step = false;
  bool s1step = false;
  bool o2step = false;
  bool s2step = false;
  
  int step1 = -500;    // -24 to +24 semitones (later apply quantisation)
  int step2 = -500;    // -24 to +24 semitones (later apply quantisation)
  int substep1 = -500; // -8 to +8 for subfrequencies
  int substep2 = -500; // -8 to +8 for subfrequencies
  
  if (seq1adv)
  {
    step1 = !doneseqadv.seq1adv ? seq1->next() : seq1->getCurStep();
    doneseqadv.seq1adv = true;
    substep1 = floor(-step1 / 3.0);
#if DEBUG_SERIAL
    Serial.print("\tCalculated step1 ");Serial.print(step1);Serial.print(" and substep1 ");Serial.println(substep1);
#endif    
  }
  if (seq2adv)
  {
    step2 = !doneseqadv.seq2adv ? seq2->next() : seq2->getCurStep();
    doneseqadv.seq2adv = true;
    substep2 = floor(-step2 / 3.0);
#if DEBUG_SERIAL
    Serial.print("\tCalculated step2 ");Serial.print(step2);Serial.print(" and substep2 ");Serial.println(substep2);
#endif    
  }
  
  if (seq1adv)
  {
    o1step = config->seq[0].applyOsc;
    s1step = config->seq[0].applySub;
  }
  
  if (seq2adv)
  {
    o2step = config->seq[1].applyOsc;
    s2step = config->seq[1].applySub;
  }

  if (o1step || s1step)
  {
#if DEBUG_SERIAL    
    Serial.print("Stepping O1 - ");
    if (o1step)
    {
      Serial.print("\n\tFreq step ");Serial.print(step1);
      Serial.print(" => freq offset ");Serial.print(offsetToFreq(step1, noteToFreq(config->osc[0].freq)));
      Serial.print(" from base ");Serial.print(noteToFreq(config->osc[0].freq));
    }
    if (s1step)
    {
      Serial.print("\n\tSub step ");Serial.print(substep1);
    }
    Serial.println();
#endif    
    synth->offset(o1step ? step1 : 0, s1step ? substep1 : 0, o1step, s1step, 0);
  }

  if (o2step || s2step)
  {
#if DEBUG_SERIAL    
    Serial.print("Stepping O2 - ");
    if (o2step)
    {
      Serial.print("\n\tFreq step ");Serial.print(step2);
      Serial.print(" => freq offset ");Serial.print(offsetToFreq(step2, noteToFreq(config->osc[1].freq)));
      Serial.print(" from base ");Serial.print(noteToFreq(config->osc[1].freq));
    }
    if (s2step)
    {
      Serial.print("\n\tSub step ");Serial.print(substep2);
    }
    Serial.println();
#endif    
    synth->offset(o2step ? step2 : 0, s2step ? substep2 : 0, o2step, s2step, 1);
  }
  
  if (!synth->droning && poly->pause)
    synth->retrigger(!(o1step || s1step), !(o2step || s2step));
  
  return doneseqadv;
}

seqadv beatcallback(int n, seqadv doneseqadv)
{
#if DEBUG_SERIAL
  Serial.print("Beat ");
  Serial.print(n);
  Serial.print(", advseq1 ");
  Serial.print(config->seq[0].advBeat[n]);
  Serial.print(", advseq2 ");
  Serial.print(config->seq[1].advBeat[n]);
  Serial.print(", doneadvseq1 ");
  Serial.print(doneseqadv.seq1adv);
  Serial.print(", doneadvseq2 ");
  Serial.print(doneseqadv.seq2adv);
  Serial.println();
#endif  
  return advsequencer(config->seq[0].advBeat[n], config->seq[1].advBeat[n], doneseqadv, true);
}

seqadv offbeatcallback(int ignored, seqadv doneseqadv)
{
#if DEBUG_SERIAL
  Serial.print("Offbeat:");
  Serial.print(" advseq1 ");
  Serial.print(config->seq[0].advOffbeat);
  Serial.print(", advseq2 ");
  Serial.print(config->seq[1].advOffbeat);
  Serial.print(", doneadvseq1 ");
  Serial.print(doneseqadv.seq1adv);
  Serial.print(", doneadvseq2 ");
  Serial.print(doneseqadv.seq2adv);
  Serial.println();
#endif  
  return advsequencer(config->seq[0].advOffbeat, config->seq[1].advOffbeat, doneseqadv, false);
}

bool checkBeat(Beat beat, int beat1, beatState state)
{
  switch (state)
  {
    case BT_ALL:
      return true;
    case BT_NONE:
      return false;
    case BT_ONE:
      return beat == BEAT1 && beat1;
    case BT_SEL:
      if (beat == BEAT1)
        return config->seq[0].advBeat[0] || config->seq[1].advBeat[0];
      else if (beat == BEAT2)
        return config->seq[0].advBeat[1] || config->seq[1].advBeat[1];
      else if (beat == BEAT3)
        return config->seq[0].advBeat[2] || config->seq[1].advBeat[2];
      return false;
    case BT_OFF:
    default:
      return false;
  }
}

void notifybeats(int beat1, int beat2, int beat3, bool offbeat)
{
  showNextBeats(poly->getNext(10), 10);
  
  if (beat1 && isDrum(BEAT1))
  {
    if (checkBeat(BEAT1, beat1, config->drum.beatstate))
    {
      playDrumSample(BEAT1, beat1 > 1); 
    }
  }
  if (beat2 && isDrum(BEAT2))
  {
    if (checkBeat(BEAT2, beat1, config->drum.beatstate))
    {
      playDrumSample(BEAT2, beat1 > 1); 
    }
  }
  if (beat3 && isDrum(BEAT3))
  {
    if (checkBeat(BEAT3, beat1, config->drum.beatstate))
    {
      playDrumSample(BEAT3, beat1 > 1); 
    }
  }

  if (isDrum(OFFBEAT))
  {
    bool play = false;
    switch (config->drum.offbeatstate)
    {
      case BT_ONE:
        play = !beat1;
        break;
      case BT_SEL:
        play = offbeat && (config->seq[0].advOffbeat || config->seq[1].advOffbeat);
        break;
      case BT_ALL:
        play = !beat1 && !beat2 && !beat3;
        break;
      case BT_NONE:
        play = true;
        break;
      case BT_OFF:
      default:
        break;
    }
    if (play)
    {
      playDrumSample(OFFBEAT, false); 
    }
  }
}

void setup(void) 
{
  //pinMode(SD_CS, INPUT_PULLUP);  // don"t touch the SD card
  Serial.begin(9600);
  Serial.println("Welcome to Euclid");
  
  Wire.setClock(1000000);
  
  setupSynth();
  synth = makeSynth();
  synth->loadConfig(config);

  setupLCD();
  setupControls();
  setupMIDI();
  setupPresets();
  managePlayState();
  polySetup();
  poly->stop();
  synth->stop();
  initDrums(&drummix, &sound1, &sound2, &sound3, &sound4);
  setInitialVolumes();
  levelL.gain(1);
  levelR.gain(1);
  sgtl5000_1.unmuteLineout();
  Serial.println("Setup complete");
}

static ccInt lastpeak1 = 0;
static ccInt lastpeak2 = 0;

void loop() 
{
  checkMIDI();
  checkControls(loopcount++);
  // hacked pulse width modulation
  if (loopcount % 20 == 0)
  {
    if (config->osc[0].pwm && config->osc[0].shape == PULSE && peak1.available())
    {
      ccInt v = 127 * peak1.read();
      if (lastpeak1 != v)
      {
        lastpeak1 = v;
        synth->modPWAmount(v, 0);
      }
    }
    if (config->osc[1].pwm && config->osc[1].shape == PULSE && peak2.available())
    {
      ccInt v = 127 * peak2.read();
      if (lastpeak2 != v)
      {
        lastpeak2 = v;
        synth->modPWAmount(v, 1);
      }
    }
  }

  poly->handleCallbacks();
  checkSerialControl();
}


/*
 * - Drop in gain from effects (especially chorus)
 * - Consider some gain increase inverse to cutoff frequency
 * - Add support for MIDI CCs?
 */
