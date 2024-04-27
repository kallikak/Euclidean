#pragma once

extern Polyrhythm *poly;
extern Sequencer *seq1;
extern Sequencer *seq2;

class Filter
{
  private:
    AudioFilterStateVariable *drumfilter;
    AudioFilterStateVariable *filter;
    AudioSynthWaveformDc     *fenvamt;
    AudioSynthWaveformDc     *fmodamt;
    AudioEffectEnvelope      *fenv;
    AudioMixer4              *fmix;
    AudioAmplifier           *filterpreamp;
    AudioAmplifier           *drumfilterpreamp;

    float co;
    float q;

  public:
    Filter(AudioFilterStateVariable* filter, AudioFilterStateVariable* drumfilter, AudioSynthWaveformDc* fenvamt, AudioSynthWaveformDc* fmodamt,
      AudioEffectEnvelope* fenv, AudioMixer4* fmix, AudioAmplifier* filterpreamp, AudioAmplifier* drumfilterpreamp);

    void loadConfig(filterCfg *fCfg, envelopeCfg *envCfg);
    void setCutoff(ccInt u);
    float getCutoffFreq();
    void setResonance(ccInt u);
    void setModAmount(ccInt u);
    void setAttack(ccInt a);
    void setDecay(ccInt d);
    void setEnvAmount(ccInt u);

    void trigger();
};

class Envelope
{
  private:
    bool holdMode;
    AudioEffectEnvelope *env;

  public:
    Envelope(AudioEffectEnvelope *env);
    
    void loadConfig(envelopeCfg *cfg);
    void setAttack(ccInt a);
    void setDecay(ccInt d);
    void setRelease(ccInt r);
    void setHoldMode(bool onoff);
    void setASRMode(bool onoff);
    bool isHoldMode();
    void trigger();
    void noteoff();
};

class Modulator
{
  private:
    AudioSynthWaveform *lfo;
    float f;

  public:
    Modulator(AudioSynthWaveform *lfo);
    
    void loadConfig(modulatorCfg *cfg);
    void setModRate(ccInt u);
    void setModShape(waveshape s);
    float getModRate();
};

class Oscillator
{
  private:
    AudioSynthWaveformModulated *oscA;
    AudioSynthWaveformModulated *oscB;
    AudioSynthWaveform *subA;
    AudioSynthWaveform *subB;
    AudioMixer4 *mix;
    AudioSynthWaveformDc *modamt;
    AudioSynthWaveformDc *pwamt;
    AudioMixer4 *pwmix;

    int index;
    
    float freq;
    int subfreqdiv;
    quantisation scale;

    float playf;
    float playsubf;
    float savefreq;

    int saven;
    int savesubn;
    
    int offsetSteps;
    int suboffsetSteps;
    int detune;
    
    const int16_t (*activeWavetable)[45][257];
    const int16_t (*activeSubWavetable)[45][257];
    int cursubidx;
    int curidx;
  
    bool overrideTuning;
    waveshape saveshape;
    waveshape savesubshape;

  public:
    Oscillator(int i, AudioSynthWaveformModulated *o1, AudioSynthWaveform *s1, 
      AudioSynthWaveformModulated *o2, AudioSynthWaveform *s2, 
      AudioMixer4 *mix, AudioSynthWaveformDc *modamt, AudioSynthWaveformDc *pwamt, AudioMixer4 *pwmix);

    void loadConfig(oscillatorCfg *cfg, mixerCfg *mixCfg);
    void setModAmount(ccInt u);
    void setPWAmount(ccInt u);
    void setPWMod(bool onoff);
    float getFreq();
    int getSubFreqDivisor();
    void applyFreq(bool applySubOnly);
    void update(float f, int subf, int off, int suboff, bool applyOsc, bool applySub);
    void setFreq(float f);
    void setDetuneCents(int c);
    void setShape(waveshape s);
    void setSubFreqDivisor(int n);
    void setSubShape(waveshape s);
    void setOffsetSteps(int o, int s, bool doosc, bool dosub);
    void setLevel(ccInt lvl);
    void setSubLevel(ccInt lvl);
    void muteosc(bool mute);
    void mutesub(bool mute);
    void updatePulseWidth(ccInt u);
    void setScale(quantisation q);
    void setupPlay();
    void cancelPlay();
    void getPlayingFrequencies(float *f, float *subf);
};

class AudioEffectEnsemble;

class Effects
{
  private:
    AudioMixer4 *delaymix;
    AudioEffectDelay *delayeff;
    AudioEffectEnsemble *chorus;
    AudioEffectFlange *flange;
    AudioMixer4 *effectmixL;
    AudioMixer4 *effectmixR;

    void setDelayParams(float time, float feedback);

  public:
    Effects(AudioEffectDelay *deff, AudioEffectEnsemble *ch, AudioEffectFlange *f, AudioMixer4 *dmix, AudioMixer4 *emixL, AudioMixer4 *emixR);

    void loadConfig(effectsCfg *cfg);

    void setBypassLevel(float level);
    void setChorusPreset(effect_preset p, float level);
    void setFlangePreset(effect_preset p, float level);
    void setDelayPreset(effect_preset p, float level);
};

class Synth
{
  private:
    Filter *filter = NULL;
    Envelope *env[2] = {NULL, NULL};
    Oscillator *osc[2] = {NULL, NULL};
    Modulator *mod = NULL;
    Effects *eff = NULL;
    AudioAmplifier *levelL, *levelR;
    
    int dronestep = 0;
  
  public:
    bool started = false;
    bool droning = false;
    bool initialised = false;

    Synth(Filter *f, Envelope *e1, Envelope *e2, Oscillator *o1, Oscillator *o2, Modulator *m, 
          Effects *e, AudioAmplifier *l, AudioAmplifier *r);
    ~Synth();

    void loadConfig(euclid_config *cfg);
    
    void loadOscConfig(oscillatorCfg *cfg, int i);
    void loadMixerConfig(mixerCfg *cfg);
    void loadEnvConfig(envelopeCfg *cfg);
    void loadModConfig(modulatorCfg *cfg);
    void loadFilterConfig(filterCfg *cfg);
    void loadEffectsConfig(effectsCfg *cfg);

    void setupPlay(int oscIdx);
    void cancelPlay(int oscIdx);
    void play(float freq, int subfreq, int osc);
    void noteoff(int osc);
    void offset(int offset, int suboffset, bool doosc, bool dosub, int osc);
    void globaloffset(int offset, bool absolute);
    void start();
    void stop();
    void setHoldMode();
    void restoreEnvMode();
    void retrigger(bool osc1, bool osc2);
    void drone(bool trigger);
    void updateDrone();
    void updateOscillator(int i, oscillatorCfg *cfg, int curStep, sequencerCfg *seqcfg);
    void setModRate(ccInt u);
    float getModRate();
    void setModShape(waveshape shape);
    void setLevel(int l);
    void setAttack(ccInt u);
    void setDecay(ccInt u);
    float getCutoffFreq();   
    void modPWAmount(ccInt u, int i);
    void getDroningFrequencies(float *f, float *subf);
    void getPlayingFrequencies(float *f1, float *subf1, float *f2, float *subf2);
};
