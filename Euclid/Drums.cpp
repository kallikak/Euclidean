#include "Drums.h"
#include "Config.h"

#include "drum_defs.h"
#include "drum_samples.h"

drumCfg *curCfg = NULL;//&(config->drum);
float drumvol = 4;

typedef struct
{
  int beats[4];
} drumKit;

#define NUM_KITS 13
const drumKit kits[NUM_KITS] = 
{
  {DK_BASS, DK_SNARE, DK_TOM_H, DK_HAT_C}, 
  {DK_BASS, DK_SNARE, DK_HAT_O, DK_HAT_C}, 
  {DK_BASS, DK_SNARE, DK_CYMBAL, DK_HAT_C}, 
  {DK_BASS, DK_SNARE, DK_HAT_C, DK_RIDECYMBAL},  
  {DK_BASS, DK_SNARE, DK_HAT_C, DK_HAT_O}, 
  {DK_BASS, DK_SNARE, DK_SNARE, DK_HAT_C}, 
  {DK_TOM_L, DK_TOM_M, DK_TOM_H, DK_HAT_C}, 
  {DK_TOM_L, DK_TOM_M, DK_TOM_H, DK_CLAVES},
  {DK_BASS, DK_BASS, DK_BASS, DK_HAT_O},
  {DK_SNARE, DK_SNARE, DK_SNARE, DK_HAT_O},
  {DK_SNARE, DK_SNARE, DK_RIMSHOT, DK_HAT_O},
  {DK_CONGA_L, DK_CONGA_M, DK_CONGA_H, DK_HAT_C},
  {DK_CONGA_L, DK_CONGA_M, DK_CONGA_H, DK_CLAVES},
};

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
}

void setDrumsConfig(drumCfg *cfg)
{
  curCfg = cfg;
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
    drummix->gain(0, curCfg->beatstate == BT_OFF ? 0 : getVolumeFactor(curCfg->beats[BEAT1]) * drumvol);
    drummix->gain(1, curCfg->beatstate == BT_OFF ? 0 : getVolumeFactor(curCfg->beats[BEAT2]) * drumvol);
    drummix->gain(2, curCfg->beatstate == BT_OFF ? 0 : getVolumeFactor(curCfg->beats[BEAT3]) * drumvol);
  }
}

void manageoffbeatstate(bool back)
{
  curCfg->offbeatstate = (beatState)((curCfg->offbeatstate + 5 + (back ? -1 : 1)) % 5);
  drummix->gain(3, curCfg->offbeatstate == BT_OFF ? 0 : getVolumeFactor(curCfg->beats[OFFBEAT]) * drumvol);
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
  drummix->gain(0, curCfg->beatstate == BT_OFF ? 0 : getVolumeFactor(curCfg->beats[BEAT1]) * drumvol);
  drummix->gain(1, curCfg->beatstate == BT_OFF ? 0 : getVolumeFactor(curCfg->beats[BEAT2]) * drumvol);
  drummix->gain(2, curCfg->beatstate == BT_OFF ? 0 : getVolumeFactor(curCfg->beats[BEAT3]) * drumvol);
  drummix->gain(3, curCfg->offbeatstate == BT_OFF ? 0 : getVolumeFactor(curCfg->beats[OFFBEAT]) * drumvol);
  return drumvol / 4.0;
}

void managedrumclick(bool back)
{
  kitIndex += (back ? -1 : 1);
  if (kitIndex >= NUM_KITS)
    kitIndex = 0;
  else if (kitIndex < 0)
    kitIndex = NUM_KITS - 1;

  curCfg->beats[BEAT1] = kits[kitIndex].beats[BEAT1];
  curCfg->beats[BEAT2] = kits[kitIndex].beats[BEAT2];
  curCfg->beats[BEAT3] = kits[kitIndex].beats[BEAT3];
  curCfg->beats[OFFBEAT] = kits[kitIndex].beats[OFFBEAT];
}

void manageoffbeatclick(bool back)
{
  curCfg->beats[OFFBEAT] = curCfg->beats[OFFBEAT] + (back ? -1 : 1);
  if (curCfg->beats[OFFBEAT] > DK_LAST)
    curCfg->beats[OFFBEAT] = DK_BASS;
  else if (curCfg->beats[OFFBEAT] < DK_BASS)
    curCfg->beats[OFFBEAT] = DK_LAST;
}

void playDrumSample(Beat beat, bool accent)
{
  AudioPlayMemory *sound = sounds[beat];
  int instr = curCfg->beats[beat];

  if (beat == BEAT1)
    drummix->gain(0, (accent ? 1.5 : 1) * drumvol);
  else if (beat == BEAT2)
    drummix->gain(1, (accent ? 1.5 : 1) * drumvol);
  else if (beat == BEAT3)
    drummix->gain(2, (accent ? 1.5 : 1) * drumvol);
    
  switch (instr)
  {
    case DK_BASS: 
      sound->play(DK_BASS_ARRAY);
      break;
    case DK_HAT_C: 
      sound->play(DK_HAT_C_ARRAY);
      break;
    case DK_HAT_O: 
      sound->play(DK_HAT_O_ARRAY);
      break;
    case DK_SNARE: 
      sound->play(DK_SNARE_ARRAY);
      break;
    case DK_TOM_L: 
      sound->play(DK_TOM_L_ARRAY);
      break;
    case DK_TOM_M: 
      sound->play(DK_TOM_M_ARRAY);
      break;
    case DK_TOM_H: 
      sound->play(DK_TOM_H_ARRAY);
      break;
    case DK_CLAVES: 
      sound->play(DK_CLAVES_ARRAY);
      break;
    case DK_CONGA_H: 
      sound->play(DK_CONGA_H_ARRAY);
      break;
    case DK_CONGA_L: 
      sound->play(DK_CONGA_L_ARRAY);
      break;
    case DK_CONGA_M: 
      sound->play(DK_CONGA_M_ARRAY);
      break;
    case DK_CYMBAL: 
      sound->play(DK_CYMBAL_ARRAY);
      break;
    case DK_MARACAS: 
      sound->play(DK_MARACAS_ARRAY);
      break;
    case DK_RIMSHOT: 
      sound->play(DK_RIMSHOT_ARRAY);
      break;
    case DK_RIDECYMBAL: 
      sound->play(DK_RIDECYMBAL_ARRAY);
      break;
    case DK_OFF:
    default:
      break;
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
