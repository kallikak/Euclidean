#include "Drums.h"
#include "Config.h"

#include "drum_samples.h"

drumCfg *curCfg = NULL;//&(config->drum);
float drumvol = 4;

static int kitIndex = 0;

static AudioMixer4 *drummix = NULL;
static AudioPlayMemory *sounds[4] = {NULL, NULL, NULL, NULL};

void initDrums(AudioMixer4 *mix, AudioPlayMemory *s1, AudioPlayMemory *s2, AudioPlayMemory *s3, AudioPlayMemory *s4)
{
  drummix = mix;
  sounds[0] = s1;
  sounds[1] = s2;
  sounds[2] = s3;
  sounds[3] = s4;
  curCfg = &(config->drum);
  initSamples();
  updateSamples(curCfg->beats[0], curCfg->beats[1], curCfg->beats[2], curCfg->beats[3]);
}

void setDrumsConfig(drumCfg *cfg)
{
  curCfg = cfg;
  loadSampleSet(cfg->sampleset);
}

bool drumsEnabled()
{
  return curCfg->beatstate != BT_OFF || curCfg->offbeatstate != BT_OFF;
}

bool isDrum(Beat beat) 
{
  if (!drumsEnabled() || curCfg->beats[beat] == DK_OFF)
    return false;
  else
    return (beat == OFFBEAT ? curCfg->offbeatstate : curCfg->beatstate) != BT_OFF;
}

void managebeatstate(bool back)
{
  curCfg->beatstate = (beatState)((curCfg->beatstate + 5 + (back ? -1 : 1)) % 5);
  if (curCfg->beatstate == BT_NONE)
    managebeatstate(back);
  else
  {
    drummix->gain(0, curCfg->beatstate == BT_OFF ? 0 : drumvol);
    drummix->gain(1, curCfg->beatstate == BT_OFF ? 0 : drumvol);
    drummix->gain(2, curCfg->beatstate == BT_OFF ? 0 : drumvol);
  }
}

void manageoffbeatstate(bool back)
{
  curCfg->offbeatstate = (beatState)((curCfg->offbeatstate + 5 + (back ? -1 : 1)) % 5);
  drummix->gain(3, curCfg->offbeatstate == BT_OFF ? 0 : drumvol);
}

float adjustDrumsVolume(int d)
{
#if PROTOTYPE
  drumvol = max(min(4, drumvol + d / 100.0), 0);
#else  
  // make logarithmic f () k x ~ {Int (x / (1 + (1 - x / 1023) * k))}
  int k = 2;
  int log_u = min(127, 3.0 * d / (1 + k * (1 - d / 127)));
  drumvol = 4 * log_u / 127.0;  // new samples are fairly low volume
#endif  
  drummix->gain(0, curCfg->beatstate == BT_OFF ? 0 : drumvol);
  drummix->gain(1, curCfg->beatstate == BT_OFF ? 0 : drumvol);
  drummix->gain(2, curCfg->beatstate == BT_OFF ? 0 : drumvol);
  drummix->gain(3, curCfg->offbeatstate == BT_OFF ? 0 : drumvol);
  return drumvol / 4.0;
}

void managedrumclick(bool back)
{
  kitIndex += (back ? -1 : 1);
  if (kitIndex >= numKits)
    kitIndex = 0;
  else if (kitIndex < 0)
    kitIndex = numKits - 1;
  
  curCfg->beats[BEAT1] = kits[kitIndex].beats[BEAT1];
  curCfg->beats[BEAT2] = kits[kitIndex].beats[BEAT2];
  curCfg->beats[BEAT3] = kits[kitIndex].beats[BEAT3];
  curCfg->beats[OFFBEAT] = kits[kitIndex].beats[OFFBEAT];
  
  // kits[kitIndex].beats[BEAT1] != curCfg->beats[BEAT1]  
  // updateSamples(curCfg->beats[0], curCfg->beats[1], curCfg->beats[2], curCfg->beats[3]);
  needsKitUpdate = true;
}

void manageoffbeatclick(bool back)
{
  curCfg->beats[OFFBEAT] = curCfg->beats[OFFBEAT] + (back ? -1 : 1);
  if (curCfg->beats[OFFBEAT] > DK_LAST)
    curCfg->beats[OFFBEAT] = DK_FIRST;
  else if (curCfg->beats[OFFBEAT] < DK_FIRST)
    curCfg->beats[OFFBEAT] = DK_LAST;
  // updateSamples(DK_OFF, DK_OFF, DK_OFF, curCfg->beats[3]);
  needsOffbeatUpdate = true;
}

void playDrumSample(Beat beat, bool accent)
{
  AudioPlayMemory *sound = sounds[beat];

  if (beat == BEAT1)
    drummix->gain(0, (accent ? 1.5 : 1) * drumvol);
  else if (beat == BEAT2)
    drummix->gain(1, (accent ? 1.5 : 1) * drumvol);
  else if (beat == BEAT3)
    drummix->gain(2, (accent ? 1.5 : 1) * drumvol);

  switch (beat)
  {
    case BEAT1: if (drumSample1) sound->play(drumSample1); break;
    case BEAT2: if (drumSample2) sound->play(drumSample2); break;
    case BEAT3: if (drumSample3) sound->play(drumSample3); break;
    case OFFBEAT: if (drumSample4) sound->play(drumSample4); break;
  }
}

static char tempStr[80];
const char *getKitString(bool shortDesc)
{
  sprintf(tempStr, "%s/%s/%s/%s",
    getBeatString(BEAT1, shortDesc), getBeatString(BEAT2, shortDesc), 
    getBeatString(BEAT3, shortDesc), getBeatString(OFFBEAT, shortDesc));
  return tempStr;
}

const char *getBeatString(Beat beat, bool shortDesc)
{
  return getInstrumentDesc(curCfg->beats[beat], shortDesc);
//  if (isDrum(beat))
//    return getInstrumentDesc(curCfg->beats[beat], shortDesc);
//  else
//    return "Off";
}
