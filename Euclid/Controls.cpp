#define USE_ADC_LIB 1

#include <IntervalTimer.h>

#if USE_ADC_LIB
#include <ADC.h>
#endif

#include "Adafruit_MCP23017.h"
#include "Rotary.h"
#include <Bounce2mcp.h>
#include "LCD.h"

#include "Config.h"
#include "Utility.h"
#include "Sequencer.h"
#include "Synth.h"
#include "Presets.h"
#include "Controls.h"
#include "MIDIManager.h"
#include "Drums.h"

#if USE_ADC_LIB
ADC *adc = new ADC(); // adc object;
#endif

static IntervalTimer shifttimer;

#if PROTOTYPE     
#else
static bool shift = false;
#endif

static bool metershift = true;
static volatile bool toggleshiftled = false;
static bool selectledstate = true;

int selected = 0;

void shiftflashcallback()
{
  toggleshiftled = !toggleshiftled;
}

#define SELECT_O1   0
#define SELECT_O2   1
#define SELECT_OS   2
#define SELECT_ENV  3
#define SELECT_MIX  4
#define SELECT_F    5

int preset = SET_DEFAULT;
bool enablePresetButton = false;  // must have been turned first
bool saveNewPresetSelected = false;
int lastupdatemillis = 0;
int activemeter = 0;

bool presetMenuMode = false;
int presetMenuSelect = -1;
void showPresetLabel(int p);

long transposeTime = -1;
#if PROTOTYPE
bool transposeMode = false;
#endif
int transpose;
void applyTranspose(unsigned long curmillis);

void setToCurTempo();

#define UPDATE_WAIT_TIME 3000
#define METER_UPDATE_WAIT_TIME 100

bool demoMode = false;
#define DEMO_TIME_PER_PRESET 8
  
static char tempstr[50] = {0};

Adafruit_MCP23017 *expanders[] = {NULL, NULL, NULL, NULL};

typedef struct
{
  int expander;
  int pin;
  int led1pin;
  int led2pin;
  BounceMcp *debouncer;
} pushswitch;

#define NUM_SEQ_SWITCHES 10
#define NUM_SWITCHES 14

pushswitch switches[] = 
{
  {2, 0, 1, -1, NULL},   // seq 1 beat 1
  {2, 2, 3, -1, NULL},   // seq 1 beat 2
  {2, 4, 5, -1, NULL},   // seq 1 beat 3
  {2, 6, 7, -1, NULL},   // seq 1 offbeat
  {2, 8, 9, 10, NULL},   // seq 1 apply
  
  {3, 0, 1, -1, NULL},   // seq 2 beat 1
  {3, 2, 3, -1, NULL},   // seq 2 beat 2
  {3, 4, 5, -1, NULL},   // seq 2 beat 3
  {3, 6, 7, -1, NULL},   // seq 2 offbeat
  {3, 8, 9, 10, NULL},   // seq 2 apply
  
  {1,  9,  8, -1, NULL},   // pause
  {1, 11, 10, -1, NULL},   // hold
  {1, 12, -1, -1, NULL},   // stop // FIXME - button doesn't register if ledpin set to -1
  {1, 13, 14, -1, NULL},   // run
};

#define STEP_LED 8
#define HOLD_LED 10
#define RUN_LED 14

typedef struct
{
  int expander;
  int seq;
  int beat;
  int pin;
} sequencerled;

sequencerled seqleds[] = 
{
  {2, 1, 0, 11},
  {2, 1, 1, 12},
  {2, 1, 2, 13},
  {2, 1, 3, 14},
  {3, 2, 0, 11},
  {3, 2, 1, 12},
  {3, 2, 2, 13},
  {3, 2, 3, 14},
};

typedef struct
{
  int expander;
  int pin;
} selectled;

selectled selleds[] = 
{
  {0, 11},
  {0, 15},
  {1,  3},
  {1,  7},
  {1, 15},
};

#define NUM_ENCODERS 7

encoder encoders[] = {
#if PROTOTYPE
  {0,  0,  1, -1, 0x0003, false, R_START, {0, 0, 0, 0, 0, 0}, {4, 4, 4, 4, 4, 4}, NULL },
  // Freq, Quantise, Osc 1 level, Cutoff
  {0,  2,  3,  4, 0x000C, false, R_START, {40, 40, 40, 0, 0, 0}, {76, 76, 76, 10, 127, 127}, NULL },
  // PW, Attack, Sub 1 level, Res
  {0,  5,  6,  7, 0x0060, false, R_START, {0, 0, 0, 0, 0, 0}, {127, 127, 127, 127, 127, 127}, NULL },
  // Mod amt, Decay, Osc 2 level, Env amt
  {0,  8,  9, 10, 0x0300, false, R_START, {0, 0, 0, 0, 0, -64}, {127, 127, 127, 127, 127, 63},  NULL },
  // Sub-freq, Rate, Sub 2 level, Mod amt
  {0, 12, 13, 14, 0x6000, false, R_START, {2, 2, 2, 0, 0, 0}, {16, 16, 16, 127, 127, 127}, NULL },
#else
  {0,  0,  1,  4, 0x0003, false, R_START, {0, 0, 0, 0, 0, 0}, {4, 4, 4, 4, 4, 4}, NULL },
  // Freq, Quantise, Osc 1 level, Cutoff
  {0,  2,  3, -1, 0x000C, false, R_START, {40, 40, 40, 0, 0, 0}, {76, 76, 76, 10, 127, 127}, NULL },
  // PW, Attack, Sub 1 level, Res
  {0,  5,  6, -1, 0x0060, false, R_START, {0, 0, 0, 0, 0, 0}, {127, 127, 127, 127, 127, 127}, NULL },
  // Mod amt, Decay, Osc 2 level, Env amt
  {0,  8,  9, -1, 0x0300, false, R_START, {0, 0, 0, 0, 0, -64}, {127, 127, 127, 127, 127, 63},  NULL },
  // Sub-freq, Rate, Sub 2 level, Mod amt
  {0, 12, 13, -1, 0x6000, false, R_START, {2, 2, 2, 0, 0, 0}, {16, 16, 16, 127, 127, 127}, NULL },
#endif
  {1,  0,  1,  2, 0x0003, false, R_START, {1, 1, 1, 1, 1, 1}, {32, 32, 32, 32, 32, 32}, NULL }, // meter
  {1,  4,  5,  6, 0x0030, false, R_START, {0, 0, 0, 0, 0, 0}, {5, 5, 5, 5, 5, 5}, NULL } // preset
};

#define SELECT_ENC 0
#define METER_ENC 5
#define PRESET_ENC 6

pot pots[] = {
  {15, 0, 0, 0,  true, NO_ADJUST, 0, false},
  {14, 0, 0, 0,  true, NO_ADJUST, 0, false},
  {13, 0, 0, 0,  true, NO_ADJUST, 0, false},
  {12, 0, 0, 0,  true, NO_ADJUST, 0, false},
  {11, 0, 0, 0,  true, NO_ADJUST, 0, false},
  {10, 0, 0, 0,  true, NO_ADJUST, 0, false},
  { 9, 0, 0, 0,  true, NO_ADJUST, 0, false},
  { 8, 0, 0, 0,  true, NO_ADJUST, 0, false},
  { 7, 0, 0, 0, false, NO_ADJUST, 0, false},
  { 6, 0, 0, 0, false, NO_ADJUST, 0, false},
  { 5, 0, 0, 0, false, NO_ADJUST, 0, false},
};

#if PROTOTYPE
#define NUM_POTS 10
#else
#define NUM_POTS 11
#endif
#define INIT_POT_VAL -1000
#define POTLOOPSET  16
#define POTLOOPREAD 6
#define POTLOOPSKIP (POTLOOPSET + POTLOOPREAD)

#if USE_ADC_LIB
#define MAXRAWVAL 4095
#else
#define MAXRAWVAL 1023
#endif
#define MAXPOTVAL 1023
#define ZERO_RANGE 50
const float CENTRE = MAXPOTVAL / 2;
const float SCALE = CENTRE / (MAXPOTVAL - CENTRE - ZERO_RANGE);

#define S1STEP1_POT 0
#define S1STEP2_POT 1
#define S1STEP3_POT 2
#define S1STEP4_POT 3
#define S2STEP1_POT 4
#define S2STEP2_POT 5
#define S2STEP3_POT 6
#define S2STEP4_POT 7
#define TEMPO_POT 8
#define VOLUME_POT 9
#define DRUM_VOL_POT 10
 
uint16_t blockMillis = 0;

void setupExpanders()
{
  int i, j;
  Adafruit_MCP23017 *expander;
  for (i = 0; i < 4; ++i)
  {
    expander = new Adafruit_MCP23017();
    expander->begin(i);
    expanders[i] = expander;
  }
  for (j = 0; j < NUM_ENCODERS; ++j)
  {
    encoder *enc = &encoders[j];
    expander = expanders[enc->expander];
    expander->pinMode(enc->pinA, INPUT);
    expander->pullUp(enc->pinA, HIGH); // turn on a 100K pullup internally
    expander->pinMode(enc->pinB, INPUT);
    expander->pullUp(enc->pinB, HIGH); // turn on a 100K pullup internally
    if (enc->pinSW >= 0)
    {
      expander->pinMode(enc->pinSW, INPUT);
      expander->pullUp(enc->pinSW, HIGH); // turn on a 100K pullup internally
      enc->debouncer = new BounceMcp();
      enc->debouncer->attach(*expander, enc->pinSW, blockMillis);
    }
  }
  for (j = 0; j < NUM_SWITCHES; ++j)
  {
    pushswitch *sw = &switches[j];
    expander = expanders[sw->expander];
    expander->pinMode(sw->pin, INPUT);
    expander->pullUp(sw->pin, HIGH); // turn on a 100K pullup internally
    sw->debouncer = new BounceMcp();
    sw->debouncer->attach(*expander, sw->pin, blockMillis);
    if (sw->led1pin >= 0)
      expander->pinMode(sw->led1pin, OUTPUT);
    if (sw->led2pin >= 0)
      expander->pinMode(sw->led2pin, OUTPUT);
  }
  for (j = 0; j < 8; ++j)
  {
    sequencerled seqled = seqleds[j];
    expander = expanders[seqled.expander];
    expander->pinMode(seqled.pin, OUTPUT);
    expander->digitalWrite(seqled.pin, LOW);
  }
  for (j = 0; j < 5; ++j)
  {
    selectled selled = selleds[j];
    expander = expanders[selled.expander];
    expander->pinMode(selled.pin, OUTPUT);
    expander->digitalWrite(selled.pin, LOW);
  }
}

static int lastencoder = -1;
static long lastencodermillis = -1;

#define MIN_ENC_TIME 50
#define MAX_ENC_TIME 200
#define MAX_ENC_ACCEL 8

void handleEncoderShiftTurn(int i, bool back);

void checkEncoders(unsigned long now)
{
  uint16_t vals[2];
  vals[0] = expanders[0]->readGPIOAB();
  vals[1] = expanders[1]->readGPIOAB();
//  Serial.print("Expanders: ");Serial.print(vals[0], BIN);Serial.print(" ");Serial.print(vals[1], BIN);Serial.println();
  for (int i = 0; i < NUM_ENCODERS; ++i)
  {
    encoder *e = &encoders[i]; 
    uint16_t val = vals[e->expander];
    val >>= e->pinA;
    int p0 = val & 0x0001;
    int p1 = (val & 0x0002) >> 1;
    uint8_t pinstate = (p0 << 1) | p1;  
    if (e->halfstate)
      e->_state = ttable_half[e->_state & 0xf][pinstate];
    else
      e->_state = ttable[e->_state & 0xf][pinstate]; 
    int d;      
    if ((e->_state & 0x30) == DIR_CW)
      d = 1;
    else if ((e->_state & 0x30) == DIR_CCW)
      d = -1;
    else
      d = 0;
    bool pressed = false;
    if (e->debouncer)
    {
      e->debouncer->update();
      pressed = e->debouncer->fell();
    }

    // add some acceleration for the oscillator shape encoder    
    if (d)
    {
      bool accel = false;
      if (i > 0 && i < 5)
      {
        switch (selected)
        {
          case SELECT_O1:
          case SELECT_O2:
          case SELECT_OS:
            accel = (i != 4);  // don't accelerate subfrequency
            break;
          case SELECT_ENV:
            accel = (i != 1);  // don't accelerate quantise
            break;
          case SELECT_MIX:
          case SELECT_F:
            accel = true;
            break;
        }
      }

      if (accel)
      {
        if (lastencoder != i)
        {
          lastencoder = i;
          lastencodermillis = -1;
        }
        else 
        {
          lastencoder = i;
          long delta = now - lastencodermillis;
          int factor;
          if (delta >= MAX_ENC_TIME)
            factor = 1;
          else if (delta <= MIN_ENC_TIME)
            factor = MAX_ENC_ACCEL;
          else
            factor = MAX_ENC_ACCEL - round(1.0 * (delta  - MIN_ENC_TIME) / (MAX_ENC_TIME - MIN_ENC_TIME) * (MAX_ENC_ACCEL - 1));
    //      Serial.print("  ### Encoder timing ");Serial.print(delta);Serial.print(" -> ");Serial.println(factor);
          d *= factor;
          lastencodermillis = now;
        }
      }
#if PROTOTYPE      
      if (selected != SELECT_ENV || i != 1)
      {
        if (transposeMode)
        {
          transposeMode = false;
//          textatrow(1, "Transpose mode", LCD_BLACK, LCD_WHITE);
//          textatrow(2, "Off", LCD_BLACK, LCD_WHITE);
        }
      }
#endif      
      handleEncoderTurn(i, d);
      lastupdatemillis = now;
    }
    else if (pressed)
    {
      handleEncoderPress(i);
      lastupdatemillis = now;
    }
  }
}

void updateActiveMeter(int d)
{
  rhythmCfg *r = &config->rhythm;
  switch (activemeter)
  {
    case 0:
      r->r1[0] = max(min(r->r1[0] + d, r->r1[1]), 1);
      break;
    case 1:
      r->r1[1] = max(min(r->r1[1] + d, 32), r->r1[0]);
      break;
    case 2:
      r->r1[2] = max(min(r->r1[2] + d, r->r1[1] - 1), 0);
      break;
    case 3:
      r->r2[0] = max(min(r->r2[0] + d, r->r2[1]), 1);
      break;
    case 4:
      r->r2[1] = max(min(r->r2[1] + d, 32), r->r2[0]);
      break;
    case 5:
      r->r2[2] = max(min(r->r2[2] + d, r->r2[1] - 1), 0);
      break;
    case 6:
      r->r3[0] = max(min(r->r3[0] + d, r->r3[1]), 1);
      break;
    case 7:
      r->r3[1] = max(min(r->r3[1] + d, 32), r->r3[0]);
      break;
    case 8:
      r->r3[2] = max(min(r->r3[2] + d, r->r3[1] - 1), 0);
      break;
    case 9:
      r->r1[1] = max(min(r->r1[1] + d, 32), 1);
      r->r2[1] = r->r1[1];
      r->r3[1] = r->r1[1];
      if (r->r1[0] > r->r1[1])
        r->r1[0] = r->r1[1];
      if (r->r2[0] > r->r2[1])
        r->r2[0] = r->r2[1];
      if (r->r3[0] > r->r3[1])
        r->r3[0] = r->r3[1];
      if (r->r1[2] > r->r1[1])
        r->r1[2] = 0;
      if (r->r2[2] > r->r2[1])
        r->r2[2] = 0;
      if (r->r3[2] > r->r3[1])
        r->r3[2] = 0;
      break;
  }
  showRhythm(activemeter, r, metershift);
  poly->regenerate(r);
  seq1->update(false);
  seq2->update(false);
}

void handleFreq(int i, encoder *e, int d, bool dual1, bool dual2)
{
  // range is MIDI note 40 to 76
  float step = config->tuning.scale == FREE ? 0.1 : 1;
  float val = max(e->minval[i], min(e->maxval[i], config->osc[i].freq + d * step));
  float f = noteToFreq(val);
#if DEBUG_SERIAL          
  Serial.print("Oscillator ");Serial.print(i + 1);Serial.print(": Freq ");Serial.print(val);Serial.print(" -> ");Serial.println(f);
#endif          
  config->osc[i].freq = val;
  synth->loadOscConfig(&config->osc[i], i);
  if (synth->droning)
    showDroningFrequencies();
  else if (poly->running && poly->pause)
    showPauseInfo();
  else
  {
    int fline = dual1 ? 1 : 2;
    if (f > 1000)
      sprintf(tempstr, "%s%.2f kHz (%s)", dual1 ? "1. " : (dual2 ? "2. " : ""), f / 1000, notestring((int)round(val)));
    else
      sprintf(tempstr, "%s%.1f Hz (%s)", dual1 ? "1. " : (dual2 ? "2. " : ""), f, notestring((int)round(val)));
    textatrow(fline, tempstr, LCD_BLACK, LCD_WHITE);
    if (!dual1 && !dual2)
    {
      sprintf(tempstr, "Osc %d frequency", i + 1);
      textatrow(1, tempstr, LCD_BLACK, LCD_WHITE);
    }
  }
}

void handleSubFreq(int i, encoder *e, int d, bool dual1, bool dual2)
{
  config->osc[i].subfreq = max(e->minval[i], min(e->maxval[i], config->osc[i].subfreq  + d));
  synth->loadOscConfig(&config->osc[i], i);
  float f = noteToFreq(config->osc[i].freq);
  float subf = f / config->osc[i].subfreq;
  sprintf(tempstr, "  /%d %dHz (%s)", config->osc[i].subfreq, (int)subf, notestring(freqToNote(subf)));
  if (synth->droning)
    showDroningFrequencies();
  else if (poly->running && poly->pause)
    showPauseInfo();
  else
  {
    int fline = dual1 ? 1 : 2;
    if (subf > 1000)
      sprintf(tempstr, "%s /%d %.2fkHz (%s)", dual1 ? "1." : (dual2 ? "2." : ""), config->osc[i].subfreq, subf / 1000, notestring(freqToNote(subf)));
    else
      sprintf(tempstr, "%s /%d %dHz (%s)", dual1 ? "1." : (dual2 ? "2." : ""), config->osc[i].subfreq, (int)round(subf), notestring(freqToNote(subf)));
    textatrow(fline, tempstr, LCD_BLACK, LCD_WHITE);
    if (!dual1 && !dual2)
    {
      sprintf(tempstr, "Osc %d subfreq", i + 1);
      textatrow(1, tempstr, LCD_BLACK, LCD_WHITE);
    }
  }
#if DEBUG_SERIAL
  Serial.print("Oscillator ");Serial.print(i + 1);Serial.print(": Sub freq ");Serial.println(config->osc[i].subfreq);
#endif   
}

void handleEncoderTurn(int i, int d)
{
#if PROTOTYPE
#else
  if (d && shift && i != SELECT_ENC && i != METER_ENC && i != PRESET_ENC)
  {
    handleEncoderShiftTurn(i, d < 0);
    return;
  }
#endif
  
  encoder *e = &encoders[i];
  switch (i)
  {
    case SELECT_ENC:
//      shifttimer.end();
//      selectledstate = true;
//      shift = false;
      selected += d;
      selected = (selected + NUM_SELECT) % NUM_SELECT; 
      setSelectLed(selected);
      showSelectSummary(selected);
      break;
    case PRESET_ENC:
      enablePresetButton = true;
      saveNewPresetSelected = false;
      if (presetMenuMode)
      {
        showPresetLabel(preset);
        presetMenuSelect = (presetMenuSelect + 4 + d) % 4;
        if (presetMenuSelect == 1)
          textatrow(2, "Load?", LCD_BLACK, LCD_WHITE);
        else if (presetMenuSelect == 2)
          textatrow(2, "Save?", LCD_BLACK, LCD_WHITE);
        else if (presetMenuSelect == 3)
          textatrow(2, "Delete?", LCD_BLACK, LCD_WHITE);          
        else
          textatrow(2, "Cancel?", LCD_BLACK, LCD_WHITE);
      }
      else
      {
        preset = d > 0 ? nextPreset(preset) : prevPreset(preset);
        if (preset == SET_MANUAL)
          sprintf(tempstr, "Set to manual");
        else if (preset == SET_DEFAULT)
          sprintf(tempstr, "Init settings");
        else if (preset == MAX_PRESETS)
        {
          sprintf(tempstr, "Save new preset");
          saveNewPresetSelected = true;
        }
        else if (preset < ROM_PRESETS)
          sprintf(tempstr, "ROM: %s", getROMPresetName(preset));
        else
          sprintf(tempstr, "User: %02d", preset - ROM_PRESETS);
        textatrow(1, tempstr, LCD_BLACK, LCD_WHITE);
        textatrow(2, "", LCD_BLACK, LCD_WHITE);
      }
      break;
    case METER_ENC:
    {
      if (metershift)
      {
        activemeter += (d > 0 ? 1 : -1);
        if (activemeter > 9)
          activemeter = 0;
        else if (activemeter < 0)
          activemeter = 9;
        lastencoder = METER_ENC;
        showRhythm(activemeter, &config->rhythm, metershift);
      }
      else
      {
        lastencoder = METER_ENC;
        updateActiveMeter(d);
      }
      break;
    }
    case 1: // Freq, Quantise, Osc 1 level, Cutoff
      switch (selected)
      {
        case SELECT_O1:
        case SELECT_O2: // Frequency
          handleFreq(selected, e, d, false, false);
          break;
        case SELECT_OS: // Both frequencies
          handleFreq(0, e, d, true, false);
          handleFreq(1, e, d, false, true);
          break;
        case SELECT_ENV: // Quantisation
        {
#if PROTOTYPE          
          if (transposeMode)
          {
            transpose += d;
            transpose = min(24, max(-24, transpose));
            textatrow(1, "Transpose mode", LCD_BLACK, LCD_WHITE);
            if (synth->droning || poly->pause) // immediate update if droning or paused
            {
              config->tuning.transpose = transpose;
              synth->globaloffset(config->tuning.transpose, true);
            }
            else
            {
              sprintf(tempstr, "On [%+2d]", transpose);
              textatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
            }
          }
          else
#endif          
          {
            int quant = config->tuning.scale + d;
            quant = (quant + e->maxval[selected]) % e->maxval[selected]; 
            config->tuning.scale = (quantisation)quant;
  #if DEBUG_SERIAL
            Serial.print("Quantisation ");Serial.println(quantisationStr(config->tuning.scale, true));
  #endif          
            if (poly->running)
            {
              textatrow(1, quantisationStr(config->tuning.scale, true), LCD_BLACK, LCD_WHITE);
              textatrow(2, "", LCD_BLACK, LCD_WHITE);
            }
            else
            {
              showSelectSummary(3);
              showTuning(&config->tuning);
            }
          }
          break;
        }
        case SELECT_MIX: // Osc 1 level
        {
          config->mix.osclevel[0] = max(e->minval[selected], min(e->maxval[selected], config->mix.osclevel[0] + d));
          synth->loadMixerConfig(&config->mix);
          textatrow(1, "Osc 1 level", LCD_BLACK, LCD_WHITE);
          sprintf(tempstr, "%5d %s", config->mix.osclevel[0], config->mix.oscmute[0] ? "(muted)" :"");
          textatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
#if DEBUG_SERIAL
          Serial.print("Osc 1 level ");Serial.println(config->mix.osclevel[0]);
#endif          
          break;
        }
        case SELECT_F: // Cutoff
        {
#if PROTOTYPE
          // hacked in drum volume control
          pushswitch *runsw = &switches[RUN];
          Adafruit_MCP23017 *expander = expanders[runsw->expander];
          if (expander->digitalRead(runsw->pin) == LOW)
          {
            float vol = adjustDrumsVolume(d);
            textatrow(1, "Drums volume", LCD_BLACK, LCD_WHITE);
            sprintf(tempstr, "%.2f", vol);
            textatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
            break;
          } 
#endif        
          config->filter.cutoff = max(e->minval[selected], min(e->maxval[selected], config->filter.cutoff + d));
          synth->loadFilterConfig(&config->filter);
          textatrow(1, "Cutoff frequency", LCD_BLACK, LCD_WHITE);
          float co = (int)synth->getCutoffFreq();
          if (co >= 1000)
            sprintf(tempstr, "%3d (%.2fkHz)", config->filter.cutoff, co / 1000);
          else
            sprintf(tempstr, "%5d (%dHz)", config->filter.cutoff, (int)co);
          textatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
#if DEBUG_SERIAL
          Serial.print("Cutoff ");Serial.println(config->filter.cutoff);
#endif          
          break;
        }
      }
      break;
    case 2: // PW, Attack, Sub 1 level, Res
      switch (selected)
      {
        case SELECT_O1:
        case SELECT_O2: // PW
        {
          config->osc[selected].pwamt = max(e->minval[selected], min(e->maxval[selected], config->osc[selected].pwamt + d));
          synth->loadOscConfig(&config->osc[selected], selected);
          sprintf(tempstr, "Osc %d PW amount", selected + 1);
          textatrow(1, tempstr, LCD_BLACK, LCD_WHITE);
          sprintf(tempstr, "%5d %s", config->osc[selected].pwamt, config->osc[selected].pwm ? "(mod)" : "");
          textatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
#if DEBUG_SERIAL
          Serial.print("Oscillator ");Serial.print(selected);Serial.print(": PW ");Serial.println(config->osc[selected].pwamt);
#endif          
          break;
        }
        case SELECT_OS: // PW
        {
          config->osc[0].pwamt = max(e->minval[0], min(e->maxval[0], config->osc[0].pwamt + d));
          config->osc[1].pwamt = config->osc[0].pwamt;
          textatrow(1, "Osc PW amount", LCD_BLACK, LCD_WHITE);
          sprintf(tempstr, "%5d", config->osc[0].pwamt);
          textatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
          synth->loadOscConfig(&config->osc[0], 0);
          synth->loadOscConfig(&config->osc[1], 1);
#if DEBUG_SERIAL
          Serial.print("Oscillator ");Serial.print(0);Serial.print(": PW ");Serial.println(config->osc[0].pwamt);
          Serial.print("Oscillator ");Serial.print(1);Serial.print(": PW ");Serial.println(config->osc[1].pwamt);
#endif          
          break; 
        }
        case SELECT_ENV: // Attack
        {
          config->env.attack = max(e->minval[selected], min(e->maxval[selected], config->env.attack + d));
          synth->loadEnvConfig(&config->env);
          textatrow(1, "Attack", LCD_BLACK, LCD_WHITE);
          sprintf(tempstr, "%5d", config->env.attack);
          textatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
#if DEBUG_SERIAL
          Serial.print("Attack ");Serial.println(config->env.attack);
#endif          
          break;
        }
        case SELECT_MIX: // Sub 1 level
        {
          config->mix.sublevel[0] = max(e->minval[selected], min(e->maxval[selected], config->mix.sublevel[0] + d));
          synth->loadMixerConfig(&config->mix);
          textatrow(1, "Sub 1 level", LCD_BLACK, LCD_WHITE);
          sprintf(tempstr, "%5d %s", config->mix.sublevel[0], config->mix.submute[0] ? "(muted)" :"");
          textatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
#if DEBUG_SERIAL
          Serial.print("Sub 1 level ");Serial.println(config->mix.sublevel[0]);
#endif          
          break;
        }
        case SELECT_F: // Resonance
        {
          config->filter.resonance = max(e->minval[selected], min(e->maxval[selected], config->filter.resonance + d));
          synth->loadFilterConfig(&config->filter);
          textatrow(1, "Resonance", LCD_BLACK, LCD_WHITE);
          sprintf(tempstr, "%5d", config->filter.resonance);
          textatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
#if DEBUG_SERIAL
          Serial.print("Resonance ");Serial.println(config->filter.resonance);
#endif          
          break;
        }
      }
      break;
    case 3: // Mod amt, Decay, Osc 2 level, Env amt
      switch (selected)
      {
        case SELECT_O1:
        case SELECT_O2: // Mod amt
        {
          config->osc[selected].modamt = max(e->minval[selected], min(e->maxval[selected], config->osc[selected].modamt  + d));
          synth->loadOscConfig(&config->osc[selected], selected);
          sprintf(tempstr, "Osc %d mod amount", selected + 1);
          textatrow(1, tempstr, LCD_BLACK, LCD_WHITE);
          sprintf(tempstr, "%5d", config->osc[selected].modamt);
          textatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
#if DEBUG_SERIAL
          Serial.print("Oscillator ");Serial.print(selected);Serial.print(": Mod amount ");Serial.println(config->osc[selected].modamt);
#endif          
          break;
        }
        case SELECT_OS: // Mod amt
        {
          config->osc[0].modamt = max(e->minval[0], min(e->maxval[0], config->osc[0].modamt  + d));
          config->osc[1].modamt = config->osc[0].modamt;
          textatrow(1, "Osc mod amount", LCD_BLACK, LCD_WHITE);
          sprintf(tempstr, "%5d", config->osc[0].modamt);
          textatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
          synth->loadOscConfig(&config->osc[0], 0);
          synth->loadOscConfig(&config->osc[1], 1);
#if DEBUG_SERIAL
          Serial.print("Oscillator ");Serial.print(0);Serial.print(": Mod amount ");Serial.println(config->osc[0].modamt);
          Serial.print("Oscillator ");Serial.print(1);Serial.print(": Mod amount ");Serial.println(config->osc[1].modamt);
#endif          
          break;
        }
        case SELECT_ENV: // Decay
        {
          config->env.decay = max(e->minval[selected], min(e->maxval[selected], config->env.decay + d));
          synth->loadEnvConfig(&config->env);
          textatrow(1, "Decay", LCD_BLACK, LCD_WHITE);
          sprintf(tempstr, "%5d", config->env.decay);
          textatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
#if DEBUG_SERIAL
          Serial.print("Decay ");Serial.println(config->env.decay);
#endif          
          break;
        }
        case SELECT_MIX: // Osc 2 level
        {
          config->mix.osclevel[1] = max(e->minval[selected], min(e->maxval[selected], config->mix.osclevel[1] + d));
          synth->loadMixerConfig(&config->mix);
          textatrow(1, "Osc 2 level", LCD_BLACK, LCD_WHITE);
          sprintf(tempstr, "%5d %s", config->mix.osclevel[1], config->mix.oscmute[1] ? "(muted)" :"");
          textatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
#if DEBUG_SERIAL
          Serial.print("Osc 2 level ");Serial.println(config->mix.osclevel[1]);
#endif          
          break;
        }
        case SELECT_F: // Env amt
        {
          config->filter.envamt = max(e->minval[selected], min(e->maxval[selected], config->filter.envamt + d));
          synth->loadFilterConfig(&config->filter);
          textatrow(1, "Env amount", LCD_BLACK, LCD_WHITE);
          sprintf(tempstr, "%5d", config->filter.envamt);
          textatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
#if DEBUG_SERIAL
          Serial.print("Env amount ");Serial.println(config->filter.envamt);
#endif          
          break;
        }
      }
      break;
    case 4: // Sub-freq, Rate, Sub 2 level, Mod amt
      switch (selected)
      {
        case SELECT_O1:
        case SELECT_O2: // Sub-freq
          handleSubFreq(selected, e, d, false, false);
          break;
        case SELECT_OS:
          handleSubFreq(0, e, d, true, false);
          handleSubFreq(1, e, d, false, true);
          break;
        case SELECT_ENV: // Rate
        {
          config->lfo.rate = max(e->minval[selected], min(e->maxval[selected], config->lfo.rate + d));
          synth->loadModConfig(&config->lfo);
          textatrow(1, "Mod rate", LCD_BLACK, LCD_WHITE);
          sprintf(tempstr, "%5d (%.2fHz)", config->lfo.rate, synth->getModRate());
          textatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
#if DEBUG_SERIAL
          Serial.print("Rate ");Serial.print(config->lfo.rate);Serial.print(" = ");Serial.print(synth->getModRate());Serial.println("Hz");
#endif          
          break;
        }
        case SELECT_MIX: // Sub 2 level
        {
          config->mix.sublevel[1] = max(e->minval[selected], min(e->maxval[selected], config->mix.sublevel[1] + d));
          synth->loadMixerConfig(&config->mix);
          textatrow(1, "Sub 2 level", LCD_BLACK, LCD_WHITE);
          sprintf(tempstr, "%5d %s", config->mix.sublevel[1], config->mix.submute[1] ? "(muted)" :"");
          textatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
#if DEBUG_SERIAL
          Serial.print("Sub 2 level ");Serial.println(config->mix.sublevel[1]);
#endif          
          break;
        }
        case SELECT_F: // Mod amt
        {
          config->filter.modamt = max(e->minval[selected], min(e->maxval[selected], config->filter.modamt + d));
          synth->loadFilterConfig(&config->filter);
          textatrow(1, "Mod amount", LCD_BLACK, LCD_WHITE);
          sprintf(tempstr, "%5d", config->filter.modamt);
          textatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
#if DEBUG_SERIAL
          Serial.print("Mod amount ");Serial.println(config->filter.modamt);
#endif          
          break;
        }
      break;
    }
  }
}

void displayDrumInfo()
{
  sprintf(tempstr, "Beat: %s, Off: %s", beatstateStr(config->drum.beatstate), beatstateStr(config->drum.offbeatstate));
  textatrow(1, tempstr, LCD_BLACK, LCD_WHITE);
  textatrow(2, getKitString(true), LCD_BLACK, LCD_WHITE);   
}

void handleEncoderPress(int i)
{
#if DEBUG_SERIAL
  Serial.print("Encoder ");Serial.print(i);Serial.print(" pressed with selected ");Serial.println(selected);
#endif
  switch (i)
  {
#if PROTOTYPE
    case 1:
    case 2:
    case 3:
    case 4:
      Serial.println(i);
      handleEncoderShiftTurn(i, false);
      break;
#else      
    case SELECT_ENC:
      shift = !shift;
      if (shift)
        shifttimer.begin(shiftflashcallback, 200 * 1000);
      else
        shifttimer.end();
      selectledstate = true;
      setSelectLed(selected);
      break;
#endif
    case METER_ENC:
      metershift = !metershift;
      showRhythm(activemeter, &config->rhythm, metershift);
      break;
    case PRESET_ENC:
      if (!enablePresetButton)
        break;
      if (preset == SET_MANUAL)
      {
        for (int k = 0; k < NUM_POTS; ++k)
        {
          pots[k].adjust = NO_ADJUST;
          handlePot(k, pots[k].value >> 3);
        }
        textatrow(1, "Set to manual", LCD_BLACK, LCD_WHITE);
        textatrow(2, "Done", LCD_BLACK, LCD_WHITE);
      }
      else if (preset == SET_DEFAULT)
      {
        loadPreset(DEFAULT_CONFIG);
      }
      else if (preset == MAX_PRESETS)
      {
        if (saveNewPresetSelected)
        {
          saveNewPresetSelected = false;
          int newpreset = nextFreePreset();
          if (newpreset == -1)
          {
            textatrow(1, "Unable to save", LCD_BLACK, LCD_WHITE);
            textatrow(2, "", LCD_BLACK, LCD_WHITE);
          }
          else
          {
            savePresetSD(newpreset, config);
            textatrow(1, "Saved values", LCD_BLACK, LCD_WHITE);
            sprintf(tempstr, "to preset %02d", newpreset - ROM_PRESETS);
            textatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
          }
        }
      }
      else if (presetMenuMode)
      {
        bool isrom = preset < ROM_PRESETS;
        showPresetLabel(preset);
        if (presetMenuSelect == 1)
        {
          if (loadPreset(preset))
          {
            textatrow(2, "Loaded", LCD_BLACK, LCD_WHITE);
            presetMenuMode = false;
          }
          else
            textatrow(2, "Unable to load", LCD_BLACK, LCD_WHITE);
        }
        else if (presetMenuSelect == 2)
        {
          if (!isrom && savePresetSD(preset, config))
          {
            textatrow(2, "Saved", LCD_BLACK, LCD_WHITE);
            presetMenuMode = false;
          }
          else
            textatrow(2, "Unable to save", LCD_BLACK, LCD_WHITE);
        }
        else if (!isrom && presetMenuSelect == 3)
        {
          if (deletePresetSD(preset))
          {
            textatrow(2, "Deleted", LCD_BLACK, LCD_WHITE);
            presetMenuMode = false;
          }
          else
            textatrow(2, "Unable to delete", LCD_BLACK, LCD_WHITE);
        }
        else
        {
          textatrow(2, "", LCD_BLACK, LCD_WHITE);
          presetMenuMode = false;
        }
      }
      else
      {
        showPresetLabel(preset);
        presetMenuMode = true;
        presetMenuSelect = 1;
        textatrow(2, "Load?", LCD_BLACK, LCD_WHITE);
      }
      break;
  }
}

// this is up or down 1 step only
void handleEncoderShiftTurn(int i, bool back)
{
#if DEBUG_SERIAL
  Serial.print("Encoder ");Serial.print(i);Serial.print(" pressed with selected ");Serial.println(selected);
#endif
  switch (i)
  {
    case 1: // Shape, Trans, O1 mute, none
      switch (selected)
      {
        case SELECT_O1:
        case SELECT_O2: // Shape
          config->osc[selected].shape = getNextWaveshape(true, false, false, selected, back);
          sprintf(tempstr, "Osc %d shape", selected + 1);
          textatrow(1, tempstr, LCD_BLACK, LCD_WHITE);
          textatrow(2, waveshapeStr(config->osc[selected].shape, true), LCD_BLACK, LCD_WHITE);
#if DEBUG_SERIAL
          Serial.print("Oscillator ");Serial.print(selected);Serial.print(": ");Serial.println(waveshapeStr(config->osc[selected].shape, true));
#endif          
          synth->loadOscConfig(&config->osc[selected], selected);
          break;
        case SELECT_OS: // Shape
          config->osc[0].shape = getNextWaveshape(true, false, false, 0, back);
          config->osc[1].shape = config->osc[0].shape;
          textatrow(1, "Osc shape", LCD_BLACK, LCD_WHITE);
          textatrow(2, waveshapeStr(config->osc[0].shape, true), LCD_BLACK, LCD_WHITE);
#if DEBUG_SERIAL
          Serial.print("Oscillator 0: ");Serial.println(waveshapeStr(config->osc[0].shape, true));
          Serial.print("Oscillator 1: ");Serial.println(waveshapeStr(config->osc[0].shape, true));
#endif          
          synth->loadOscConfig(&config->osc[0], 0);
          synth->loadOscConfig(&config->osc[1], 1);
          break;
        case SELECT_ENV: // Transpose
#if PROTOTYPE
          textatrow(1, "Transpose mode", LCD_BLACK, LCD_WHITE);
          if (transposeMode)
          { 
            bool switchOff = synth->droning || poly->pause || (transpose == config->tuning.transpose);
            if (switchOff)
            {
              textatrow(2, "Off", LCD_BLACK, LCD_WHITE);
              transposeMode = false;
            }
            else
            {
              synth->globaloffset(transpose, true);
            }
            showTuning(&config->tuning);
          }
          else
          {
            transposeMode = true;
            textatrow(1, "Transpose mode", LCD_BLACK, LCD_WHITE);
            if (synth->droning || poly->pause)
              sprintf(tempstr, "On");
            else
              sprintf(tempstr, "On [%+2d]", transpose);
            textatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
          }
#else        
          transposeTime = millis() + 200;
          transpose += back ? -1 : 1;
          transpose = min(24, max(-24, transpose));
          textatrow(1, "Transpose", LCD_BLACK, LCD_WHITE);
          sprintf(tempstr, "%+2d", transpose);
          textatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
#endif          
          break;
        case SELECT_MIX: // Osc 1 mute
          config->mix.oscmute[0] = !config->mix.oscmute[0];
          synth->loadMixerConfig(&config->mix);
          textatrow(1, "Osc 1 level", LCD_BLACK, LCD_WHITE);
          sprintf(tempstr, "%5d %s", config->mix.osclevel[0], config->mix.oscmute[0] ? "(muted)" :"");
          textatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
#if DEBUG_SERIAL
          Serial.print("Osc 1 mute");Serial.println(config->mix.oscmute[0]);
#endif          
          break;
        case SELECT_F: // Drums beat state
          managebeatstate(back);
          displayDrumInfo();
          break;
      }
      break;
    case 2: // PWMod, Effect, S1 mute, none
      switch (selected)
      {
        case SELECT_O1:
        case SELECT_O2: // PW mod
          config->osc[selected].pwm = !config->osc[selected].pwm;
          sprintf(tempstr, "Osc %d PW amount", selected + 1);
          textatrow(1, tempstr, LCD_BLACK, LCD_WHITE);
          sprintf(tempstr, "%5d %s", config->osc[selected].pwamt, config->osc[selected].pwm ? "(mod)" : "");
          textatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
          synth->loadOscConfig(&config->osc[selected], selected);
#if DEBUG_SERIAL
          Serial.print("Oscillator ");Serial.print(selected);Serial.print(": PWM ");Serial.println(config->osc[selected].pwm);
#endif          
          break;
        case SELECT_OS: // PW mod
          config->osc[0].pwm = !config->osc[0].pwm;
          config->osc[1].pwm = config->osc[0].pwm;
          textatrow(1, "Osc PW mod", LCD_BLACK, LCD_WHITE);
          sprintf(tempstr, "%s", config->osc[0].pwm ? "On" : "Off");
          textatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
          synth->loadOscConfig(&config->osc[0], 0);
          synth->loadOscConfig(&config->osc[1], 1);
#if DEBUG_SERIAL
          Serial.print("Oscillator 0: PWM ");Serial.println(config->osc[0].pwm);
          Serial.print("Oscillator 1: PWM ");Serial.println(config->osc[1].pwm);
#endif          
          break;
        case SELECT_ENV: // Effect
        {
          effectsCfg *cfg = &config->eff;
          cfg->active = (effect)((cfg->active + 5 + (back ? -1 : 1)) % 5);
          textatrow(1, "Active effect", LCD_BLACK, LCD_WHITE);
          switch (cfg->active)
          {
            case NONE:
              textatrow(2, "None", LCD_BLACK, LCD_WHITE);
              break;
            case CHORUS:
              textatrow(2, "Chorus", LCD_BLACK, LCD_WHITE);
              break;
            case FLANGE:
              textatrow(2, "Flanger", LCD_BLACK, LCD_WHITE);
              break;
            case DELAY:
              textatrow(2, "Delay", LCD_BLACK, LCD_WHITE);
              break;
            case CHORUS_DELAY:
              textatrow(2, "Chorus + Delay", LCD_BLACK, LCD_WHITE);
              break;
          }
          synth->loadEffectsConfig(cfg);
          break;
        }
        case SELECT_MIX: // Sub 1 mute
          config->mix.submute[0] = !config->mix.submute[0];
          synth->loadMixerConfig(&config->mix);
          textatrow(1, "Sub 1 level", LCD_BLACK, LCD_WHITE);
          sprintf(tempstr, "%5d %s", config->mix.sublevel[0], config->mix.submute[0] ? "(muted)" :"");
          textatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
#if DEBUG_SERIAL
          Serial.print("Sub 1 mute ");Serial.println(config->mix.submute[0]);
#endif          
          break;
        case SELECT_F: // Drum Kit
          managedrumclick(back);
          displayDrumInfo();
          break;
      }
      break;
    case 3: // Detune, Effect adj, Osc 2 mute, none
      switch (selected)
      {
        case SELECT_O1:
        case SELECT_O2: // detune
#if PROTOTYPE        
          config->osc[selected].detune += 5;
          if (config->osc[selected].detune > 30)
            config->osc[selected].detune = 0;
#else
          config->osc[selected].detune = max(0, min(32, config->osc[selected].detune + (back ? -1 : 1)));
#endif
          textatrow(1, "Osc detune", LCD_BLACK, LCD_WHITE);
          sprintf(tempstr, "%5d", config->osc[selected].detune);
          textatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
          synth->loadOscConfig(&config->osc[selected], selected);
#if DEBUG_SERIAL
          Serial.print("Oscillator ");Serial.print(selected);Serial.print(": Detune ");
          Serial.println(config->osc[selected].detune);
#endif          
          break;
        case SELECT_OS: 
#if PROTOTYPE        
          config->osc[0].detune += 5;
          if (config->osc[0].detune > 30)
            config->osc[0].detune = 0;
#else
          config->osc[0].detune = max(0, min(32, config->osc[0].detune + (back ? -1 : 1)));
#endif
          config->osc[1].detune = config->osc[0].detune;
          textatrow(1, "Osc detune", LCD_BLACK, LCD_WHITE);
          sprintf(tempstr, "%5d", config->osc[0].detune);
          textatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
          synth->loadOscConfig(&config->osc[0], 0);
          synth->loadOscConfig(&config->osc[1], 1);
#if DEBUG_SERIAL
          Serial.print("Oscillator 0: Detune ");Serial.println(config->osc[0].detune);
          Serial.print("Oscillator 1: Detune ");Serial.println(config->osc[1].detune);
#endif          
          break;
        case SELECT_ENV: // Effect adj
        {          
          effectsCfg *cfg = &config->eff;
          if (cfg->active == NONE)
          {
            textatrow(1, "No effect", LCD_BLACK, LCD_WHITE);
            textatrow(2, "", LCD_BLACK, LCD_WHITE);
          }
          else
          {
            sprintf(tempstr, "%s %s", effectStr(cfg->active), cfg->active == CHORUS_DELAY ? "" : "setting");
            textatrow(1, tempstr, LCD_BLACK, LCD_WHITE);
            switch (cfg->active)
            {
              case CHORUS:
                cfg->chorus = (effect_preset)((cfg->chorus + 4 + (back ? -1 : 1)) % 4);
                textatrow(2, effectPresetStr(cfg->chorus), LCD_BLACK, LCD_WHITE);
                break;
              case FLANGE:
                cfg->flange = (effect_preset)((cfg->flange + 4 + (back ? -1 : 1)) % 4);
                textatrow(2, effectPresetStr(cfg->flange), LCD_BLACK, LCD_WHITE);
                break;
              case DELAY:
                cfg->delay = (effect_preset)((cfg->delay + 4 + (back ? -1 : 1)) % 4);
                textatrow(2, effectPresetStr(cfg->delay), LCD_BLACK, LCD_WHITE);
                break;
              case CHORUS_DELAY:
                cfg->chorus = (effect_preset)((cfg->chorus + 4 + (back ? -1 : 1)) % 4);
                cfg->delay = cfg->chorus;
                textatrow(2, effectPresetStr(cfg->chorus), LCD_BLACK, LCD_WHITE);
                break;
              default:
                break;
            }
            synth->loadEffectsConfig(cfg);
          }
          break;
        }
        case SELECT_MIX: // Osc 2 mute
          config->mix.oscmute[1] = !config->mix.oscmute[1];
          synth->loadMixerConfig(&config->mix);
          textatrow(1, "Osc 2 level", LCD_BLACK, LCD_WHITE);
          sprintf(tempstr, "%5d %s", config->mix.osclevel[1], config->mix.oscmute[1] ? "(muted)" :"");
          textatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
#if DEBUG_SERIAL
          Serial.print("Osc 2 mute ");Serial.println(config->mix.oscmute[1]);
#endif          
          break;
        case SELECT_F:
          manageoffbeatstate(back);
          displayDrumInfo();
          break;
      }
      break;
    case 4: // Sub-shape, Rate shape, Sub 2 mute, none
      switch (selected)
      {
        case SELECT_O1:
        case SELECT_O2: // Subshape
          config->osc[selected].subshape = getNextWaveshape(false, true, false, selected, back);
          sprintf(tempstr, "Sub %d shape", selected + 1);
          textatrow(1, tempstr, LCD_BLACK, LCD_WHITE);
          textatrow(2, waveshapeStr(config->osc[selected].subshape, true), LCD_BLACK, LCD_WHITE);
#if DEBUG_SERIAL
          Serial.print("Oscillator ");Serial.print(selected);Serial.print(": subshape ");Serial.println(waveshapeStr(config->osc[selected].subshape, true));
#endif          
          synth->loadOscConfig(&config->osc[selected], selected);
          break;
        case SELECT_OS: // Subshape
          config->osc[0].subshape = getNextWaveshape(false, true, false, 0, back);
          config->osc[1].subshape = config->osc[0].subshape;
          textatrow(1, "Sub shape", LCD_BLACK, LCD_WHITE);
          textatrow(2, waveshapeStr(config->osc[0].subshape, true), LCD_BLACK, LCD_WHITE);
#if DEBUG_SERIAL
          Serial.print("Oscillator 0: subshape ");Serial.println(waveshapeStr(config->osc[0].subshape, true));
          Serial.print("Oscillator 1: subshape ");Serial.println(waveshapeStr(config->osc[1].subshape, true));
#endif          
          synth->loadOscConfig(&config->osc[0], 0);
          synth->loadOscConfig(&config->osc[1], 1);
          break;
        case SELECT_ENV: // Rate shape
          config->lfo.shape = getNextWaveshape(false, false, true, selected, back);
          textatrow(1, "Mod shape", LCD_BLACK, LCD_WHITE);
          textatrow(2, waveshapeStr(config->lfo.shape, true), LCD_BLACK, LCD_WHITE);
#if DEBUG_SERIAL
          Serial.print("Mod shape ");Serial.println(waveshapeStr(config->lfo.shape, true));
#endif          
          synth->loadModConfig(&config->lfo);
          break;
        case SELECT_MIX: // Sub 2 mute
          config->mix.submute[1] = !config->mix.submute[1];
          synth->loadMixerConfig(&config->mix);
          textatrow(1, "Sub 2 level", LCD_BLACK, LCD_WHITE);
          sprintf(tempstr, "%5d %s", config->mix.sublevel[1], config->mix.submute[1] ? "(muted)" :"");
          textatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
#if DEBUG_SERIAL
          Serial.print("Sub 2 mute ");Serial.println(config->mix.submute[1]);
#endif          
          break;
        case SELECT_F:
          manageoffbeatclick(back);
          displayDrumInfo();
          break;
      }
    break;
  }
}

// special check for startup
void checkForDemoMode()
{
  pushswitch *runsw = &switches[RUN];
  Adafruit_MCP23017 *expander = expanders[runsw->expander];
  demoMode = expander->digitalRead(runsw->pin) == LOW;
}

// special check for startup
bool checkForBackupPrevent()
{
  pushswitch *pausesw = &switches[STEP];
  Adafruit_MCP23017 *expander = expanders[pausesw->expander];
  return expander->digitalRead(pausesw->pin) == LOW;
}

// special check for startup
void checkMIDIClock()
{
  pushswitch *pausesw = &switches[STOP];
  Adafruit_MCP23017 *expander = expanders[pausesw->expander];
  useMIDIClock = expander->digitalRead(pausesw->pin) == LOW;
  if (useMIDIClock)
  {
    textatrow(1, "Using MIDI clock", LCD_BLACK, LCD_WHITE);
    textatrow(2, "", LCD_BLACK, LCD_WHITE);
  }
}

void buttonPressed(int j, int e)
{
#if DEBUG_SERIAL
  Serial.print("Button ");Serial.println(j);
#endif      
  handleButton(j);
  // Serial.print(j);Serial.print(" ");Serial.print(sw->value);Serial.print(", ");
  Adafruit_MCP23017 *expander = expanders[e];
  if (j >= NUM_SEQ_SWITCHES)
  {
#if PROTOTYPE    
    if (transposeMode)
    {
      transposeMode = false;
      textatrow(1, "Transpose mode", LCD_BLACK, LCD_WHITE);
      textatrow(2, "Off", LCD_BLACK, LCD_WHITE);
    }
#endif    
    if (demoMode)
    {
      textatrow(1, "Demo mode off", LCD_BLACK, LCD_WHITE);
      textatrow(2, "", LCD_BLACK, LCD_WHITE);
      demoMode = false;
    }
    presetMenuMode = false;
    expander->digitalWrite(STEP_LED, j == STEP ? HIGH : LOW);
    expander->digitalWrite(RUN_LED, j == RUN ? HIGH : LOW);
    expander->digitalWrite(HOLD_LED, j == HOLD ? HIGH : LOW);
    switch (j)
    {
      case STEP:
        switchOn();
        if (!poly->running)
        {
          polySetup();
          poly->pause = seq1->pause = seq2->pause = true;
        }
        else if (poly->pause)
        {
          seq1->doonestep();
          seq2->doonestep();
          poly->doonestep();
        }
        else
        {
          poly->pause = seq1->pause = seq2->pause = true;
        }
        managePlayState();
        showPauseInfo();
        break;
      case HOLD:
        switchOn();
        synth->setHoldMode();
        poly->stop();
        showDroningFrequencies();
        synth->drone(true);
        managePlayState();
        showTuning(&config->tuning);
        break;
      case STOP:
#if DEBUG_SERIAL
        Serial.println("Stopping");
#endif            
        switchOff();
        synth->stop();
        poly->stop();
        managePlayState();
        resetMIDI();
        showTuning(&config->tuning);
        break;
      case RUN:
        switchOn();
        if (poly->running)
        {
          if (poly->pause)
            poly->setoneshotfn(polyResume);
          else
            poly->setoneshotfn(polyRestart);
        }
        else
        {
          poly->stop();
          synth->stop();
          polySetup();
          managePlayState();
        }
        showSelectSummary(selected);
        break;
    }
  }
//      Serial.println();
}

void checkButtons(bool checkAll, unsigned long now)
{
  for (int j = checkAll ? 0 : NUM_SEQ_SWITCHES; j < NUM_SWITCHES; ++j)
  {
    pushswitch *sw = &switches[j];
    sw->debouncer->update();
    if (sw->debouncer->fell())
    {
      buttonPressed(j, sw->expander);
    }
  }
}

void handleButton(int i)
{
  switch (i)
  {
    case 0: // seq 1 beat 1
    case 1: // seq 1 beat 2
    case 2: // seq 1 beat 3
    case 5: // seq 2 beat 1
    case 6: // seq 2 beat 2
    case 7: // seq 2 beat 3
      config->seq[i > 2 ? 1 : 0].advBeat[i % 5] = !config->seq[i > 2 ? 1 : 0].advBeat[i % 5];
      break;
    case 3: // seq 1 offbeat
    case 8: // seq 2 offbeat
      config->seq[i == 3 ? 0 : 1].advOffbeat = !config->seq[i == 3 ? 0 : 1].advOffbeat;
      break;
      break;
    case 4: // seq 1 apply
    case 9: // seq 2 apply
    {
#if PROTOTYPE      
      encoder *selencoder = &encoders[1]; // for the prototype use the first clickable encoder
#else
      encoder *selencoder = &encoders[0];
#endif      
      Adafruit_MCP23017 *expander = expanders[selencoder->expander];
      if (expander->digitalRead(selencoder->pinSW) == LOW)
      {
        (i == 4 ? seq1 : seq2)->togglePermute();
        showSequencerLabels(seq1->isPermuting(), seq2->isPermuting());
        return;
      }
      else
      {
        int osc = config->seq[i == 4 ? 0 : 1].applyOsc ? 1 : 0;
        int sub = config->seq[i == 4 ? 0 : 1].applySub ? 2 : 0;
        int val = (sub + osc + 1) % 4;
        config->seq[i == 4 ? 0 : 1].applyOsc = val & 0x01;
        config->seq[i == 4 ? 0 : 1].applySub = val & 0x02; 
        break;
      }
    }
  }
  if (i <= 9)
  {
    seq1->update(false);
    seq2->update(false);
    showSequencerState();
    if (i == 4 || i == 9)
    {
      managePlayState();
      synth->updateDrone();
    }
  }
}

// setup for MUXes
void setupPots() 
{
  // 4 pin select outputs
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);

  // disable until reading required
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW); // active low enable

  // analog read is on A8
  pinMode(A8, INPUT);

#if USE_ADC_LIB  
  adc->adc0->setAveraging(16); // set number of averages
  adc->adc0->setResolution(14); // set bits of resolution
  // where the numbers are the frequency of the ADC clock in MHz and are independent on the bus speed.
  adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::MED_SPEED); // change the conversion speed
  // it can be any of the ADC_MED_SPEED enum: VERY_LOW_SPEED, LOW_SPEED, MED_SPEED, HIGH_SPEED or VERY_HIGH_SPEED
  adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::MED_SPEED); // change the sampling speed
#endif    
}

int scalepot(int actual, int saved, int delta, int maxval, int minval, int threshold)
{
  if (delta == 0)
    return saved;
    
  int diff = actual - saved;
  if (abs(diff) < threshold)
    return actual;
  else if (actual - minval < threshold)
    return actual;  // use actual around the limits
  else if (maxval - actual < threshold)
    return actual;  // use actual around the limits

  float scale;
  if (delta > 0)
  {
    scale = 1.0 * (maxval - saved) / (maxval - actual);
  }
  else
  {
    scale = 1.0 * (saved - minval) / (actual - minval);
  }
  return max(minval, min(maxval, round(saved + delta * scale)));
}

void checkPots(int potindex, bool setOnly, bool avgOnly, unsigned long now)
{      
  int i = potindex;
  pot *p = &pots[i];
  if (setOnly)
  {
#if PROTOTYPE
    digitalWrite(6, p->pin & 0x01);
    digitalWrite(5, p->pin & 0x02);
    digitalWrite(4, p->pin & 0x04);
    digitalWrite(3, p->pin & 0x08);
#else        
    digitalWrite(6, i & 0x08);
    digitalWrite(5, i & 0x04);
    digitalWrite(4, i & 0x02);
    digitalWrite(3, i & 0x01);
#endif    
    return;
  }
//  digitalWrite(9, LOW);

  int delta;
#if USE_ADC_LIB
  int raw = adc->adc0->analogRead(A8);
  int v = movingaverage(MAXRAWVAL - raw, p->avgrawvalue, 3);
  p->avgrawvalue = v;
  v = (MAXRAWVAL - raw) >> 2;
  raw >>= 2;
#else
  int raw = analogRead(A8);
  int v = movingaverage(MAXRAWVAL - raw, p->avgrawvalue, 3);
  p->avgrawvalue = v;
#endif  
  
  if (avgOnly)
    return;
  if ((now - p->lastadjustmillis < 200) && ((p->value - raw > 0) == p->lastadjustup))  // not jitter if recent and same direction
    delta = 5;
  else
    delta = 15;
  if (p->signedrange)
  {
    float dv = abs(v - CENTRE);
    if (dv < ZERO_RANGE)
      v = MAXPOTVAL / 2;
    else
      v = MAXPOTVAL / 2 + (v > CENTRE ? 1 : -1) * SCALE * (dv - ZERO_RANGE);
  }
  if (abs(p->value - v) >= delta || (v >= MAXPOTVAL && p->value != MAXPOTVAL) || (v == 0 && p->value != 0))
  {
    p->lastadjustmillis = now;
    p->lastadjustup = p->value - v > 0;
    if (i == TEMPO_POT)
    {
      int adjv;
      if (p->adjust != NO_ADJUST)
      {
        adjv = scalepot(v, p->loadedvalue << 3, v - p->value, MAXPOTVAL, 0, 10);
        p->loadedvalue = adjv >> 3; // update for next check
      }
      else
        adjv = v;
      handlePot(i, adjv >> 3);
    }
    else
    {
      bool cansend = p->adjust == NO_ADJUST;
      cansend = cansend || (p->adjust == ABOVE && ((p->value >> 3) <= p->loadedvalue + 2));
      cansend = cansend || (p->adjust == BELOW && ((p->value >> 3) >= p->loadedvalue - 2));
      if (cansend)
      {
        handlePot(i, v >> 3);
        p->adjust = NO_ADJUST;
      }
    }
    p->value = v;
  }
//  digitalWrite(9, HIGH); // active low enable
}

void handlePot(int i, int value)
{
  switch (i)
  {
    case TEMPO_POT:
    {
      config->rhythm.tempo = value;
      poly->setTempo(value);
      showTempo(useMIDIClock ? -1 : (int)round(60000.0 / poly->tempoToDelay(value)));
#if DEBUG_SERIAL
      Serial.print("Tempo: ");Serial.print(value);Serial.print(" -> ");Serial.print((int)round(60000.0 / poly->tempoToDelay(value)));Serial.println("bpm");
#endif      
      break;
    }
    case VOLUME_POT:
#if PROTOTYPE    
      synth->setLevel(127 - value);
#else      
      synth->setLevel(value);
#endif      
      break;
    case DRUM_VOL_POT:
    {
      lastupdatemillis = millis();
      adjustDrumsVolume(value);
      break;
    }
    default:
    {
      int seqnum = i < 4 ? 0 : 1;
      setStepValue(seqnum, i, round((value - 63.5) * 49 / 127.0));
      break;
    }
  }
}

void setStepValue(int seqnum, int i, int value)
{
  sequencerCfg *seqcfg = &config->seq[seqnum];
  value = min(24, max(-24, value));
  i %= 4;
  if (seqcfg->steps[i] != value)
  {
    seqcfg->steps[i] = value;
  }
  if (seqnum == 0)
    seq1->update(false);
  else
    seq2->update(false);
  if (synth->droning)
    synth->updateDrone();      
  else if (poly->running && poly->pause)
  {
    // update the playing notes if necessary 
    synth->updateOscillator(0, &config->osc[0], seq1->getCurStepIndex(), &seq1->seqcfg);
    synth->updateOscillator(1, &config->osc[1], seq2->getCurStepIndex(), &seq2->seqcfg);
    showPauseInfo();
  }
  sprintf(tempstr, "%+3d", seqcfg->steps[i]);
  showSequencer(seqnum, tempstr, i);
}

void setupControls()
{
  setupExpanders();
  setupPots();
  setSelectLed(selected);
  showSequencerState();
//  showSelectSummary(selected);
  showTuning(&config->tuning);
  showTempo((int)round(60000.0 / poly->tempoToDelay(config->rhythm.tempo)));
  Adafruit_MCP23017 *expander = expanders[1];
  expander->digitalWrite(STEP_LED, LOW);
  expander->digitalWrite(RUN_LED, LOW);
  expander->digitalWrite(HOLD_LED, LOW);
  checkForDemoMode();
  if (demoMode)
  {
    textatrow(1, "Demo mode on", LCD_BLACK, LCD_WHITE);
    textatrow(2, "", LCD_BLACK, LCD_WHITE);
    polySetup();
    expanders[1]->digitalWrite(RUN_LED, HIGH);
    preset = -1;

    pushswitch *runsw = &switches[RUN];
    Adafruit_MCP23017 *expander = expanders[runsw->expander];
    while (expander->digitalRead(runsw->pin) == LOW)
      ;

    loadPreset(0);
  }
  checkMIDIClock();
  lastupdatemillis = millis();
}

bool isDemoMode()
{
  return demoMode;
}

void showSequencerState()
{
  for (int i = 0; i < 2; ++i)
  {
    int j = 0;
    sequencerCfg seqcfg = config->seq[i];
    Adafruit_MCP23017 *expander;
    pushswitch *sw;
    do 
    {
      sw = &switches[i * 5 + j];
      expander = expanders[sw->expander];
      expander->digitalWrite(sw->led1pin, seqcfg.advBeat[j] ? HIGH : LOW);
    } while (++j < 3);
    sw = &switches[i * 5 + j];
    expander = expanders[sw->expander];
    expander->digitalWrite(sw->led1pin, seqcfg.advOffbeat ? HIGH : LOW);
    sw = &switches[i * 5 + j + 1];
    expander = expanders[sw->expander];
    expander->digitalWrite(sw->led1pin, seqcfg.applyOsc ? HIGH : LOW);
    expander->digitalWrite(sw->led2pin, seqcfg.applySub ? HIGH : LOW);
  }
}

void updateStepsDisplay()
{
  for (int i = 0; i < 2; ++i)
  { 
    updateOneSeqStepsDisplay(i, config->seq[i].steps);
  }
}

void updateOneSeqStepsDisplay(int i, int *steps)
{
  for (int j = 0; j < 4; ++j)
  {
    sprintf(tempstr, "%+3d", steps[j]);
    showSequencer(i, tempstr, j);
  }
}

void checkControls(int loopcount)
{
  if (toggleshiftled)
  {
    selectledstate = !selectledstate;
    setSelectLed(selected);
    toggleshiftled = false;
  }

  if (loopcount == 200) // ensure an early initial update
    updateStepsDisplay();

  unsigned long curmillis = millis();
  if (transposeTime > 0)
    applyTranspose(curmillis);
  if (demoMode)
  {
    if (loopcount == 10)
    {
      textatrow(1, "Running Demo", LCD_BLACK, LCD_WHITE);
      textatrow(2, "", LCD_BLACK, LCD_WHITE);
    }
    if (loopcount % 150 == 0)
      expanders[1]->digitalWrite(RUN_LED, loopcount % 300 == 0);
    static unsigned long t = millis() - 1000 * DEMO_TIME_PER_PRESET + 1000;
    if (millis() - t > 1000 * DEMO_TIME_PER_PRESET)
    {
      preset = nextPreset(preset);
      if (preset < MAX_PRESETS)
      {
        loadPreset(preset);
        if (preset == 0)
        {
          switchOn();
          polySetup();
          managePlayState();
        }
        t = millis();
      }
      else
      {
        poly->stop();
        synth->stop();
        expanders[1]->digitalWrite(RUN_LED, LOW);
        demoMode = false;
        textatrow(1, "Demo complete", LCD_BLACK, LCD_WHITE);
        textatrow(2, "", LCD_BLACK, LCD_WHITE);
      }
    }
  }

  static int potindex = 0;
  //    select a new pot every POTLOOPSKIP steps
  //    read it every step and update the moving average
  //    only use it every POTLOOPSKIP steps (once the average has stabilised)
  int count = loopcount % POTLOOPSKIP;
  if (count == 0)
  {
    potindex = (potindex + 1) % NUM_POTS;
    checkPots(potindex, true, false, curmillis);
  }
  else if (count == POTLOOPSKIP - 1)
  {
    checkPots(potindex, false, false, curmillis);
  }
  else if (count >= POTLOOPSET)
  {
    checkPots(potindex, false, true, curmillis);
  }
  if (loopcount % 10 == 0)
  {
    checkButtons(loopcount % 20 == 0, curmillis);
  }
  checkEncoders(curmillis);
  if (lastupdatemillis > 0)
  {
    if (lastencoder == METER_ENC)
    {
      lastencoder = -1;
#if DEBUG_SERIAL  
      Serial.println("###  Meter update ###");
      Serial.println(poly->getRhythmString(0));
      Serial.println(poly->getRhythmString(1));
      Serial.println(poly->getRhythmString(2));
#endif      
      showRhythmGraph(poly, activemeter);
    }
    else if (curmillis - lastupdatemillis > UPDATE_WAIT_TIME)
    {
      saveNewPresetSelected = false;
      showSelectSummary(selected);
      lastupdatemillis = -1;
      if (!metershift)
      {
        metershift = true;
        showRhythm(activemeter, &config->rhythm, metershift);
      }
    }
  }
}

void showOscillatorSummary(int selected)
{
  oscillatorCfg cfg = config->osc[selected];
  float f = noteToFreq(cfg.freq);
  sprintf(tempstr, "Osc %d: %dHz %s %s", selected + 1, (int)f, notestring((int)round(cfg.freq)), waveshapeStr(cfg.shape, false));
  textathalfrow(1, false, tempstr, LCD_BLACK, LCD_WHITE, 0, 4);
  sprintf(tempstr, "Sub %d: /%d %dHz %s %s", selected + 1, cfg.subfreq, (int)(f / cfg.subfreq), 
    notestring(freqToNote(f / cfg.subfreq)), waveshapeStr(cfg.subshape, false));
  textathalfrow(1, true, tempstr, LCD_BLACK, LCD_WHITE, -2, 4);
  sprintf(tempstr, "PW: %d%s, Mod: %d, DT: %d", cfg.pwamt, cfg.pwm ? " mod" : "", cfg.modamt, cfg.detune);
  textathalfrow(2, false, tempstr, LCD_BLACK, LCD_WHITE, 6, 4);
}

void showCombinedOscillatorSummary()
{
  oscillatorCfg cfg1 = config->osc[0];
  oscillatorCfg cfg2 = config->osc[1];
  float f1 = noteToFreq(cfg1.freq);
  float f2 = noteToFreq(cfg2.freq);
  sprintf(tempstr, "1. %dHz %s %s, /%d %s", (int)f1, notestring((int)round(cfg1.freq)), waveshapeStr(cfg1.shape, false),
    cfg1.subfreq, waveshapeStr(cfg1.subshape, false));
  textathalfrow(1, false, tempstr, LCD_BLACK, LCD_WHITE, 0, 4);
  sprintf(tempstr, "2. %dHz %s %s, /%d %s", (int)f2, notestring((int)round(cfg2.freq)), waveshapeStr(cfg2.shape, false),
    cfg2.subfreq, waveshapeStr(cfg2.subshape, false));
  textathalfrow(1, true, tempstr, LCD_BLACK, LCD_WHITE, -2, 4);
  sprintf(tempstr, "PW %d%s/%d%s, Mod %d/%d, DT %d/%d", cfg1.pwamt, cfg1.pwm ? "m" : "", 
    cfg2.pwamt, cfg2.pwm ? "m" : "", cfg1.modamt, cfg2.modamt, cfg1.detune, cfg2.detune);
  textathalfrow(2, false, tempstr, LCD_BLACK, LCD_WHITE, 6, 4);
}

void showSelectSummary(int selected)
{
  if (presetMenuMode)
    return;
  textatrow(1, "", LCD_BLACK, LCD_WHITE);
  textatrow(2, "", LCD_BLACK, LCD_WHITE);
  switch (selected)
  {
    case SELECT_O1: // freq, pw, modamt, subfreq & shape, mod, subshape
    case SELECT_O2:
      showOscillatorSummary(selected);
      break;
    case SELECT_OS:
      showCombinedOscillatorSummary();
      break;
    case SELECT_ENV:
    {
      envelopeCfg cfg = config->env;
      modulatorCfg mod = config->lfo;
      effectsCfg eff = config->eff;
      sprintf(tempstr, "%s, Env: A %d, D %d", quantisationStr(config->tuning.scale, false), cfg.attack, cfg.decay);
      textathalfrow(1, false, tempstr, LCD_BLACK, LCD_WHITE, 0, 4);
      sprintf(tempstr, "Mod: rate %d (%.2fHz), %s", mod.rate, synth->getModRate(), waveshapeStr(mod.shape, false));
      textathalfrow(1, true, tempstr, LCD_BLACK, LCD_WHITE, -2, 4);
      switch (eff.active)
      {
        case NONE:
          textathalfrow(2, false, "No effects", LCD_BLACK, LCD_WHITE, 6, 4);
          break;
        case CHORUS:
        case CHORUS_DELAY:
          sprintf(tempstr, "Effect: %s %s", effectStr(eff.active), effectPresetStr(eff.chorus));
          textathalfrow(2, false, tempstr, LCD_BLACK, LCD_WHITE, 6, 4);
          break;
        case FLANGE:
          sprintf(tempstr, "Effect: %s %s", effectStr(eff.active), effectPresetStr(eff.flange));
          textathalfrow(2, false, tempstr, LCD_BLACK, LCD_WHITE, 6, 4);
          break;
        case DELAY:
          sprintf(tempstr, "Effect: %s %s", effectStr(eff.active), effectPresetStr(eff.delay));
          textathalfrow(2, false, tempstr, LCD_BLACK, LCD_WHITE, 6, 4);
          break;
      }
      break;
    }
    case SELECT_MIX:
    {
      mixerCfg cfg = config->mix;
      textathalfrow(1, false, "Levels", LCD_BLACK, LCD_WHITE, 0, 4);
      sprintf(tempstr, "Osc 1: %d%s, Sub 1: %d%s", cfg.osclevel[0], cfg.oscmute[0] ? "*" : "", 
        cfg.sublevel[0], cfg.submute[0] ? "*" : "");
      textathalfrow(1, true, tempstr, LCD_BLACK, LCD_WHITE, -2, 4);
      sprintf(tempstr, "Osc 2: %d%s, Sub 2: %d%s", cfg.osclevel[1], cfg.oscmute[1] ? "*" : "", 
        cfg.sublevel[1], cfg.submute[1] ? "*" : "");
      textathalfrow(2, false, tempstr, LCD_BLACK, LCD_WHITE, 6, 4);
      break;
    }
    case SELECT_F:
    {
      filterCfg cfg = config->filter;
      float co = (int)synth->getCutoffFreq();
      if (co >= 1000)
        sprintf(tempstr, "Filter c/o: %3d (%.2fkHz)", cfg.cutoff, co / 1000);
      else
        sprintf(tempstr, "Filter c/o: %3d (%dHz)", cfg.cutoff, (int)co);
      textathalfrow(1, false, tempstr, LCD_BLACK, LCD_WHITE, 0, 2);
      sprintf(tempstr, "Res: %d, Env: %+d, Mod: %d", cfg.resonance, cfg.envamt, cfg.modamt);
      textathalfrow(1, true, tempstr, LCD_BLACK, LCD_WHITE, -2, 2);

      sprintf(tempstr, "Drums %s B %s O %s", getKitString(true),
        beatstateStr(config->drum.beatstate), beatstateStr(config->drum.offbeatstate));
      textathalfrow(2, false, tempstr, LCD_BLACK, LCD_WHITE, 6, 2);
      break;
    }
  }
}

void showDroningFrequencies()
{
  float f, subf;
  synth->getDroningFrequencies(&f, &subf);
  sprintf(tempstr, "Osc: %dHz (%s)", (int)f, notestring(freqToNote(f)));
  textatrow(1, tempstr, LCD_BLACK, LCD_WHITE);
  sprintf(tempstr, "Sub: %dHz (%s)", (int)subf, notestring(freqToNote(subf)));
  textatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
}

void showPauseInfo()
{
  if (!poly->running)
    return;
  float f1, subf1, f2, subf2;
  synth->getPlayingFrequencies(&f1, &subf1, &f2, &subf2);
  char savenote[10];
  strcpy(savenote, notestring(freqToNote(f1)));
  sprintf(tempstr, "1: %d/%dHz (%s/%s)", (int)f1, (int)subf1, savenote, notestring(freqToNote(subf1)));
  smallertextatrow(1, tempstr, LCD_BLACK, LCD_WHITE);
  strcpy(savenote, notestring(freqToNote(f2)));
  sprintf(tempstr, "2: %d/%dHz (%s/%s)", (int)f2, (int)subf2, savenote, notestring(freqToNote(subf2)));
  smallertextatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
}

void setSelectLed(int led)
{
  expanders[selleds[0].expander]->digitalWrite(selleds[0].pin, led == SELECT_O1 || led == SELECT_OS ? selectledstate : LOW); 
  expanders[selleds[1].expander]->digitalWrite(selleds[1].pin, led == SELECT_O2 || led == SELECT_OS ? selectledstate : LOW); 
  expanders[selleds[2].expander]->digitalWrite(selleds[2].pin, led == SELECT_ENV ? selectledstate : LOW); 
  expanders[selleds[3].expander]->digitalWrite(selleds[3].pin, led == SELECT_MIX ? selectledstate : LOW); 
  expanders[selleds[4].expander]->digitalWrite(selleds[4].pin, led == SELECT_F ? selectledstate : LOW); 
}

void setSequenceLed(int seq, int led, bool onoff)
{
  int i = led + (seq - 1) * 4;
  if (i < 0 || i > 7)
    return;
  sequencerled *seqled = &seqleds[i];
  Adafruit_MCP23017 *expander = expanders[seqled->expander];
  expander->digitalWrite(seqled->pin, onoff ? HIGH : LOW); 
}

void setToCurTempo()
{
  pot *p = &pots[TEMPO_POT];
  digitalWrite(6, p->pin & 0x01);
  digitalWrite(5, p->pin & 0x02);
  digitalWrite(4, p->pin & 0x04);
  digitalWrite(3, p->pin & 0x08);
//  digitalWrite(9, LOW);
  
  int v = 1023 - analogRead(A8);
  handlePot(TEMPO_POT, v >> 3);
  p->adjust = NO_ADJUST;
  p->value = v;
}

void setInitialVolumes()
{
  int ps[] = { VOLUME_POT, DRUM_VOL_POT };
  for (int i = 0; i < 2; ++i)
  {
    pot *p = &pots[ps[i]];    
    digitalWrite(6, p->pin & 0x01);
    digitalWrite(5, p->pin & 0x02);
    digitalWrite(4, p->pin & 0x04);
    digitalWrite(3, p->pin & 0x08);
//    digitalWrite(9, LOW);
  
    int v = 1023 - analogRead(A8);
    handlePot(ps[i], v >> 3);
    p->adjust = NO_ADJUST;
    p->value = v;
  }
}

void polySetup()
{
  poly->pause = seq1->pause = seq2->pause = false;
  seq1->update(false);
  seq2->update(false);
  showSequencerLabels(seq1->isPermuting(), seq2->isPermuting());
  rhythmCfg *r = &config->rhythm;
  if (r->tempo < 0)
    setToCurTempo();
  poly->generate(r);
  poly->run(beatcallback, offbeatcallback, r->tempo);
}

void polyRestart()
{
  poly->stop();
  seq1->reset();
  seq2->reset();
  poly->run(beatcallback, offbeatcallback, config->rhythm.tempo);
  managePlayState();
}

void polyResume()
{
  poly->pause = seq1->pause = seq2->pause = false;
  poly->run(beatcallback, offbeatcallback, config->rhythm.tempo);
  managePlayState();
}

void showPresetLabel(int p)
{
  bool isrom = p < ROM_PRESETS;
  if (isrom)
    sprintf(tempstr, "ROM: %s", getROMPresetName(p));
  else
    sprintf(tempstr, "User: %02d", p - ROM_PRESETS);
  textatrow(1, tempstr, LCD_BLACK, LCD_WHITE);
}

bool loadPreset(int preset)
{
  if (preset == DEFAULT_CONFIG)
  {
    textatrow(1, "Init settings", LCD_BLACK, LCD_WHITE);
    setDefaultConfig();
    setToCurTempo();
  }
  else
  {
    loadPresetSD(preset, config);
    showPresetLabel(preset);
  }
  textatrow(2, "Loaded", LCD_BLACK, LCD_WHITE);
#if DEBUG_SERIAL
  Serial.print("Setting to preset ");
  Serial.println(preset);
#endif      
  synth->loadConfig(config);
  poly->regenerate(&config->rhythm);
  seq1->update(true);
  seq2->update(true);
  showSequencerLabels(false, false);
  managePlayState();

  for (int i = 0; i < 7; ++i)
    manageLoadedValue(&pots[i], config->seq[i < 4 ? 0 : 1].steps[i % 4], true);
  manageLoadedValue(&pots[TEMPO_POT], config->rhythm.tempo, false);    
  
  showSequencerState();
//  showSelectSummary(selected);
  showTuning(&config->tuning);
  showRhythm(0, &config->rhythm, metershift);
  updateStepsDisplay();
  showTempo((int)round(60000.0 / poly->tempoToDelay(config->rhythm.tempo)));
  transposeTime = -1;
#if PROTOTYPE  
  transposeMode = false;
#endif  
  transpose = config->tuning.transpose;
  
  if (preset == DEFAULT_CONFIG)
    pots[TEMPO_POT].adjust = NO_ADJUST;

  return true;
}

#define STEP_TO_VAL(x) round((x + 24) * 127.0 / 48)
void manageLoadedValue(pot *p, int v, bool isstep)
{
  if (isstep)
    v = STEP_TO_VAL(v);
  p->loadedvalue = v;
  int checkval = p->value >> 3;
  if (p->loadedvalue > checkval)
    p->adjust = BELOW;  // do not send while still below
  else if (p->loadedvalue < checkval) 
    p->adjust = ABOVE;  // do not send while still above
  else
    p->adjust = NO_ADJUST;
}

void applyTranspose(unsigned long curmillis)
{
  if (transposeTime > 0 && (long)curmillis > transposeTime)
  {
    config->tuning.transpose = transpose;
    synth->globaloffset(config->tuning.transpose, true);
    transposeTime = -1;
  }
}
