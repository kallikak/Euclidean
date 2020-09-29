#include <Audio.h>

#include <Arduino.h>

#include "Config.h"
#include "Sequencer.h"
#include "Synth.h"
#include "Utility.h"

#include "sawtoothWave.h"
#include "squareWave.h"
#include "triangleWave.h"

#define ATTACK_FACTOR 10
#define DECAY_FACTOR 4
#define RELEASE_FACTOR 12

#define MAX_FREQ 22050

// The filter

Filter::Filter(AudioFilterStateVariable* filter, AudioFilterStateVariable* drumfilter, AudioSynthWaveformDc* fenvamt, AudioSynthWaveformDc* fmodamt,
      AudioEffectEnvelope* fenv, AudioMixer4* fmix, AudioAmplifier* filterpreamp, AudioAmplifier* drumfilterpreamp)
{
  this->filter = filter;
  this->drumfilter = drumfilter;
  this->fenvamt = fenvamt;
  this->fmodamt = fmodamt;
  this->fenv = fenv;
  this->fmix = fmix;
  this->filterpreamp = filterpreamp;
  this->drumfilterpreamp = drumfilterpreamp;

  fenv->attack(40);
  fenv->decay(10000);
  fenv->sustain(0);
  
  // the filter mixer
  fmix->gain(0, 1);  
  fmix->gain(1, 1);  
  
  filterpreamp->gain(0.5);
  drumfilterpreamp->gain(0.25);
  
  // and the filter values
  co = 6000;
  q = 0.7;
  filter->octaveControl(4);
  filter->frequency(co);
  filter->resonance(q);
  drumfilter->octaveControl(4);
  drumfilter->frequency(co);
  drumfilter->resonance(q);
}

void Filter::loadConfig(filterCfg *fCfg, envelopeCfg *envCfg)
{
  if (fCfg)
  {
    setCutoff(fCfg->cutoff);
    setResonance(fCfg->resonance);
    setModAmount(fCfg->modamt);
    setEnvAmount(fCfg->envamt);
  }

  if (envCfg)
  {
    setAttack(envCfg->attack);
    setDecay(envCfg->decay);
  }
}

void Filter::setCutoff(ccInt u)
{
  // exp curve from about 25Hz to 6000 Hz
  co = 25 * exp(5.5 * u / 127.0);
#if DEBUG_SERIAL
Serial.print("Filter cutoff: ");Serial.print(u);Serial.print(" -> ");Serial.println(co);
#endif
  filter->frequency(co);
  filter->octaveControl(log2f(12000.0 / (float)co));
  drumfilter->frequency(co);
  drumfilter->octaveControl(log2f(12000.0 / (float)co));
// https://forum.pjrc.com/threads/57755-Noise-at-higher-filterfreqs  
//  That means that each of your myFilter.frequency(Fnew) function calls would immediately be followed by a 
// myFilter.octaveControl(log2f(10000.0/(float)Fnew)).
}

float Filter::getCutoffFreq()
{
  return co;
}

void Filter::setResonance(ccInt u)
{
#if DEBUG_SERIAL
  Serial.print("Resonance: ");Serial.print(u);Serial.print(" -> ");Serial.println(u * 4.3 / 127.0 + 0.7);
#endif  
  q = u * 4.3 / 127.0 + 0.7;
  filter->resonance(q);
  drumfilter->resonance(q);

  // exp curve from 0 to 30dB
//  filter.Q.setValueAtTime(Math.exp(3.43 * q) - 1, ctx.currentTime);
}

void Filter::setModAmount(ccInt u)
{
  fmodamt->amplitude(u / 127.0);
}

void Filter::setEnvAmount(ccInt u)
{
  fenvamt->amplitude(u / 64.0);
}

void Filter::setAttack(ccInt a)
{
  fenv->attack(a / 127.0 * 11880 / ATTACK_FACTOR);
}

void Filter::setDecay(ccInt d)
{
  fenv->decay(d / 127.0 * 11880 / DECAY_FACTOR);
}

void Filter::trigger()
{
  fenv->noteOn();
}

// The amplitude envelope

Envelope::Envelope(AudioEffectEnvelope *e)
{
  env = e;
  holdMode = false;
  env->delay(0.0);
  env->attack(400.0);
  env->hold(0.0);
  env->decay(10000.0);
  env->sustain(0.0);
  env->release(0.0);
}

void Envelope::loadConfig(envelopeCfg *cfg)
{
  holdMode = false;
  setAttack(cfg->attack);
  setDecay(cfg->decay);
}

void Envelope::setAttack(ccInt a)
{
  env->attack(a / 127.0 * 11880 / ATTACK_FACTOR);
}

void Envelope::setDecay(ccInt d)
{
  env->decay(d / 127.0 * 11880 / DECAY_FACTOR);
}

void Envelope::setRelease(ccInt d)
{
  env->release(d / 127.0 * 11880 / RELEASE_FACTOR);
}

void Envelope::setHoldMode(bool onoff)
{
  holdMode = onoff;
  if (holdMode)
  {
    env->attack(0.0);
    env->decay(0.0);
    env->sustain(1.0);
  }
  else
  {
    env->sustain(0.0);
    setAttack(config->env.attack);
    setDecay(config->env.decay);
  }
}

void Envelope::setASRMode(bool onoff)
{
  if (onoff)
  {
    if (holdMode)
      setHoldMode(false);
    // turn ASR on
    env->sustain(1.0);
//    setAttack(config->env.attack);
//    setDecay(config->env.decay);
    setRelease(config->env.decay);
  }
  else
  {
    // turn ASR off
    env->sustain(0.0);
    setRelease(0);
    if (holdMode)
      setHoldMode(true);
  }
}

bool Envelope::isHoldMode()
{
  return holdMode;
}

void Envelope::trigger()
{
  env->noteOn();
}

void Envelope::noteoff()
{
  env->noteOff();
}

#define OSC_CHANNEL_A 1
#define OSC_CHANNEL_B 3
#define SUB_CHANNEL_A 0
#define SUB_CHANNEL_B 2

#define MIX_DIV (5 * 127.0)

Oscillator::Oscillator(int i, AudioSynthWaveformModulated *o1, AudioSynthWaveform *s1, 
      AudioSynthWaveformModulated *o2, AudioSynthWaveform *s2, 
      AudioMixer4 *mix, AudioSynthWaveformDc *modamt, AudioSynthWaveformDc *pwamt, AudioMixer4 *pwmix)
{
  index = i;
  
  offsetSteps = 0;
  suboffsetSteps = 0;
  detune = 0;
  overrideTuning = false;

  this->oscA = o1;
  this->oscB = o2;
  this->subA = s1;
  this->subB = s2;
  this->mix = mix;
  this->modamt = modamt;
  this->pwamt = pwamt;
  this->pwmix = pwmix;
  
  // setup oscillators - max amplitude and support frequency modulation +/- 1 octave
  oscA->amplitude(1.0);
  oscA->frequencyModulation(1);
  subA->amplitude(1.0);
  oscB->amplitude(1.0);
  oscB->frequencyModulation(1);
  subB->amplitude(1.0);

  modamt->amplitude(0.0);
  pwamt->amplitude(0.0);
  
  pwmix->gain(0, 0);  
  pwmix->gain(1, 0);
  // the individual osc/sub mixer
  mix->gain(0, 0.2);  
  mix->gain(1, 0.2);
}

void Oscillator::loadConfig(oscillatorCfg *cfg, mixerCfg *mixCfg)
{
  if (cfg)
  {
    if (cfg->freq != savefreq)
    {
      freq = noteToFreq(cfg->freq);
//      oscA->frequency(freq);
//      oscB->frequency(freq);
      playf = freq;
    }
    subfreqdiv = cfg->subfreq;
    playsubf = freq / subfreqdiv;
//    subA->frequency(freq / cfg->subfreq);
//    subB->frequency(freq / cfg->subfreq);
//    offsetSteps = 0;
//    suboffsetSteps = 0;
    detune = cfg->detune;
    
    setShape(cfg->shape);
    setSubShape(cfg->subshape);
    applyFreq(false);
    setModAmount(cfg->modamt);
    setPWAmount(cfg->pwamt);
    setPWMod(cfg->pwm);
  }

  if (mixCfg)
  { 
    mix->gain(OSC_CHANNEL_A, mixCfg->oscmute[index] ? 0 : mixCfg->osclevel[index] / MIX_DIV);
    mix->gain(SUB_CHANNEL_A, mixCfg->submute[index] ? 0 : mixCfg->sublevel[index] / MIX_DIV);
    mix->gain(OSC_CHANNEL_B, mixCfg->oscmute[index] ? 0 : mixCfg->osclevel[index] / MIX_DIV);
    mix->gain(SUB_CHANNEL_B, mixCfg->submute[index] ? 0 : mixCfg->sublevel[index] / MIX_DIV);
  }
}

void Oscillator::setupPlay()
{
  overrideTuning = true;
  savefreq = config->osc[index].freq;
}

void Oscillator::cancelPlay()
{
  overrideTuning = false;
  savefreq = -1;
}

void Oscillator::setModAmount(ccInt u)
{
  modamt->amplitude(u / 127.0);
}

void Oscillator::setPWAmount(ccInt u)
{
  pwamt->amplitude(u / 127.0);
  updatePulseWidth(u);    
}

void Oscillator::setPWMod(bool onoff)
{
  if (onoff)
  {
//    pwmix->gain(0, 0.5);  
    pwmix->gain(1, 0);
  }
  else
  {
//    pwmix->gain(0, 0);  
    pwmix->gain(1, 0.5);
  }
}

float Oscillator::getFreq()
{
  return freq + offsetToFreq(offsetSteps, freq);
}

int Oscillator::getSubFreqDivisor()
{
  return min(max(subfreqdiv + suboffsetSteps, 2), 16);
}

int oscFreqIndex(float freq)
{
  int freqIndex = freqToNote(freq) / 3 + 1;
  return max(min(freqIndex, 44), 0); // protect against overflow & underflow
}

void Oscillator::getPlayingFrequencies(float *f, float *subf)
{
  if (f)
    *f = playf;
  if (subf)
    *subf = playsubf;
}

void Oscillator::applyFreq(bool applySubOnly)
{
  quantisation savetuning = config->tuning.scale;
  if (overrideTuning)
  {
    config->tuning.scale = FREE;
    applySubOnly = false;
  }
  float f = quantise(getFreq(), &config->tuning, true);
  float subf = quantise(f / getSubFreqDivisor(), &config->tuning, false);
  config->tuning.scale = savetuning;

#if DEBUG_SERIAL
  Serial.print("Apply frequency oscillator ");
  Serial.print(index + 1);
//  Serial.print(": sub only=");Serial.print(applySubOnly);
  if (!applySubOnly)
  {
    Serial.print(", freq=");Serial.print(freq);
    Serial.print(", offsetSteps=");Serial.print(offsetSteps);
  }
  Serial.print(", subfreqdiv=");Serial.print(subfreqdiv);
  Serial.print(", suboffsetSteps=");Serial.print(suboffsetSteps);
  Serial.print("\n\t");
  if (!applySubOnly) 
  {
      Serial.print("quantised f=");Serial.print(f);Serial.print(", ");
  }
  Serial.print("quantised subf=");Serial.print(subf);
  Serial.println();
#endif  
  if (!applySubOnly)
  {
    if (activeWavetable)
    {
      int idx = oscFreqIndex(f);
      if (idx != curidx)
      {
        curidx = idx;
        oscA->arbitraryWaveform((*activeWavetable)[idx], MAX_FREQ);
        oscB->arbitraryWaveform((*activeWavetable)[idx], MAX_FREQ);
      }
    }
    if (f <= 0)
    {
      Serial.print("Error - non-positive frequency: ");
      Serial.println(f);
    }
    else
    {
      playf = f;    
      oscA->frequency(f);
      oscB->frequency(detuneCents(f, detune));
    }
  }
  if (activeSubWavetable)
  {
    int idx = oscFreqIndex(subf);
    if (idx != cursubidx)
    {
      cursubidx = idx;
      subA->arbitraryWaveform((*activeSubWavetable)[idx], MAX_FREQ);
      subB->arbitraryWaveform((*activeSubWavetable)[idx], MAX_FREQ);
    }
  }
  if (subf <= 0)
  {
    Serial.print("Error - non-positive subfrequency: ");
    Serial.println(subf);
  }
  else
  {
    playsubf = subf;
    subA->frequency(subf);
    subB->frequency(subf);
  }
}

void Oscillator::update(float f, int subf, int off, int suboff, bool applyOsc, bool applySub)
{
#if DEBUG_SERIAL
  Serial.print("####################\nUpdating osc ");Serial.print(index);Serial.println("\n####################");
#endif  
  if (applyOsc)
  {
    freq = noteToFreq(f);
    offsetSteps = off;
#if DEBUG_SERIAL
    Serial.print("\tn = ");Serial.print(f);Serial.print(" -> ");Serial.print(freq);
    Serial.print(" and off = ");Serial.println(off);
#endif  
  }
  if (applySub)
  {
    subfreqdiv = subf;
    suboffsetSteps = suboff;
#if DEBUG_SERIAL
    Serial.print("\tsubfreqdiv = ");Serial.print(subf);Serial.print(" and suboffsetSteps = ");Serial.println(suboff);
#endif  
  }
  applyFreq(false);
}

void Oscillator::setFreq(float f)
{
  freq = f;
  offsetSteps = 0;
  applyFreq(false);
}

static int16_t pulseWavetable[45][257] = {{0}, {0}};

void Oscillator::updatePulseWidth(ccInt u)
{
  if (config->osc[index].shape != PULSE)
    return;    
  float pw = 0.5 + 0.5 * u / 127.0;
  // s[x] - s[x + d] + s[d]
  // PW 0 => 50%, PW 127 => 100%
  int dt = round(256 * pw);
  int offset = sawtoothWavetable[0][0];
//  Serial.print("updatePulseWidth offset ");Serial.println(dt);
  for (int i = 0; i < 45; ++i)
  {
    int j = -dt;
    for (int k = 0; k < 257; ++k, ++j)
    {
      int32_t s = sawtoothWavetable[i][k];
      int32_t t = -sawtoothWavetable[i][j];
      pulseWavetable[i][k] = min(32767, max(-32768, s + t + offset));
    }
  }
  activeWavetable = &pulseWavetable;
  oscA->begin(WAVEFORM_ARBITRARY);
  oscB->begin(WAVEFORM_ARBITRARY);
}

void Oscillator::setShape(waveshape s)
{
  if (s == saveshape)
    return;
  saveshape = s;
  curidx = -1;
  switch (s)
  {
    case TRIANGLE:
      activeWavetable = &triangleWavetable;
      oscA->begin(WAVEFORM_ARBITRARY);
      oscB->begin(WAVEFORM_ARBITRARY);
      break;
    case SAWTOOTH:
      activeWavetable = &sawtoothWavetable;
      oscA->begin(WAVEFORM_ARBITRARY);
      oscB->begin(WAVEFORM_ARBITRARY);
      break;
    case SQUARE:
      activeWavetable = &squareWavetable;
      oscA->begin(WAVEFORM_ARBITRARY);
      oscB->begin(WAVEFORM_ARBITRARY);
      break;
    case PULSE:
      updatePulseWidth(config->osc[index].pwamt);
      break;
    case SINE:
    case VAR_TRIANGLE:
      activeWavetable = NULL;
      oscA->begin(s);
      oscB->begin(s);
      break;
    default:
      break;  // unused waveshape
  }
}

void Oscillator::setSubFreqDivisor(int n)
{
  subfreqdiv = n;
  suboffsetSteps = 0;
  applyFreq(true);
}

void Oscillator::setOffsetSteps(int o, int s, bool doosc, bool dosub)
{
  if (doosc || dosub)
  {
    if (doosc)
      offsetSteps = o;
    if (dosub)
      suboffsetSteps = s;
    applyFreq(!doosc);
  }
}

void Oscillator::setSubShape(waveshape s)
{
  if (s == savesubshape)
    return;
  savesubshape = s;
  cursubidx = -1;
  switch (s)
  {
    case TRIANGLE:
      activeSubWavetable = &triangleWavetable;
      subA->begin(WAVEFORM_ARBITRARY);
      subB->begin(WAVEFORM_ARBITRARY);
      break;
    case SAWTOOTH:
      activeSubWavetable = &sawtoothWavetable;
      subA->begin(WAVEFORM_ARBITRARY);
      subB->begin(WAVEFORM_ARBITRARY);
      break;
    case SQUARE:
      activeSubWavetable = &squareWavetable;
      subA->begin(WAVEFORM_ARBITRARY);
      subB->begin(WAVEFORM_ARBITRARY);
      break;
    case SINE:
    case VAR_TRIANGLE:
    case PULSE:
      activeSubWavetable = NULL;
      subA->begin(s);
      subB->begin(s);
      break;
    default:
      break;  // unused waveshape
  }
}

void Oscillator::setLevel(ccInt lvl)
{
  mix->gain(OSC_CHANNEL_A, lvl / MIX_DIV);
  mix->gain(OSC_CHANNEL_B, lvl / MIX_DIV);
}

void Oscillator::setSubLevel(ccInt lvl)
{
  mix->gain(SUB_CHANNEL_A, lvl / MIX_DIV);
  mix->gain(SUB_CHANNEL_B, lvl / MIX_DIV);
}

void Oscillator::muteosc(bool mute)
{
  mix->gain(OSC_CHANNEL_A, mute ? 0 : config->mix.osclevel[index] / MIX_DIV);
  mix->gain(OSC_CHANNEL_B, mute ? 0 : config->mix.osclevel[index] / MIX_DIV);
}

void Oscillator::mutesub(bool mute)
{
  mix->gain(SUB_CHANNEL_A, mute ? 0 : config->mix.sublevel[index] / MIX_DIV);
  mix->gain(SUB_CHANNEL_B, mute ? 0 : config->mix.sublevel[index] / MIX_DIV);
}

void Oscillator::setScale(quantisation q)
{
  scale = q;
}

Modulator::Modulator(AudioSynthWaveform *lfo)
{
  this->lfo = lfo;
  lfo->amplitude(1.0);
  f = 0.1;
  lfo->frequency(f); // minimum
  lfo->begin(WAVEFORM_TRIANGLE);
}

void Modulator::loadConfig(modulatorCfg *cfg)
{
  setModRate(cfg->rate);
  setModShape(cfg->shape);
}
    
void Modulator::setModRate(ccInt u)
{
// convert log scale 1 to 127 to a linear rate
// f(1) = 0.5Hz
// f(127) = 40Hz
// f(x) = Math.exp(x / 25) / 4
// ?+ 100{]_,E^(_/25) / 4}
  f = exp(u / 25.0) / 4.0;
  lfo->frequency(f);
}

float Modulator::getModRate()
{
  return f;
}

void Modulator::setModShape(waveshape s)
{
  lfo->begin(s);
}


#define CHORUS_AMP_FACTOR 2.0
#define FLANGE_AMP_FACTOR 1.2
#define DELAY_AMP_FACTOR 1.5

#define FLANGE_DELAY_LENGTH (6 * AUDIO_BLOCK_SAMPLES)
static int offset =  FLANGE_DELAY_LENGTH / 4;
static int depth = FLANGE_DELAY_LENGTH / 4;
static double flangeRate = 0.01;
static short delay_flanger[FLANGE_DELAY_LENGTH];

Effects::Effects(AudioEffectDelay *deff, AudioEffectEnsemble *ch, AudioEffectFlange *f, 
                 AudioMixer4 *dmix, AudioMixer4 *emixL, AudioMixer4 *emixR)
{
  delayeff = deff;
  chorus = ch;
  flange = f;
  delaymix = dmix;
  effectmixL = emixL;
  effectmixR = emixR;

  effectmixL->gain(0, 1);
  effectmixL->gain(1, 0);
  effectmixL->gain(2, 0);
  effectmixR->gain(0, 1);
  effectmixR->gain(1, 0);
  effectmixR->gain(2, 0);
  
  delaymix->gain(0, 0);
  delaymix->gain(1, 0);
  delayeff->disable(0);
  
  flange->begin(delay_flanger, FLANGE_DELAY_LENGTH, offset, depth, flangeRate);
}
  
void Effects::loadConfig(effectsCfg *cfg)
{
  bool chorusOn = (cfg->active == CHORUS || cfg->active == CHORUS_DELAY) && (cfg->chorus != BYPASS);
  bool flangeOn = (cfg->active == FLANGE) && (cfg->flange != BYPASS);
  bool delayOn = (cfg->active == DELAY || cfg->active == CHORUS_DELAY) && (cfg->delay != BYPASS);
  
  int count = 0;
  if (chorusOn)
    count++;
  if (flangeOn)
    count++;
  if (delayOn)
    count++;
  setBypassLevel(count == 0 ? 1 : 0); // turn on if no effects
  setFlangePreset(cfg->flange, flangeOn ? 1.0 / count : 0); 
  setDelayPreset(cfg->delay, delayOn ? 1.0 / count : 0); 
  setChorusPreset(cfg->chorus, chorusOn ? 1.0 / count : 0);
}

void Effects::setBypassLevel(float level)
{
  level = max(0, min(1, level));
  effectmixL->gain(0, level);
  effectmixR->gain(0, level);
//  Serial.print("Effects bypass level set to ");Serial.println(level);
}

void Effects::setChorusPreset(effect_preset p, float level)
{
  level = max(0, min(1, level)) * CHORUS_AMP_FACTOR;
  effectmixL->gain(1, p == BYPASS ? 0 : level);
  effectmixR->gain(1, p == BYPASS ? 0 : level);
  chorus->lfoRate(2 * (int)p);
//  Serial.print("Effects chorus level set to ");Serial.print(level);Serial.print(" and rate ");Serial.println(2 * (int)p);
}

void Effects::setFlangePreset(effect_preset p, float level)
{
  level = max(0, min(1, level)) * FLANGE_AMP_FACTOR;
  effectmixL->gain(3, p == BYPASS ? 0 : level);
  effectmixR->gain(3, p == BYPASS ? 0 : level);
  flange->voices(offset, depth, flangeRate * (2 * (int)p - 1));
//  Serial.print("Effects flange level set to ");Serial.print(level);Serial.print(" and rate ");Serial.println(flangeRate * (2 * (int)p - 1));
}

void Effects::setDelayPreset(effect_preset p, float level)
{
  level = max(0, min(1, level)) * DELAY_AMP_FACTOR;
  effectmixL->gain(2, p == BYPASS ? 0 : level);
  effectmixR->gain(2, p == BYPASS ? 0 : level);
  switch (p)
  {
    case BYPASS: // off
      delayeff->disable(0);
      delaymix->gain(1, 0);
      break;
    case FAST:
      setDelayParams(3, 0.9);
      break;
    case MEDIUM:
      setDelayParams(2, 0.75);
      break;
    case SLOW:
      setDelayParams(0.5, 0.6);
      break;
  }
//  Serial.print("Effects delay level set to ");Serial.print(level);Serial.print(" and preset to ");Serial.println(p);
}

void Effects::setDelayParams(float perBeat, float feedback)
{
  long delaytime = 1.0 * poly->getDelayMillis() / perBeat;
  while (delaytime > 1000)
    delaytime /= 2;
  feedback = max(0, min(1, feedback));
  delayeff->delay(0, delaytime);
  delaymix->gain(0, 0.75);
  delaymix->gain(1, 0.8 * feedback);
}
    
Synth::Synth(Filter *f, Envelope *e1, Envelope *e2, Oscillator *o1, Oscillator *o2, Modulator *m, 
             Effects *e, AudioAmplifier *l, AudioAmplifier *r)
{
//  env = new (Envelope *)[2];
//  osc = new (Oscillator *)[2];
  filter = f;
  env[0] = e1;
  env[1] = e2;
  osc[0] = o1;
  osc[1] = o2;
  mod = m;
  eff = e;
  levelL = l;
  levelR = r;
}

Synth::~Synth()
{
  delete filter;
  delete env[0];
  delete env[1];
//  delete env;
  delete osc[0];
  delete osc[1];
//  delete osc;
  delete mod;
}

void Synth::setupPlay(int oscIdx)
{
#if DEBUG_SERIAL
  Serial.print("setupPlay ");Serial.println(oscIdx);
#endif  
  env[oscIdx]->noteoff();
  env[oscIdx]->setASRMode(true);
  osc[oscIdx]->setupPlay();
  osc[oscIdx]->setOffsetSteps(0, 0, true, true);
}

void Synth::cancelPlay(int oscIdx)
{
#if DEBUG_SERIAL
  Serial.print("cancelPlay ");Serial.println(oscIdx);
#endif  
  if (!droning)
  {
    env[oscIdx]->setASRMode(false);
    env[oscIdx]->noteoff();
  }
  osc[oscIdx]->cancelPlay();
//  osc[oscIdx]->setFreq(noteToFreq(config->osc[oscIdx].freq));
//  float val = config->osc[oscIdx].freq;
//  float f = noteToFreq(val);
  loadOscConfig(&config->osc[oscIdx], oscIdx);
}

void Synth::play(float freq, int subfreq, int oscIdx)
{
#if DEBUG_SERIAL
  Serial.print("playing oscillator ");Serial.println(oscIdx);
#endif  
  if (!started)
    start();
  droning = false;
  if (subfreq > 0)
    osc[oscIdx]->setSubFreqDivisor(subfreq);
  if (freq > 0)
    osc[oscIdx]->setFreq(freq);
  env[oscIdx]->trigger();
  filter->trigger();
};

void Synth::noteoff(int oscIdx)
{
  env[oscIdx]->noteoff();
}

void Synth::offset(int offset, int suboffset, bool doosc, bool dosub, int oscIdx)
{
#if DEBUG_SERIAL
  Serial.print("Setting offsets for oscillator ");Serial.print(oscIdx + 1);
  if (doosc) { Serial.print(": offset=");Serial.print(offset); }
  if (dosub) { Serial.print(", suboffset=");Serial.print(suboffset); }
  Serial.println();
#endif  
  if (!started)
    start();
  osc[oscIdx]->setOffsetSteps(offset, suboffset, doosc, dosub);
  env[oscIdx]->trigger();
  filter->trigger();
};

void Synth::globaloffset(int offset, bool absolute)
{
  if (!started)
    start();
  config->tuning.transpose = offset;
  osc[0]->applyFreq(false);
  osc[1]->applyFreq(false);
};

void Synth::start()
{
  if (started)
    return;
  started = true;
  osc[0]->loadConfig(&config->osc[0], &config->mix);
  osc[1]->loadConfig(&config->osc[1], &config->mix);
};

void Synth::setLevel(int u)
{
  // make logarithmic f () k x ~ {Int (x / (1 + (1 - x / 1023) * k))}
  int k = 2;
  int log_u = min(127, 3.0 * u / (1 + k * (1 - u / 127)));
//  Serial.print(u);Serial.print(" => ");Serial.println(log_u);
  levelL->gain(2 * log_u / 127.0); 
  levelR->gain(2 * log_u / 127.0); 
}

void Synth::stop()
{
  droning = false;
  env[0]->noteoff();
  env[1]->noteoff();
  restoreEnvMode();
};

void Synth::setHoldMode()
{
  env[0]->setHoldMode(true);
  env[1]->setHoldMode(true);
}

void Synth::restoreEnvMode()
{
  env[0]->setHoldMode(false);
  env[1]->setHoldMode(false);
}

void Synth::drone(bool trigger)
{
  if (!started)
    start();

  if (!droning)
  {
    poly->stop();
    droning = true;
    dronestep = 0;
  }
  else
  {
    dronestep++;
    dronestep %= 8;
  }

  int oscIdx = dronestep < 4 ? 0 : 1;
  int step = dronestep % 4;
//  synth.droneparam = "seq" + (osc + 1) + "_step" + (step + 1);
  Sequencer *seq = oscIdx == 0 ? seq1 : seq2;
  seq1->allLedsOff();
  seq2->allLedsOff();
  seq->ledOn(step);

  int offset = seq->seqcfg.steps[step];
  int suboffset = floor(-seq->seqcfg.steps[step] / 3.0);

  bool adjosc = seq->seqcfg.applyOsc;
  bool adjsub = seq->seqcfg.applySub;

#if DEBUG_SERIAL
  Serial.print("Drone");
  Serial.print("\tosc ");Serial.print(oscIdx + 1);Serial.print("\tstep ");Serial.println(step);
  Serial.print("\toffset ");Serial.print(seq->seqcfg.steps[step]);Serial.print("\toffset freq ");Serial.println(offset);
  Serial.print("\tsuboffset ");Serial.print(seq->seqcfg.steps[step]);Serial.print(" => ");Serial.print(suboffset);  
  Serial.print("\tapplyOsc ");Serial.print(adjosc);Serial.print("\tapplySub ");Serial.print(adjsub);
  Serial.println();
#endif
      
  osc[oscIdx]->setOffsetSteps(offset, suboffset, adjosc, adjsub);
  if (trigger)
  {
    env[oscIdx]->trigger();
    env[oscIdx == 0 ? 1 : 0]->noteoff();
  }
}

void Synth::updateOscillator(int i, oscillatorCfg *cfg, int curStep, sequencerCfg *seqcfg)
{
  float f = cfg->freq;
  int subfdiv = cfg->subfreq;
  bool applyOsc = seqcfg->applyOsc;
  bool applySub = seqcfg->applySub;
  int off = seqcfg->steps[curStep];
  int suboff = floor(-off / 3.0);
  osc[i]->update(f, subfdiv, off, suboff, applyOsc, applySub);
  env[i]->trigger();
}

void Synth::retrigger(bool osc1, bool osc2)
{
  if (osc1)
    env[0]->trigger();
  if (osc2)
    env[1]->trigger();
}

void Synth::updateDrone()
{
  if (!droning)
    return;
  
  if (dronestep < 4)
    seq1->update(false);
  else
    seq2->update(false);
  dronestep--;
  drone(false);
}

void Synth::setModRate(ccInt u)
{
  mod->setModRate(u);
}

float Synth::getModRate()
{
  return mod->getModRate();
}

void Synth::setModShape(waveshape shape)
{
  mod->setModShape(shape);
}

void Synth::setAttack(ccInt u)
{
  env[0]->setAttack(u);
  env[1]->setAttack(u);
  filter->setAttack(u);
}

void Synth::setDecay(ccInt u)
{
  env[0]->setDecay(u);
  env[1]->setDecay(u);
  filter->setDecay(u);
}

void Synth::modPWAmount(ccInt pw, int i)
{
//  osc[i]->setPWAmount(pw);
  osc[i]->updatePulseWidth(pw);
}

float Synth::getCutoffFreq()
{
  return filter->getCutoffFreq();
}

void Synth::loadConfig(euclid_config *cfg)
{
  filter->loadConfig(&cfg->filter, &cfg->env); 
  env[0]->loadConfig(&cfg->env); 
  env[1]->loadConfig(&cfg->env); 
  osc[0]->loadConfig(&cfg->osc[0], &config->mix); 
  osc[1]->loadConfig(&cfg->osc[1], &config->mix);
  mod->loadConfig(&cfg->lfo);
  eff->loadConfig(&cfg->eff);
}

void Synth::loadOscConfig(oscillatorCfg *cfg, int i)
{
  osc[i]->loadConfig(cfg, NULL); 
}

void Synth::loadMixerConfig(mixerCfg *cfg)
{
  osc[0]->loadConfig(NULL, cfg); 
  osc[1]->loadConfig(NULL, cfg); 
}

void Synth::loadEnvConfig(envelopeCfg *cfg)
{
  env[0]->loadConfig(cfg); 
  env[1]->loadConfig(cfg); 
  filter->loadConfig(NULL, cfg); 
}

void Synth::loadModConfig(modulatorCfg *cfg)
{
  mod->loadConfig(cfg); 
}

void Synth::loadFilterConfig(filterCfg *cfg)
{
  filter->loadConfig(cfg, NULL);  
}

void Synth::loadEffectsConfig(effectsCfg *cfg)
{
  eff->loadConfig(cfg); 
}

void Synth::getDroningFrequencies(float *f, float *subf)
{
  if (!droning)
  {
    if (f)
      *f = 0;
    if (subf)
      *subf = 0;
  }
  osc[dronestep < 4 ? 0 : 1]->getPlayingFrequencies(f, subf);
}

void Synth::getPlayingFrequencies(float *f1, float *subf1, float *f2, float *subf2)
{
  osc[0]->getPlayingFrequencies(f1, subf1);
  osc[1]->getPlayingFrequencies(f2, subf2);
}
