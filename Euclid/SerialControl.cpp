#include <string.h>
#include <ctype.h>

#include "Controls.h"
#include "LCD.h"
#include "Presets.h"
#include "Drums.h"

#define MAXLENGTH 100
#define ARGLENGTH 6
#define MAXARGS 4

void runSerialCommand();
bool saveSerialStream(int n);

typedef char Argument[ARGLENGTH];
typedef struct
{
  bool valid;
  Argument cmd;
  Argument args[MAXARGS];
} command;
  
command getcmd(char *s)
{
  command cmd = {false, "", {"", "", "", ""}};

  int i = 0;
  while (isspace(s[i]))
    i++;
  for (int k = -1; k < MAXARGS; ++k)
  {
    if (!s[i])
      break;
    while (isspace(s[i]))
      i++;
    for (int j = 0; j < ARGLENGTH; ++j)
    {
      char c = s[i];
      if (c && !isspace(c))
      {
        if (k < 0)
          cmd.cmd[j] = c;
        else
          cmd.args[k][j] = c;
        cmd.valid = true;
        i++;
      }
      else
      {
        break; 
      }
    }
  }
  
  return cmd;
}

bool isplus(char *arg)
{
  return strcmp(arg, "+") == 0;
}

bool isminus(char *arg)
{
  return strcmp(arg, "-") == 0;
}

int getargnum(char *arg, bool subone)
{
  if (isplus(arg))
    return 1;
  else if (isminus(arg))
    return -1;
  else
    return subone ? atol(arg) - 1 : atol(arg);
}

static char s_str[MAXLENGTH + 1] = {0};
static int s_index = 0;
bool getSerialInput();

void clearSerialInput()
{
  memset(s_str, 0, MAXLENGTH + 1);
  s_index = 0;
}

void checkSerialControl()
{
  if (getSerialInput())
  {
    runSerialCommand();
    clearSerialInput();
  }
}
      
bool getSerialInput()
{
  char c;
  while (s_index < MAXLENGTH && Serial.available())
  {
    c = Serial.read();
    if (c == 0x7F || c == '\b')
    {
      if (s_index)
      {
        s_str[--s_index] = '\0';
        Serial.print('X');
        Serial.flush();
      }
    }
    else if (c == '\n' || c == '\r')
    {
      Serial.println();
      return true;
    }
    else if (iscntrl(c))
    {
      // discard all the control data
      while (Serial.available())   
        c = Serial.read();
    }
    else //if (!iscntrl(c))
    {
      Serial.print(c);
      Serial.flush();
      s_str[s_index++] = c;
    }
  }
  return false;
}

void runSerialCommand()
{
  command thecmd = getcmd(s_str);
  if (!thecmd.valid)
    return;
  
//  Serial.print("Running '");
//  Serial.print(s_str);
//  Serial.println("'");
//  
//  Serial.println(thecmd.cmd);
//  for (int k = 0; k < MAXARGS; ++k)
//  {
//    if (strlen(thecmd.args[k]))
//    {
//      Serial.print("Arg ");
//      Serial.print(k + 1);
//      Serial.print(": ");
//      Serial.println(thecmd.args[k]);
//    }
//  }

  char *cmd = thecmd.cmd;
  char *a1 = strlen(thecmd.args[0]) ? thecmd.args[0] : NULL;
  char *a2 = strlen(thecmd.args[1]) ? thecmd.args[1] : NULL;
  char *a3 = strlen(thecmd.args[2]) ? thecmd.args[2] : NULL;
  char *a4 = strlen(thecmd.args[3]) ? thecmd.args[3] : NULL;
  lastupdatemillis = millis();

  if ((strcasecmp(cmd, "r") == 0 || strcasecmp(cmd, "run") == 0) && !a1)
  {
    Serial.println("Run");
    buttonPressed(RUN, 1);
  }
  else if (strcasecmp(cmd, "stop") == 0 || strcmp(cmd, ".") == 0)
  {
    Serial.println("Stop");
    buttonPressed(STOP, 1);
  }
  else if (strcasecmp(cmd, "hold") == 0 || strcasecmp(cmd, "h") == 0)
  {
    Serial.println("Hold");
    buttonPressed(HOLD, 1);
  }
  else if (strcasecmp(cmd, "pause") == 0)
  {
    Serial.println("Step");
    buttonPressed(STEP, 1);
  }
  else if (strcasecmp(cmd, "sel") == 0)
  {
    selected = getargnum(a1, true);
    selected = (selected + NUM_SELECT) % NUM_SELECT; 
    setSelectLed(selected);
    showSelectSummary(selected);
  }
  else if (strcasecmp(cmd, "turn") == 0 && a2)
  {
    int enc = atol(a1);
    if (enc > 0 && enc < 4)
    {
      int d = getargnum(a2, false);
      handleEncoderTurn(enc, d);
    }
  }
#if PROTOTYPE
  else if (strcasecmp(cmd, "press") == 0 && a1)
  {
    int enc = atol(a1);
    if (enc > 0 && enc < 4)
      handleEncoderPress(enc);
  }
#else  
  else if (strcasecmp(cmd, "shift") == 0)
  {
    handleEncoderPress(0);
  }
#endif  
  else if (strcasecmp(cmd, "step") == 0 && a3)
  {
    int s = atol(a1);
    int p = atol(a2);
    if ((s == 1 || s == 2) && (p > 0 && p < 5))
    {
      s--;
      p--;
      sequencerCfg *seqcfg = &config->seq[s];
      int value;
      if (isdigit(a3[0]))
        value = getargnum(a3, false);
       else
        value = seqcfg->steps[p] + getargnum(a3, false);
      setStepValue(s, p, value);
    }
  }
  else if (strcasecmp(cmd, "beat") == 0 && a3)
  {
    int s = atol(a1);
    int b = atol(a2);
    int o = atol(a3);
    if ((s == 1 || s == 2) && (b >= 0 && b <= 7) && (o == 0 || o == 1))
    {
      s--;
      sequencerCfg *seqcfg = &config->seq[s];
      seqcfg->advBeat[0] = b & 0x04;
      seqcfg->advBeat[1] = b & 0x02;
      seqcfg->advBeat[2] = b & 0x01;
      seqcfg->advOffbeat = o;
      showSequencerState();
    }
  }
  else if (strcasecmp(cmd, "app") == 0 && a2)
  {
    int s = atol(a1);
    int os = atol(a2);
    if ((s == 1 || s == 2) && (os >= 0 && os <= 3))
    {
      s--;
      sequencerCfg *seqcfg = &config->seq[s];
      seqcfg->applyOsc = os & 0x02;
      seqcfg->applySub = os & 0x01;
      showSequencerState();
    }
  }
  else if (strcasecmp(cmd, "meter") == 0 && a4)
  {
    int r = atol(a1);
    int i = getargnum(a2, false);
    int j = getargnum(a3, false);
    int k = getargnum(a4, false);
    if ((i > 0 && i <= j) && (j < 32) && (k >= 0 && k < j))
    {
      rhythmCfg *rcfg = &config->rhythm;
      switch (r)
      {
        case 1:
          rcfg->r1[0] = i;
          rcfg->r1[1] = j;
          rcfg->r1[2] = k;
          break;
        case 2:
          rcfg->r2[0] = i;
          rcfg->r2[1] = j;
          rcfg->r2[2] = k;
          break;
        case 3:
          rcfg->r3[0] = i;
          rcfg->r3[1] = j;
          rcfg->r3[2] = k;
          break;
        default:
          return;
      }
      updateActiveMeter(0);
    }
  }
  else if (strcasecmp(cmd, "tempo") == 0 && a1)
  {
    int curtempo = config->rhythm.tempo;
    int t = getargnum(a1, false);
    if (t < 0 || a1[0] == '+')
    {
      curtempo += t;
    }
    else
    {
      curtempo = t;
    }
    config->rhythm.tempo = curtempo;
    poly->setTempo(curtempo);
    showTempo((int)round(60000.0 / poly->tempoToDelay(curtempo)));
  }
  else if (strcasecmp(cmd, "vol") == 0 && a1)
  {      
    int value = getargnum(a1, false);
    synth->setLevel(min(max(value, 0), 127));
  }
  else if (strcasecmp(cmd, "dv") == 0 && a1)
  {      
    int value = getargnum(a1, false);
    adjustDrumsVolume(min(max(value, 0), 127));
  }
  else if (strcasecmp(cmd, "dump") == 0 && a1)
  {
    if (strcmp(a1, "*") == 0)
      dumpAllPresetsSD();
    else
      dumpPresetSD(atol(a1));
  }
  else if (strcasecmp(cmd, "pre") == 0 && a1)
  {
    long np = atol(a1);
    loadPreset(np);
    Serial.print("Setting to preset ");
    Serial.println(np);
  }
  else if (strcasecmp(cmd, "conf") == 0)
  {
    printConfig(NULL);
    Serial.println();
  }
  else if (strcasecmp(cmd, "rest") == 0 && a1)
  {
    if (strcmp(a1, "*") == 0)
    {
      Serial.print("Restoring all from backup");
      restoreFromBackupSD();
      Serial.println("Done");
    }
    else if (a1)
    {
      Serial.print("Recovering preset ");
      Serial.println(atol(a1));
      restorePresetSD(atol(a1));
    }
  }
  else if (strcasecmp(cmd, "mem") == 0)
  {
    Serial.print("Max memory usage: ");
    Serial.println(AudioMemoryUsageMax());
  }
  else if (strcasecmp(cmd, "temp") == 0)
  {
    Serial.print("MCU Temperature: ");
    Serial.println(tempmonGetTemp());
  }
  else if (strcasecmp(cmd, "mute") == 0 && a1)
  {
    if (strcasecmp(a1, "o1") == 0)
      config->mix.oscmute[0] = !config->mix.oscmute[0];
    else if (strcasecmp(a1, "s1") == 0)
      config->mix.submute[0] = !config->mix.submute[0];
    if (strcasecmp(a1, "o2") == 0)
      config->mix.oscmute[1] = !config->mix.oscmute[1];
    else if (strcasecmp(a1, "s2") == 0)
      config->mix.submute[1] = !config->mix.submute[1];
    else
      return;
    Serial.print("Mix config: O1 mute ");
    Serial.print(config->mix.oscmute[0] ? "T" : "F");
    Serial.print(", S1 mute ");
    Serial.print(config->mix.submute[0] ? "T" : "F");
    Serial.print(", O2 mute ");
    Serial.print(config->mix.oscmute[1] ? "T" : "F");
    Serial.print(", S2 mute ");
    Serial.println(config->mix.submute[1] ? "T" : "F");
    synth->loadMixerConfig(&config->mix);
  }
  else if (cmd[0] == '+' || cmd[0] == '-')
  {
    synth->globaloffset(atol(cmd), true);
    Serial.print("global offset = ");
    Serial.println(atol(cmd));
  }
  else if (strcasecmp(cmd, "mix") == 0 && a2)
  {
    long vol = min(127, max(0, atol(a2)));
    if (strcasecmp(a1, "o1") == 0)
      config->mix.osclevel[0] = vol;
    else if (strcasecmp(a1, "s1") == 0)
      config->mix.sublevel[0] = vol;
    if (strcasecmp(a1, "o2") == 0)
      config->mix.osclevel[1] = vol;
    else if (strcasecmp(a1, "s2") == 0)
      config->mix.sublevel[1] = vol;
    else
      return;
    printMixConfig(&config->mix);
    synth->loadMixerConfig(&config->mix);
  }
  else if (strcasecmp(cmd, "env") == 0 && a2)
  {
    config->env.attack = min(127, max(0, atol(a1)));
    config->env.decay = min(127, max(0, atol(a2)));
    printEnvelopeConfig(&config->env);
    synth->loadEnvConfig(&config->env);
  }
  else if (strcasecmp(cmd, "f") == 0 && a2)
  {
    if (a1[0] != '.')
      config->filter.cutoff = min(127, max(0, atol(a1)));
    if (a2[0] != '.')
      config->filter.resonance = min(127, max(0, atol(a2)));
    printFilterConfig(&config->filter);
    synth->loadFilterConfig(&config->filter);
  }
  else if (strcasecmp(cmd, "fmod") == 0 && a2)
  {
    if (a1[0] != '.')
      config->filter.envamt = min(63, max(-64, atol(a1)));
    if (a2[0] != '.')
      config->filter.modamt = min(127, max(0, atol(a2)));
    printFilterConfig(&config->filter);
    synth->loadFilterConfig(&config->filter);
  }
  else if (strcasecmp(cmd, "lfo") == 0 && a2)
  {
    if (a1[0] != '.')
      config->lfo.rate = min(127, max(0, atol(a1)));
    if (a2[0] != '.')
    {
      if (strcasecmp(a2, "sin") == 0)
        config->lfo.shape = SINE;
      else if (strcasecmp(a2, "tri") == 0)
        config->lfo.shape = TRIANGLE;
      else if (strcasecmp(a2, "sq") == 0)
        config->lfo.shape = SQUARE;
      else if (strcasecmp(a2, "saw") == 0)
        config->lfo.shape = SAWTOOTH;
      else if (strcasecmp(a2, "s+h") == 0)
        config->lfo.shape = S_AND_H;
    }
    printModulatorConfig(&config->lfo);
    synth->loadModConfig(&config->lfo);
  }
  else if (strcasecmp(cmd, "osc") == 0 && a3)
  {
    int i = getargnum(a1, false);
    if (i == 1 || i == 2)
    {
      i--;
      if (a2[0] != '.')
      {
        long f = atol(a2);
        if (f >= 40 && f <= 76)
          config->osc[i].freq = f;
      }
      if (a3[0] != '.')
      {
        if (strcasecmp(a3, "sin") == 0)
          config->osc[i].shape = SINE;
        else if (strcasecmp(a3, "tri") == 0)
          config->osc[i].shape = TRIANGLE;
        else if (strcasecmp(a3, "sq") == 0)
          config->osc[i].shape = SQUARE;
        else if (strcasecmp(a3, "saw") == 0)
          config->osc[i].shape = SAWTOOTH;
        else if (strcasecmp(a3, "p") == 0)
          config->osc[i].shape = PULSE;
      }
      printOscConfig(i, &config->osc[i]);
      synth->loadOscConfig(&config->osc[i], i);
    }
  }
  else if (strcasecmp(cmd, "omod") == 0 && a3)
  {
    int i = getargnum(a1, false);
    if (i == 1 || i == 2)
    {
      i--;
      if (a2[0] != '.')
      {
        config->osc[i].pwamt = min(127, max(0, atol(a2)));
      }
      if (a2[strlen(a2) - 1] == '+')
        config->osc[i].pwm = true;
      else if (a2[strlen(a2) - 1] == '-')
        config->osc[i].pwm = false;
      if (a3[0] != '.')
        config->osc[i].modamt = min(127, max(0, atol(a3)));
      printOscConfig(i, &config->osc[i]);
      synth->loadOscConfig(&config->osc[i], i);
    }
  }
  else if (strcasecmp(cmd, "sub") == 0 && a3)
  {
    int i = getargnum(a1, false);
    if (i == 1 || i == 2)
    {
      i--;
      if (a2[0] != '.')
      {
        long d = atol(a2);
        if (d >= 2 && d <= 16)
          config->osc[i].subfreq = d;
      }
      if (a3[0] != '.')
      {
        if (strcasecmp(a3, "sin") == 0)
          config->osc[i].subshape = SINE;
        else if (strcasecmp(a3, "tri") == 0)
          config->osc[i].subshape = TRIANGLE;
        else if (strcasecmp(a3, "sq") == 0)
          config->osc[i].subshape = SQUARE;
        else if (strcasecmp(a3, "saw") == 0)
          config->osc[i].subshape = SAWTOOTH;
        else if (strcasecmp(a3, "p") == 0)
          config->osc[i].subshape = PULSE;
      }
      printOscConfig(i, &config->osc[i]);
      synth->loadOscConfig(&config->osc[i], i);
    }
  }
  else if (strcasecmp(cmd, "det") == 0 && a1)
  {
    config->osc[0].detune = min(50, max(0, atol(a1)));
    config->osc[1].detune = min(50, max(0, atol(a1)));
    synth->loadOscConfig(&config->osc[0], 0);
    synth->loadOscConfig(&config->osc[1], 1);
  }
  else if (strcasecmp(cmd, "save") == 0 && a1)
  {
    Serial.println("Saving... <be careful>");
    if (!saveSerialStream(atol(a1)))
      Serial.println("Failed to save preset file");
  }
  else if (strcasecmp(cmd, "help") == 0 || strcmp(cmd, "?") == 0)
  {      
    Serial.println("\tHelp:      help|?        (print this message)");
    Serial.println("\tRun:       run|r");
    Serial.println("\tStop:      stop|.");
    Serial.println("\tHold:      hold|h");
    Serial.println("\tPause:     pause");
    Serial.println("\tSelect:    sel n         (n from 1 to 6)");
#if !PROTOTYPE    
    Serial.println("\tShift:     shift         (apply shift to current selection)");
#else    
    Serial.println("\tPress:     press n       (click encoder n [1-4])");
#endif    
    Serial.println("\tTurn:      turn n d      (n from 1 to 4, d +/- or value)");
    Serial.println("\tStep:      step i n d    (i = 1 or 2, n from 1 to 4, d +/- or value)");
    Serial.println("\tBeat:      beat i b123 o (i = 1 or 2, b123 is 0 to 7 and o is 0 or 1)");
    Serial.println("\tApply:     app i os      (i = 1 or 2, os is 0 to 3)");
    Serial.println("\tMeter:     meter n i j k (n = 1-3 rhythm, i = beats, j = length, k = offset)");
    Serial.println("\tTempo:     tempo t       (t is value 0-127, or +/- value for relative change)");
    Serial.println("\tPrint:     conf          (print the current config)");
    Serial.println("\tPreset:    pre n         (load preset n)");
    Serial.println("\tSave:      save n        (save file data for preset n)");
    Serial.println("\tDump:      dump n        (n or * to print one or all presets)");
    Serial.println("\tRestore:   rest n        (n or * to restore one or all presets)");
    Serial.println("\tMemory:    mem           (print maximum memory usage)");
    Serial.println("\tMix:       mix os vol    (os is o1, o2, s1 or s2, vol is 0 to 127)");
    Serial.println("\tMute:      mute os       (os is o1, o2, s1 or s2 - toggle mute status of osc1/sub1/osc2/sub2)");
    Serial.println("\tTranspose: +/-n          (transpose by the given number of semitones)");
    Serial.println("\tOsc:       osc i f sh    (set oscillator note [40,76] and shape - i = 1 or 2, sh [sin,tri,sq,saw,p])");
    Serial.println("\tSub-osc:   sub i d sh    (set sub-oscillator divisor [2,16] and shape - i = 1 or 2, sh [sin,tri,sq,saw,p])");
    Serial.println("\tOsc mod:   omod i pw mod (set oscillator pw [0,127](+/-) and modamt [0,127] - i = 1 or 2, optional (+/-) toggles pw mod)");
    Serial.println("\tDetune:    det n         (set detune to n [0,50] cents for both oscillators)");
    Serial.println("\tLFO:       lfo rate sh   (set lfo rate [0,127] and shape [sin,tri,sq,saw,s+h] - '.' for no change)");
    Serial.println("\tEnvelope:  env a d       (set attack and decay - [0,127] or '.' for no change)");
    Serial.println("\tFilter:    f co res      (set cutoff and resonance - [0,127] or '.' for no change)");
    Serial.println("\tFilter mod:fmod env mod  (set env amt [-64,63] and mod amt [0,127] - '.' for no change)");
    Serial.println("\tEffects:   eff e s       (set eff e [off,ch,fl,dl,chdl] to s [0,1,2,3]");
    Serial.println("\tVolume:    vol n         (n from 0 to 127)");
    Serial.println("\tDrum vol:  dv n          (drum volume from 0 to 127)");
  }
}

bool quickFormatCheck(char *str, int n)
{
  if (!strlen(str))
    return false;
  char c;
  int count = 1;
  for (int i = 0; (c = str[i]); i++)
  {
    if (c == ',')
      count++;
    else if (!isdigit(c) && c != '.' && c != '-')
      return false;
  }
  return count == n;
}

bool saveSerialStream(int n)
{
  Serial.print("Saving to preset file: ");
  Serial.println(n);
  if (n < 0 || n >= MAX_PRESETS)
  {
    Serial.println("Invalid preset number");
    return false;
  }

  int fieldcounts[] = {1, 8, 8, 8, 2, 2, 4, 4, 2, 10, 10, 10, 6};
  File tempFile;
  bool success = true;
  Serial.println("Enter the config data (do not include headers)");
  if (getTempPresetFile(n, &tempFile))
  {
    for (int i = 0; i < 13; ++i)
    {
      Serial.print("Line ");
      Serial.print(i + 1);
      Serial.print(": ");
      clearSerialInput();
      while (!getSerialInput())
        ;
      if (strlen(s_str) == 0)
      {
        Serial.println("Cancelling on empty line");
        success = false;
        break;
      }
      else if (!quickFormatCheck(s_str, fieldcounts[i]))
      {
        Serial.println("Invalid format");
        success = false;
        break;
      }
      tempFile.println(s_str);
    }

    convertTempPresetFile(n, &tempFile);
    tempFile.close();
    Serial.println("Data Loaded");
    Serial.flush();
  }

  return success;
}
