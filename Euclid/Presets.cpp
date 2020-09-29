#include <SD_t3.h>
#include <strings.h>

#include "Config.h"
#include "Controls.h"
#include "Presets.h"
#include "LCD.h"
#include "drum_defs.h"

//Teensy Audio Shield 4.0 SD card pins
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  11
#define SDCARD_MISO_PIN  12
#define SDCARD_SCK_PIN   13

static char tempstr[20] = {0};
boolean cardAvailable = false;

void updateBackup();
bool getPresets(File file);
bool setupSDCard();
bool getPresetsSD(File file);
void printPresets();

// 10 default presets for when there is no SD card

// Wah
euclid_config preset0 = {
  {   // oscillators
    {68, PULSE, 60, false, 0, 6, SAWTOOTH, 6}, 
    {55, PULSE, 0, false, 0, 4, SAWTOOTH, 6}
  },
  {{43, 43}, {false, false}, {127, 127}, {false, false}},   // mixer
  {0, 47},             // env
  {TRIANGLE, 0},      // modulator
  {100, 75, -20, 75},  // filter
  {PENTATONIC, 0},     // tuning
  {NONE, BYPASS, BYPASS, BYPASS},       // effect
  {    // sequencers
    {{3, 7, -6, 3}, false, true, {true, false, true}, true}, 
    {{0, 13, 4, -8}, false, true, {true, false, true}, false}
  },
  {84, {4, 13, 0}, {2, 5, 0}, {3, 7, 0}},  // rhythm
  {BT_OFF, BT_OFF, {100, 101, 102, 103}}  // drums
};

// Pluck
euclid_config preset1 = {
  {   // oscillators
    {43, PULSE, 77, true, 0, 4, SAWTOOTH, 5}, 
    {53, SQUARE, 26, false, 0, 2, SAWTOOTH, 5}
  }, 
  {{127, 127}, {false, false}, {51, 62}, {false, false}},   // mixer
  {0, 20},             // env
  {SQUARE, 85},      // modulator
  {127, 30, 13, 24},  // filter
  {CHROMATIC, 0},     // tuning
  {CHORUS_DELAY, MEDIUM, BYPASS, MEDIUM},       // effect
  {    // sequencers
    {{3, 7, -7, 2}, true, false, {true, false, false}, false}, 
    {{0, 7, -5, 7}, true, false, {true, true, true}, false}
  },
  {85, {1, 4, 0}, {2, 5, 0}, {4, 7, 0}},  // rhythm
  {BT_OFF, BT_OFF, {100, 101, 102, 103}}  // drums
};

// SciFi
euclid_config preset2 = {
  {   // oscillators
    {68, PULSE, 60, false, 0, 10, TRIANGLE, 0}, 
    {55, PULSE, 0, false, 0, 2, TRIANGLE, 0}
  }, 
  {{43, 43}, {false, false}, {127, 127}, {false, false}},   // mixer
  {0, 47},             // env
  {SQUARE, 33},      // modulator
  {100, 75, -20, 38},  // filter
  {PENTATONIC, 0},     // tuning
  {NONE, BYPASS, BYPASS, BYPASS},       // effect
  {    // sequencers
    {{3, 7, -6, 3}, true, true, {true, false, true}, false}, 
    {{0, 13, 4, -8}, true, true, {false, true, true}, false}
  },
  {96, {7, 16, 0}, {9, 16, 0}, {8, 16, 0}},  // rhythm
  {BT_OFF, BT_OFF, {100, 101, 103, 102}}  // drums
};

// Meditation
euclid_config preset3 = {
  {   // oscillators
    {53, SQUARE, 0, false, 0, 4, SAWTOOTH, 0}, 
    {73.4, TRIANGLE, 0, false, 5, 3, SAWTOOTH, 0}
  }, 
  {{46, 44}, {false, false}, {127, 127}, {false, false}},   // mixer
  {127, 127},             // env
  {TRIANGLE, 67},      // modulator
  {103, 42, 16, 19},  // filter
  {DIATONIC, 0},     // tuning
  {DELAY, BYPASS, BYPASS, SLOW},       // effect
  {    // sequencers
    {{0, 3, 6, 9}, true, false, {true, true, true}, false}, 
    {{-5, -9, -13, -17}, false, true, {true, false, false}, true}
  },
  {2, {5, 13, 0}, {1, 6, 0}, {1, 11, 0}},  // rhythm
  {BT_OFF, BT_OFF, {100, 101, 103, 102}}  // drums
};

// Echo
euclid_config preset4 = {
  {   // oscillators
    {40, SAWTOOTH, 0, false, 0, 4, TRIANGLE, 15}, 
    {47, SAWTOOTH, 0, false, 0, 6, TRIANGLE, 15}
  }, 
  {{127, 127}, {false, false}, {127, 127}, {false, false}},   // mixer
  {0, 10},             // env
  {S_AND_H, 63},      // modulator
  {81, 75, 35, 47},  // filter
  {DIATONIC, 0},     // tuning
  {DELAY, BYPASS, BYPASS, SLOW},       // effect
  {    // sequencers
    {{8, 0, -5, 5}, true, true, {true, false, true}, false}, 
    {{0, 6, 9, 24}, true, true, {false, true, true}, false}
  },
  {91, {5, 12, 0}, {4, 12, 0}, {3, 12, 0}},  // rhythm
  {BT_OFF, BT_OFF, {100, 101, 103, 102}}  // drums
};

// Funk
euclid_config preset5 = {
  {   // oscillators
    {40, PULSE, 40, true, 0, 7, SAWTOOTH, 5}, 
    {47, PULSE, 40, true, 0, 8, SAWTOOTH, 5}
  }, 
  {{53, 54}, {false, false}, {127, 127}, {false, false}},   // mixer
  {0, 15},             // env
  {TRIANGLE, 32},      // modulator
  {39, 98, 43, 0},  // filter
  {PENTATONIC, 0},     // tuning
  {FLANGE, BYPASS, MEDIUM, BYPASS},       // effect
  {    // sequencers
    {{0, -10, 4, 12}, true, false, {true, false, true}, false}, 
    {{0, -8, 9, -6}, false, true, {false, true, false}, false}
  },
  {97, {2, 5, 0}, {3, 8, 0}, {3, 7, 0}},  // rhythm
  {BT_OFF, BT_OFF, {100, 101, 103, 102}}  // drums
};

// Dance
euclid_config preset6 = {
  {   // oscillators
    {40, PULSE, 77, false, 0, 4, SAWTOOTH, 15}, 
    {47, PULSE, 103, true, 0, 6, SAWTOOTH, 15}
  }, 
  {{127, 127}, {false, false}, {127, 127}, {false, false}},   // mixer
  {0, 127},             // env
  {TRIANGLE, 31},      // modulator
  {104, 62, 20, 68},  // filter
  {PENTATONIC_M, 0},     // tuning
  {NONE, BYPASS, BYPASS, BYPASS},       // effect
  {    // sequencers
    {{0, 5, 0, -5}, true, true, {true, false, false}, false}, 
    {{12, 8, -9, -24}, true, true, {false, false, false}, true}
  },
  {95, {4, 16, 0}, {5, 17, 1}, {3, 15, 4}},  // rhythm
  {BT_ALL, BT_ONE, {100, 101, 106, 102}}  // drums
};

// Stroll
euclid_config preset7 = {
  {   // oscillators
    {40, PULSE, 20, true, 0, 6, TRIANGLE, 10}, 
    {47, PULSE, 20, true, 0, 3, TRIANGLE, 10}
  }, 
  {{127, 127}, {false, false}, {127, 127}, {false, false}},   // mixer
  {2, 18},             // env
  {SQUARE, 31},      // modulator
  {102, 36, 18, 81},  // filter
  {PENTATONIC_M, 7},     // tuning
  {DELAY, BYPASS, BYPASS, SLOW},       // effect
  {    // sequencers
    {{7, 4, -3, 0}, true, true, {true, true, false}, false}, 
    {{12, -9, -12, -9}, true, true, {true, true, true}, false}
  },
  {84, {4, 32, 0}, {5, 29, 3}, {3, 31, 7}},  // rhythm
  {BT_ALL, BT_ONE, {100, 100, 100, 102}}  // drums
};

// Flap
euclid_config preset8 = {
  {   // oscillators
    {40, TRIANGLE, 0, false, 12, 5, SAWTOOTH, 30}, 
    {43, TRIANGLE, 0, false, 12, 5, SAWTOOTH, 30}
  }, 
  {{127, 127}, {false, false}, {127, 127}, {false, false}},   // mixer
  {0, 18},             // env
  {TRIANGLE, 0},      // modulator
  {102, 39, -64, 0},  // filter
  {PENTATONIC_M, 0},     // tuning
  {DELAY, BYPASS, BYPASS, SLOW},       // effect
  {    // sequencers
    {{-9, -6, 9, 14}, true, true, {true, false, false}, false}, 
    {{11, 9, 16, -18}, true, true, {false, true, false}, true}
  },
  {99, {4, 16, 0}, {5, 15, 2}, {3, 16, 3}},  // rhythm
  {BT_SEL, BT_SEL, {100, 101, 106, 102}}  // drums
};

// Bells
euclid_config preset9 = {
  {   // oscillators
    {40, SQUARE, 0, true, 0, 4, TRIANGLE, 15}, 
    {46, SQUARE, 0, true, 0, 6, TRIANGLE, 15}
  }, 
  {{127, 127}, {false, false}, {127, 127}, {false, false}},   // mixer
  {0, 71},             // env
  {SQUARE, 33},      // modulator
  {75, 56, 30, 87},  // filter
  {PENTATONIC_M, 12},     // tuning
  {DELAY, BYPASS, BYPASS, FAST},       // effect
  {    // sequencers
    {{0, 5, 0, -5}, true, true, {true, false, true}, false}, 
    {{13, 8, -6, -24}, true, true, {false, true, true}, false}
  },
  {47, {4, 14, 0}, {5, 15, 1}, {3, 17, 3}},  // rhythm
  {BT_ALL, BT_ONE, {100, 101, 101, 102}}  // drums
};

euclid_config default_presets[] = {
  preset0, preset1, preset2, preset3, preset4, preset5, preset6, preset7, preset8, preset9
};

const char *default_preset_names[] = {
  "Wah", "Pluck", "SciFi", "Meditation", "Echo", "Funk", "Dance", "Stroll", "Flap", "Bells"
};

const char *getROMPresetName(int i)
{
  if (i >= 0 && i < ROM_PRESETS)
    return default_preset_names[i];
  return "<none>";
}

bool saved_presets[MAX_PRESETS] = {false};

bool saveConfigData(File file, config_ptr cfg);
bool loadConfigData(File file, config_ptr cfg);

int nextPreset(int i)
{
  if (i == MAX_PRESETS)
    return SET_MANUAL; // indicates "set to manual" option
  if (i == SET_MANUAL)
    return SET_DEFAULT; // indicates "default" option
  else if (i == SET_DEFAULT)
    return nextPreset(-1);
  int j = i;
  while (++j < MAX_PRESETS)
  {
    if (saved_presets[j])
      return j;
  }
  return j; // indicates "save new preset" option
}

int prevPreset(int i)
{
  if (i == SET_DEFAULT)
    return SET_MANUAL; // indicates "set to manual" option
  else if (i == SET_MANUAL)
    return MAX_PRESETS; // indicates "save new preset" option
  else if (i == 0)
    return SET_DEFAULT; // indicates "default" option
  int j = i;
  while (--j >= 0)
  {
    if (saved_presets[j])
      return j;
  }
  return i;
}

int nextFreePreset()
{
  int j = ROM_PRESETS - 1;
  if (cardAvailable)
  {
    while (++j < MAX_PRESETS)
    {
      if (!saved_presets[j])
        return j;
    }
  }
  return -1; // indicates no slots available
}

void setupPresets()
{
  for (int i = 0; i < MAX_PRESETS; ++i)
  { 
    saved_presets[i] = i < ROM_PRESETS;
  }
  setupSDCard();
  if (!cardAvailable)
  {
    Serial.println("No SD card - ROM presets only");
  }
}

void copyFile(File src, File dest)
{
  size_t n;  
  uint8_t buffer[200];
  size_t buffersize = sizeof(buffer);  
  while ((n = src.read(buffer, buffersize)) > 0) 
  {
    dest.write(buffer, n);
  }
}

void printFile(File f)
{
  size_t n;  
  uint8_t buffer[200];
  size_t buffersize = sizeof(buffer);  
  Serial.print("Contents of file ");
  Serial.println(f.name());
  f.seek(0);
  while ((n = f.read(buffer, buffersize)) > 0) 
  {
    for (int i = 0; buffer[i] && i < 200; ++i)
      Serial.print((char)buffer[i]);
  }
  Serial.println("-------------------");
}

bool setupSDCard()
{
  // Configure SPI
  textatrow(2, "Checking SD card", LCD_BLACK, LCD_WHITE);
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  Sd2Card card;
  cardAvailable = card.init(SPI_FULL_SPEED, SDCARD_CS_PIN);
  if (cardAvailable && SD.begin(SDCARD_CS_PIN)) 
  {
    // Get patch numbers and names from SD card
    getPresets(SD.open("/"));
    updateBackup();
    textatrow(2, "SD card ready", LCD_BLACK, LCD_WHITE);
    Serial.println("SD card ok");
    return true;
  }
  else
  {
    textatrow(2, "SD card failed", LCD_BLACK, LCD_WHITE);
    Serial.println("SD card failed!");
    cardAvailable = false;
    return false;
  }
}

void updateBackup()
{
  if (checkForBackupPrevent())
  {
    textatrow(1, "Backup unaltered", LCD_BLACK, LCD_WHITE);
    Serial.println("Backup unaltered");
    return;
  }
  if (!cardAvailable)
    return;
  if (!SD.exists("backup"))
    SD.mkdir("backup");
  for (int i = ROM_PRESETS; i < MAX_PRESETS; ++i) 
  {
    sprintf(tempstr, "backup/%02d", i);
    if (SD.exists(tempstr))
      SD.remove(tempstr);
    if (saved_presets[i])
    {
      File dest = SD.open(tempstr, FILE_WRITE);
      sprintf(tempstr, "%02d", i - ROM_PRESETS);
      File src = SD.open(tempstr, FILE_READ);
      copyFile(src, dest);
      dest.close();
      src.close();    
    }
  }
}

bool isValidPresetFile(File f)
{
  if (f.isDirectory())
    return false;
  char *fname = f.name();
  return (strlen(fname) == 2) && 
    isdigit(fname[0]) && 
    isdigit(fname[1]);
}

bool isTempFile(char *fname)
{
  fname = strrchr(fname, '.');

  if (fname)
    return strcmp(fname, ".tmp") == 0;

  return false;
}

bool getPresets(File file) 
{
  if (!cardAvailable)
    return false;
    
  int i;
  int count = 0;
  for (i = ROM_PRESETS; i < MAX_PRESETS; ++i) 
    saved_presets[i] = false;
  while (true) 
  {
    File presetFile =  file.openNextFile();
    if (!presetFile)
      break;
    if (isValidPresetFile(presetFile))
    {
      int presetnum = atol(presetFile.name());
      saved_presets[ROM_PRESETS + presetnum] = true;
      count++;
    }
    presetFile.close();
    if (isTempFile(presetFile.name()))
    {
      SD.remove(presetFile.name());
    }
  }
//#ifdef DEBUG_SERIAL
  Serial.print(count);
  Serial.println(" presets found");
  printPresets();
//#endif

//  if (count == 0)
//  {
//    for (i = 0; i < NUM_DEFAULT_PRESETS; ++i) 
//    {
//      savePresetSD(i, &default_presets[i]);
//    }
//  } 

  return true;
}

void printPresets()
{
  int count = 0;
  for (int i = 0; i < MAX_PRESETS; ++i) 
  {
    if (i < ROM_PRESETS)
    {
      Serial.print(default_preset_names[i]);
      Serial.print(" ");
    }
    else if (saved_presets[i]) 
    {
      if (count++ % 20)
        Serial.print(" ");
      else
        Serial.println();
      Serial.print(i - ROM_PRESETS);
    }
  }
  Serial.println();
}

bool savePresetSD(int i, config_ptr cfg)
{
  if (!cardAvailable || i < ROM_PRESETS || i > MAX_PRESETS)
    return false;
  i -= ROM_PRESETS;
  sprintf(tempstr, "%02d", i);
  Serial.print("Saving preset ");Serial.print(i);
  Serial.print(" to file ");Serial.println(tempstr);
  SD.remove(tempstr);
  File f = SD.open(tempstr, FILE_WRITE);
  bool result = saveConfigData(f, cfg);
  f.close();
  saved_presets[i + ROM_PRESETS] = result;
  return result;
}

File tempFile;
bool getTempPresetFile(int i, File *file)
{
  if (!cardAvailable || i < ROM_PRESETS || i > MAX_PRESETS)
    return false;
  i -= ROM_PRESETS;
  sprintf(tempstr, "%02d.tmp", i);
  Serial.print("Opening preset file ");Serial.println(tempstr);
  SD.remove(tempstr);
  tempFile = SD.open(tempstr, FILE_WRITE);
  *file = tempFile;
  return true;
}

void convertTempPresetFile(int i, File *file)
{
  if (!cardAvailable || i < ROM_PRESETS || !file)
    return;
  i -= ROM_PRESETS;
  sprintf(tempstr, "%02d", i);
  if (SD.exists(tempstr))
    SD.remove(tempstr);
  File dest = SD.open(tempstr, FILE_WRITE);
  file->seek(0);  
  copyFile(*file, dest);
  dest.close();
  file->close();    
  saved_presets[i] = true;
  printPresets();
}

bool loadPresetSD(int i, config_ptr cfg)
{
  if (i < 0 || i > MAX_PRESETS)
    return false;
  if (i < ROM_PRESETS)
  {
    *cfg = default_presets[i];
    return true;
  }
  i -= ROM_PRESETS;
  if (!cardAvailable)
  {
    return false;
  }
  sprintf(tempstr, "%02d", abs(i) % 100);
  File f = SD.open(tempstr, FILE_READ);
  bool result = loadConfigData(f, cfg);
  f.close();
  return result;
}

bool deletePresetSD(int i)
{
  if (!cardAvailable || i < ROM_PRESETS || i >= MAX_PRESETS)
    return false;
  sprintf(tempstr, "%02d", i - ROM_PRESETS);
  if (SD.exists(tempstr))
  {
    bool result = SD.remove(tempstr);
    Serial.print(result);Serial.print(" - Removed ");Serial.println(tempstr);
    saved_presets[i] = false;
  }

  printPresets();
  return true;
}

/*
 * Format:
 * <version>    n
 * <osc 1>      freq,waveshape,pw,pwm,mod,detune,subfreq,subshape
 * <osc 2>      freq,waveshape,pw,pwm,mod,detune,subfreq,subshape
 * <mix>        o1,s1,o2,s2
 * <env>        a,d
 * <mod>        rate,shape
 * <filter>     co,res,envamt,modamt
 * <effects>    active,chtype,fltype,deltype
 * <tuning>     scale,transpose
 * <seq1>       s1,s2,s3,s4,appO,appS,b1,b2,b3,off
 * <seq2>       s1,s2,s3,s4,appO,appS,b1,b2,b3,off
 * <rhythm>     tempo,r11,r12,r13,r21,r22,r23,r31,r32,r33
 * <drums>      enabled1,enabled2,kit1,kit2,kit3,offbeat
 */
#define VERSION 1.0

static char line[100];

bool saveConfigData(File file, config_ptr cfg)
{
  file.println(VERSION);
  Serial.println(VERSION);
  oscillatorCfg o = cfg->osc[0];
  sprintf(line, "%f,%d,%d,%d,%d,%d,%d,%d", o.freq, o.shape, o.pwamt, o.pwm, o.modamt, o.detune, o.subfreq, o.subshape);
  file.println(line);
  Serial.println(line);
  o = cfg->osc[1];
  sprintf(line, "%f,%d,%d,%d,%d,%d,%d,%d", o.freq, o.shape, o.pwamt, o.pwm, o.modamt, o.detune, o.subfreq, o.subshape);
  file.println(line);
  Serial.println(line);
  mixerCfg m = cfg->mix;
  sprintf(line, "%d,%d,%d,%d,%d,%d,%d,%d", m.osclevel[0], m.osclevel[1], m.oscmute[0], m.oscmute[1],
    m.sublevel[0], m.sublevel[1], m.submute[0], m.submute[1]);
  file.println(line);
  Serial.println(line);
  envelopeCfg e = cfg->env;
  sprintf(line, "%d,%d", e.attack, e.decay);
  file.println(line);
  Serial.println(line);
  modulatorCfg l = cfg->lfo;
  sprintf(line, "%d,%d", l.shape, l.rate);
  file.println(line);
  Serial.println(line);
  filterCfg f = cfg->filter;
  sprintf(line, "%d,%d,%d,%d", f.cutoff, f.resonance, f.envamt, f.modamt);
  file.println(line);
  Serial.println(line);
  effectsCfg eff = cfg->eff;
  sprintf(line, "%d,%d,%d,%d", eff.active, eff.chorus, eff.flange, eff.delay);
  file.println(line);
  Serial.println(line);
  tuningCfg t = cfg->tuning;
  sprintf(line, "%d,%d", t.scale, t.transpose);
  file.println(line);
  Serial.println(line);
  sequencerCfg s = cfg->seq[0];
  sprintf(line, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", s.steps[0], s.steps[1], s.steps[2], s.steps[3],
      s.applyOsc, s.applySub, s.advBeat[0], s.advBeat[1], s.advBeat[2], s.advOffbeat);
  file.println(line);
  Serial.println(line);
  s = cfg->seq[1];
  sprintf(line, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", s.steps[0], s.steps[1], s.steps[2], s.steps[3],
      s.applyOsc, s.applySub, s.advBeat[0], s.advBeat[1], s.advBeat[2], s.advOffbeat);
  file.println(line);
  Serial.println(line);
  rhythmCfg r = cfg->rhythm;
  sprintf(line, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", r.tempo, 
    r.r1[0], r.r1[1], r.r1[2], r.r2[0], r.r2[1], r.r2[2], r.r3[0], r.r3[1], r.r3[2]);
  file.println(line);
  Serial.println(line);
  drumCfg d = cfg->drum;
  sprintf(line, "%d,%d,%d,%d,%d,%d", d.beatstate, d.offbeatstate, d.beats[0], d.beats[1], d.beats[2], d.beats[3]);
  file.println(line);
  Serial.println(line);
  return true;
}

float bits[10];

int handleOneLine(File file)
{
  int i = 0;
  int j = 0;
  while (true)
  {
    char c = file.read();
    if (c == ',' || c == -1 || c == '\n')
    {
      line[i++] = '\0';
      bits[j] = (float)atof(line);
      j++;
      if (c == -1 || c == '\n')
        break;
      else
      {
        i = 0;
        continue;
      }
    }
    line[i++] = c;
    if (i > 49)
      return -1;  // something is wrong
  }
  return j;
}

float floatRange(float v, int minval, int maxval)
{
  return min(maxval, max(minval, v));
}

int intRange(float v, int minval, int maxval)
{
  return (int)floatRange(v, minval, maxval);
}

bool loadConfigData(File file, config_ptr cfg)
{
  int count = handleOneLine(file);
  if (count != 1 || abs(bits[0] - VERSION) > 0.00001)
  {
    Serial.println("Invalid file - wrong version number");
    return false;
  }

  int i, k;
  for (i = 0; i < 2; ++i)
  {
    oscillatorCfg *o = &cfg->osc[i];
    count = handleOneLine(file);
    if (count != 8)
    {
      Serial.println("Invalid file - oscillator config");
      return false;
    }
    o->freq = floatRange(bits[0], 40, 88);
    o->shape = (waveshape)intRange(bits[1], 0, 7);
    o->pwamt = intRange(bits[2], 0, 127);
    o->pwm = bits[3] ? true : false;
    o->modamt = intRange(bits[4], 0, 127);
    o->detune = intRange(bits[5], 0, 50);
    o->subfreq = intRange(bits[6], 2, 16);
    o->subshape = (waveshape)intRange(bits[7], 0, 7);
  }
  
  mixerCfg *m = &cfg->mix;
  count = handleOneLine(file);
  if (count != 8)
  {
    Serial.println("Invalid file - mixer config");
    return false;
  }
  m->osclevel[0] = intRange(bits[0], 0, 127);
  m->osclevel[1] = intRange(bits[1], 0, 127);
  m->oscmute[0] = bits[2] ? true : false;
  m->oscmute[1] = bits[3] ? true : false;
  m->sublevel[0] = intRange(bits[4], 0, 127);
  m->sublevel[1] = intRange(bits[5], 0, 127);
  m->submute[0] = bits[6] ? true : false;
  m->submute[1] = bits[7] ? true : false;

  envelopeCfg *e = &cfg->env;
  count = handleOneLine(file);
  if (count != 2)
  {
    Serial.println("Invalid file - envelope config");
    return false;
  }
  e->attack = intRange(bits[0], 0, 127);
  e->decay = intRange(bits[1], 0, 127);
  
  modulatorCfg *l = &cfg->lfo;
  count = handleOneLine(file);
  if (count != 2)
  {
    Serial.println("Invalid file - modulator config");
    return false;
  }
  l->shape = (waveshape)intRange(bits[0], 0, 7);
  l->rate = intRange(bits[1], 0, 127);
  
  filterCfg *f = &cfg->filter;
  count = handleOneLine(file);
  if (count != 4)
  {
    Serial.println("Invalid file - filter config");
    return false;
  }
  f->cutoff = intRange(bits[0], 0, 127);
  f->resonance = intRange(bits[1], 0, 127);
  f->envamt = intRange(bits[2], -64, 63);
  f->modamt = intRange(bits[3], 0, 127);
  
  effectsCfg *eff = &cfg->eff;
  count = handleOneLine(file);
  if (count != 4)
  {
    Serial.println("Invalid file - effects config");
    return false;
  }
  eff->active = (effect)intRange(bits[0], 0, 4);
  eff->chorus = (effect_preset)intRange(bits[1], 0, 3);
  eff->flange = (effect_preset)intRange(bits[2], 0, 3);
  eff->delay = (effect_preset)intRange(bits[3], 0, 3);
  
  tuningCfg *t = &cfg->tuning;
  count = handleOneLine(file);
  if (count != 2)
  {
    Serial.println("Invalid file - tuning config");
    return false;
  }
  t->scale = (quantisation)intRange(bits[0], 0, 5);
  t->transpose = intRange(bits[1], -24, 24);
      
  for (i = 0; i < 2; ++i)
  {
    sequencerCfg *s = &cfg->seq[i];
    count = handleOneLine(file);
    if (count != 10)
    {
      Serial.println("Invalid file - sequencer config");
      Serial.print("Count is ");
      Serial.println(count);
      return false;
    }
    for (k = 0; k < 4; ++k)
      s->steps[k] = intRange(bits[k], -24, 24);
    s->applyOsc = bits[4] ? true : false;
    s->applySub = bits[5] ? true : false;
    for (k = 0; k < 3; ++k)
      s->advBeat[k] = bits[6 + k] ? true : false;
    s->advOffbeat = bits[9] ? true : false;
  }
  
  rhythmCfg *r = &cfg->rhythm;
  count = handleOneLine(file);
  if (count != 10)
  {
    Serial.println("Invalid file - rhythm config");
    return false;
  }
  r->tempo = intRange(bits[0], 0, 127);
  r->r1[0] = intRange(bits[1], 1, 32);
  r->r1[1] = intRange(bits[2], r->r1[0], 32);
  r->r1[2] = intRange(bits[3], 0, 31);
  r->r2[0] = intRange(bits[4], 1, 32);
  r->r2[1] = intRange(bits[5], r->r2[0], 32);
  r->r2[2] = intRange(bits[6], 0, 31);
  r->r3[0] = intRange(bits[7], 1, 32);
  r->r3[1] = intRange(bits[8], r->r3[0], 32);
  r->r3[2] = intRange(bits[9], 0, 31);

  drumCfg *d = &cfg->drum;
  count = handleOneLine(file);
  if (count != 6)
  {
    Serial.println("Invalid file - drum config");
    return false;
  }
  d->beatstate = (beatState)intRange(bits[0], 0, 3);
  d->offbeatstate = (beatState)intRange(bits[1], 0, 3);
  d->beats[0] = intRange(bits[2], 0, DK_LAST);
  d->beats[1] = intRange(bits[3], 0, DK_LAST);
  d->beats[2] = intRange(bits[4], 0, DK_LAST);
  d->beats[3] = intRange(bits[5], 0, DK_LAST);

  return true;
}

void restorePresetSD(int i)
{
  if (!cardAvailable)
  {
    Serial.println("No SD card available");
    return;
  }
  if (!SD.exists("backup"))
  {
    Serial.println("No backup folder on SD card");
    return;
  }
  sprintf(tempstr, "backup/%02d", i);
  if (SD.exists(tempstr))
  {
    File backup = SD.open(tempstr, FILE_READ);
    sprintf(tempstr, "%02d", i);
    SD.remove(tempstr);
    File dest = SD.open(tempstr, FILE_WRITE);
    while (backup.available()) 
      dest.write(backup.read());
    backup.close();
    dest.close(); 
    saved_presets[i] = true;
  }
}

void dumpPresetSD(int i)
{
  if (!cardAvailable)
  {
    Serial.println("No SD card available");
    return;
  }
  sprintf(tempstr, "%02d", i);
  File f = SD.open(tempstr, FILE_READ);
  if (f)
  {
    Serial.println("###########################");
    Serial.print("###   Preset file: ");
    Serial.print(f.name());
    Serial.println("   ###");
    Serial.println("###########################");
    while (f.available()) 
    {
      Serial.write(f.read());
    }
    f.close();
    Serial.println("###########################");
  }
  else
  {
    Serial.print("No file: ");
    Serial.println(tempstr);
  }
}

void dumpAllPresetsSD()
{
  for (int i = ROM_PRESETS; i < MAX_PRESETS; ++i) 
  {
    if (saved_presets[i])
      dumpPresetSD(i - ROM_PRESETS);
  }
}

void restoreFromBackupSD()
{
  if (!cardAvailable)
  {
    Serial.println("No SD card available");
    return;
  }
  if (!SD.exists("backup"))
  {
    Serial.println("No backup available");
  }
  for (int i = ROM_PRESETS; i < MAX_PRESETS; ++i) 
  {
    i -= ROM_PRESETS;
    sprintf(tempstr, "%02d", i);
    if (SD.exists(tempstr))
      SD.remove(tempstr);
    sprintf(tempstr, "backup/%02d", i);
    if (SD.exists(tempstr))
    {
      saved_presets[i + ROM_PRESETS] = true;
      File src = SD.open(tempstr, FILE_READ);
      sprintf(tempstr, "%02d", i);
      File dest = SD.open(tempstr, FILE_WRITE);
      copyFile(src, dest);
      dest.close();
      src.close();    
    }
    else
      saved_presets[i + ROM_PRESETS] = false;
  }
}
