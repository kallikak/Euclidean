#include "Config.h"
#include "Presets.h"
#include "Drums.h"

typedef char Str20[21];
typedef char Str100[101];

bool useMIDIClock = false;

euclid_config defaultConfig = {
  {{40, SQUARE, 0, false, 0, 2, SAWTOOTH, 0}, {40, SQUARE, 0, false, 0, 2, SAWTOOTH, 0}}, // oscillators
  {{127, 127}, {false, false}, {127, 127}, {false, false}},   // mixer
  {0, 127},        // env
  {TRIANGLE, 0},   // modulator
  {127, 0, 0, 0},  // filter
  {CHROMATIC, 0},  // tuning
  {NONE, BYPASS, BYPASS, BYPASS},       // effect
  {{{0, 0, 0, 0}, true, true, {true, false, false}, false}, {{0, 0, 0, 0}, true, true, {false, false, false}, true}}, // sequencers
  {-1, {4, 16, 0}, {5, 16, 0}, {3, 16, 3}},  // rhythm
  {BT_SEL, BT_SEL, {100, 101, 106, 102}, "Default"}  // drums
};

euclid_config activeConfig = defaultConfig;

euclid_config *config = &activeConfig;

void setDefaultConfig()
{
  activeConfig = defaultConfig;
  setDrumsConfig(&activeConfig.drum);
}

void setPreset(int i)
{
  loadPresetSD(i, &activeConfig);
  setDrumsConfig(&activeConfig.drum);
}

waveshape getNextWaveshape(bool osc, bool sub, bool lfo, int i, bool back)
{
  waveshape curshape;
  if (osc)
    curshape = config->osc[i].shape;
  else if (sub)
    curshape = config->osc[i].subshape;
  else if (lfo)
    curshape = config->lfo.shape;
  else
    return SAWTOOTH;  // default

  // LFO: SINE <-> TRIANGLE <-> SQUARE <-> SAWTOOTH <-> REV_SAWTOOTH <-> S_AND_H
  // Osc: SINE <-> TRIANGLE <-> SQUARE <-> SAWTOOTH <-> PULSE
  // Sub: SINE <-> TRIANGLE <-> SQUARE <-> SAWTOOTH
    
  switch (curshape)
  {
    case SINE:
      return back ? (lfo ? S_AND_H : (sub ? SAWTOOTH : PULSE)) : TRIANGLE;
    case TRIANGLE:
      return back ? SINE : SQUARE;
    case SQUARE:
      return back ? TRIANGLE : SAWTOOTH;
    case SAWTOOTH:
      return back ? SQUARE : (lfo ? REV_SAWTOOTH : (sub ? SINE : PULSE));
    case REV_SAWTOOTH:
      return back ? SAWTOOTH : S_AND_H;
    case PULSE:
      return back ? SAWTOOTH : SINE;
    case S_AND_H:
      return back ? REV_SAWTOOTH : SINE;
    default:
      return SAWTOOTH;
  }
}

const char *waveshapeStr(waveshape w, bool full)
{
  switch (w)
  {
    case SINE:
      return "Sine";
    case TRIANGLE:
      return "Triangle";
    case VAR_TRIANGLE:
      return full ? "Variable triangle" : "Var tri";
    case SQUARE:
      return "Square";
    case SAWTOOTH:
      return full ? "Sawtooth" : "Saw";
    case REV_SAWTOOTH:
      return full ? "Reverse sawtooth" : "Rev saw";
    case PULSE:
      return "Pulse";
    case S_AND_H:
      return full ? "Sample and hold" : "S & H";
  }
  return "";
}

const char *quantisationStr(quantisation q, bool full)
{
  switch (q)
  {
    case FREE:
      return "Free";
    case CHROMATIC:
      return "Chromatic";
    case DIATONIC:
      return "Diatonic";
    case DIATONIC_M:
      return full ? "Diatonic minor" : "Diatonic m";
    case PENTATONIC:
      return "Pentatonic";
    case PENTATONIC_M:
      return full ? "Pentatonic minor" : "Pent. m";
    case PYTHAGOREAN:
      return "Pythagorean";
    case PYTHAGOREAN_M:
      return full ? "Pythagorean minor" : "Pyth. m";
    case JUST:
      return full ? "Just intonation" : "Just int.";
    case JUST_M:
      return "Just minor";
  }
  return "";
}

const char *effectStr(effect e)
{
  switch (e)
  {
    case NONE:
      return "None";
    case CHORUS:
      return "Chorus";
    case FLANGE:
      return "Flanger";
    case DELAY:
      return "Delay"; 
    case CHORUS_DELAY:
      return "Chorus+Delay"; 
  }
  return "-";
}

const char *effectPresetStr(effect_preset p)
{
  switch (p)
  {
    case BYPASS:
      return "Bypass";
    case SLOW:
      return "Slow";
    case MEDIUM:
      return "Medium";
    case FAST:
      return "Fast"; 
  }
  return "-";
}

void printOscConfig(int i, oscillatorCfg *cfg)
{
  Str100 str = {0};
  sprintf(str, "Oscillator %d: freq %.2f, %s, PW: %d, PWM: %d, Mod: %d, Detune: %d, subfreq %d, %s", 
    i + 1, cfg->freq, waveshapeStr(cfg->shape, true), cfg->pwamt, cfg->pwm, cfg->modamt, cfg->detune, 
    cfg->subfreq, waveshapeStr(cfg->subshape, true));
  Serial.println(str);
}

void printMixConfig(mixerCfg *cfg)
{
  Str100 str = {0};
  sprintf(str, "Levels: O1 %d%s, S1 %d%s, O2 %d%s, S2 %d%s", 
    cfg->osclevel[0], cfg->oscmute[0] ? " (mute)" : "", cfg->sublevel[0], cfg->submute[0] ? " (mute)" : "", 
    cfg->osclevel[1], cfg->oscmute[1] ? " (mute)" : "", cfg->sublevel[1], cfg->submute[1] ? " (mute)" : "");
  Serial.println(str);
}

void printEnvelopeConfig(envelopeCfg *cfg)
{
  Str100 str = {0};
  sprintf(str, "Envelope: A %d, D %d", cfg->attack, cfg->decay);
  Serial.println(str);
}

void printModulatorConfig(modulatorCfg *cfg)
{
  Str100 str = {0};
  sprintf(str, "Mod: rate %d %s", cfg->rate, waveshapeStr(cfg->shape, true));
  Serial.println(str);
}

void printFilterConfig(filterCfg *cfg)
{
  Str100 str = {0};
  sprintf(str, "Filter: cutoff %d, resonance %d, envamt %d, modamt %d", 
    cfg->cutoff, cfg->resonance, cfg->envamt, cfg->modamt);
  Serial.println(str);
}

void printTuningConfig(tuningCfg *cfg)
{
  Str100 str = {0};
  sprintf(str, "Tuning: scale %s, transpose %d", quantisationStr(cfg->scale, true), cfg->transpose);
  Serial.println(str);
}

void printSequencerConfig(int i, sequencerCfg *cfg)
{
  int k;
  Serial.print("Sequencer ");Serial.println(i + 1);
  for (k = 0; k < 4; ++k)
  {
    Serial.print("\tstep ");Serial.print(k + 1);Serial.print(": ");Serial.print(cfg->steps[k]);
  }
  Serial.println();
  Serial.print("\tapplyOsc ");Serial.print(cfg->applyOsc);
  Serial.print("\tapplySub ");Serial.print(cfg->applySub);
  Serial.println();
  for (k = 0; k < 3; ++k)
  {
    Serial.print("\tadvBeat ");Serial.print(k + 1);Serial.print(": ");Serial.print(cfg->advBeat[k]);
  }
  Serial.print("\tadvOffbeat ");Serial.print(cfg->advOffbeat);
  Serial.println();
}

void printRhythmConfig(rhythmCfg *cfg)
{  
  Str100 str = {0};
  sprintf(str, "Rhythm: tempo %d, R1 %d %d (%d), R2 %d %d (%d), R3 %d %d (%d)", 
    cfg->tempo, cfg->r1[0], cfg->r1[1], cfg->r1[2], cfg->r2[0], cfg->r2[1], cfg->r2[2], 
    cfg->r3[0], cfg->r3[1], cfg->r3[2]);
  Serial.println(str);
}

void printEffectsConfig(effectsCfg *cfg)
{
  Serial.print("Effects: Active effect ");Serial.print(effectStr(cfg->active));
  Serial.print(", Chorus type ");Serial.print(effectPresetStr(cfg->chorus));
  Serial.print(", Flange type ");Serial.print(effectPresetStr(cfg->flange));
  Serial.print(", Delay type ");Serial.println(effectPresetStr(cfg->delay));
}

const char *beatstateStr(beatState state)
{
  switch (state)
  {
    case BT_ONE:
      return "r1";
    case BT_SEL:
      return "sel";
    case BT_ALL:
      return "123";
    case BT_NONE:
      return "all";
    case BT_OFF:
    default:
      return "off";
  }
}

void printDrumConfig(drumCfg *cfg)
{
  Serial.print("Drums - beat: ");
  Serial.print(beatstateStr(cfg->beatstate));
  Serial.print(", off: ");
  Serial.print(beatstateStr(cfg->offbeatstate));
  Serial.print(", ");
  Serial.print(getKitString(false));
  if (cfg->sampleset)
  {
    Serial.print(", ");
    Serial.println(cfg->sampleset);
  }
}

void printConfig(euclid_config *cfg)
{
  if (!cfg)
    cfg = config;
  printOscConfig(0, &cfg->osc[0]);
  printOscConfig(1, &cfg->osc[1]);
  printMixConfig(&cfg->mix);
  printEnvelopeConfig(&cfg->env);
  printModulatorConfig(&cfg->lfo);
  printFilterConfig(&cfg->filter);
  printEffectsConfig(&cfg->eff);
  printTuningConfig(&cfg->tuning);
  printSequencerConfig(0, &cfg->seq[0]);
  printSequencerConfig(1, &cfg->seq[1]);
  printRhythmConfig(&cfg->rhythm);
  printDrumConfig(&cfg->drum);
}
