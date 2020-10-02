#include <math.h>

#include "Config.h"
#include "Utility.h"

bool needQuantFunctions();
float noteToFreqQ(float n, quantisation q);   
float freqToNoteQ(float f, quantisation q);

float quantise(float f, tuningCfg *cfg, bool transpose)
{
  // C C# D D# E F F# G G# A A# B 
  // 0 1  2 3  4 5 6  7 8  9 10 11
  // diatonic:     0 2 4 5 7 9 11
  // diatonic_m:   0 2 3 5 7 8 11
  // pentatonic:   0 2 5 7 9
  // pentatonic_m: 0 3 5 7 10
  
  int n = freqToNote(f);
  if (cfg->scale == FREE)
  {
    float delta = f - noteToFreq(n);
    int f1 = transpose ? noteToFreq(n + cfg->transpose) : noteToFreq(n);
    return f1 + delta;
  }   

  if (transpose)
    n += cfg->transpose;
  if (cfg->scale == CHROMATIC)
    return noteToFreq(n);

  int k = (144 + n + 8) % 12;   
  if (cfg->scale == DIATONIC || cfg->scale == JUST || cfg->scale == PYTHAGOREAN)
  {
    // if sharp/flat then drop a semitone
    if (k == 1 || k == 3 || k == 6 || k == 8 || k == 10)
      n--;
    return needQuantFunctions() ? noteToFreqQ(n, config->tuning.scale) : noteToFreq(n);
  }

  if (cfg->scale == DIATONIC_M || cfg->scale == JUST_M || cfg->scale == PYTHAGOREAN_M)
  {
    // if sharp/flat then drop a semitone
    if (k == 1 || k == 4 || k == 6 || k == 9)
      n--;
    else if (k == 10)
      n -= 2;
    return needQuantFunctions() ? noteToFreqQ(n, config->tuning.scale) : noteToFreq(n);
  }

  if (cfg->scale == PENTATONIC)
  {
    // 0 2 5 7 9
    switch (k)
    {
      case 1:
      case 3:
      case 6:
      case 8:
      case 10:
        n--;
        break;
      case 4:
      case 11:
        n -= 2;
        break;
    }
    return noteToFreq(n);
  }

  if (cfg->scale == PENTATONIC_M)
  {
    // 0 3 5 7 10
    switch (k)
    {
      case 1:
      case 4:
      case 6:
      case 8:
      case 11:
        n--;
        break;
      case 2:
      case 9:
        n -= 2;
        break;
    }
    return noteToFreq(n);
  }

  return f;
}

float freqToNote(float f)
{
  return 12 * log2(f / 440.0) + 49;
//  return round(12 * log2(f / 440.0) + 49);
//  return roundnotenumber(12 * log2(f / 440.0) + 49);
}

float noteToFreq(float n)
{
    //  f = 440 * 2 ^ ((n - 49) / 12)
    return pow(2, (n - 49) / 12.0) * 440;
}

float freqToMIDINote(float f)
{
  return 20 + freqToNote(f);
}

/* 
 * Just intonation major octave ratios: 0 9/8 5/4 4/3 3/2 5/3 15/8 2
 * Just intonation minor octave ratios: 0 9/8 6/5 4/3 3/2 8/5 15/8 2
 * 
 * Pythagorean major octave ratios: 0 9/8 81/64 4/3 3/2 27/16 243/128 2
 * Pythagorean minor octave ratios: 0 9/8 32/27 4/3 3/2 128/81 243/128 2
 */

float just[] = {261.6256f, 294.3288f, 327.032f, 348.834133f, 392.4384f, 436.04267f, 490.548f, 523.2512f};
float just_m[] = {261.6256f, 294.3288f, 313.95072f, 348.834133f, 392.4384f, 418.60096f, 490.548f, 523.2512f};
float pyth[] = {261.6256f, 294.3288f, 331.1199f, 348.834133f, 392.4384f, 441.4932f, 496.67985f, 523.2512f};
float pyth_m[] = {261.6256f, 294.3288f, 310.0748f, 348.834133f, 392.4384f, 413.4330f, 496.67985f, 523.2512f};

bool needQuantFunctions()
{
  switch (config->tuning.scale)
  {
    case PYTHAGOREAN:
    case PYTHAGOREAN_M:
    case JUST:
    case JUST_M:
      return true;
    default:
      return false;
  }
}

float noteToFreqQ(float n, quantisation q)
{
  int shift = 0;
  float f;
  float *ta;
  int *ints;

  int is[] = {0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6, 7};
  int is_m[] = {0, 0, 1, 2, 2, 3, 3, 4, 5, 5, 5, 6, 7};

  switch (q)
  {
    case PYTHAGOREAN:
      ta = pyth;
      ints = is;
      break;
    case PYTHAGOREAN_M:
      ta = pyth_m;
      ints = is_m;
      break;
    case JUST:
      ta = just;
      ints = is;
      break;
    case JUST_M:
      ta = just_m;
      ints = is_m;
      break;
    default:
      return noteToFreq(n);
  }

  // get into our desired octave
  int k = round(n);
  while (k >= 52)
  {
    k -= 12;
    shift--;
  }
  while (k < 40)
  {
    k += 12;
    shift++;
  }
  // find the reference frequency
  f = ta[ints[k - 40]];
  if (shift < 0)
  {
    while (shift++)
      f *= 2;
  }
  else if (shift > 0)
  {
    while (shift--)
      f /= 2;
  }

  return f;
}
    
float freqToNoteQ(float f, quantisation q)
{
  int shift = 0;
  int i, n;
  float *ta;
  int *ints;
  
  static int is[] = {0, 2, 4, 5, 7, 9, 11, 12};
  static int is_m[] = {0, 2, 3, 5, 7, 8, 11, 12};

  switch (q)
  {
    case PYTHAGOREAN:
      ta = pyth;
      ints = is;
      break;
    case PYTHAGOREAN_M:
      ta = pyth_m;
      ints = is_m;
      break;
    case JUST:
      ta = just;
      ints = is;
      break;
    case JUST_M:
      ta = just_m;
      ints = is_m;
      break;
    default:
      return freqToNote(f);
  }

  // get into our desired octave
  while (f > ta[7])
  {
    f /= 2;
    shift--;
  }
  while (f < ta[0])
  {
    f *= 2;
    shift++;
  }
  // find the note range
  for (i = 0; i < 7; ++i)
  {
    if (f >= ta[i] && f <= ta[i + 1])
      break;
  }
  // which is closest
  if ((f - ta[i]) > (ta[i + 1] - f))
    n = i + 1;
  else
    n = i;

  n = 40 + ints[n] - 12 * shift;
  return n;
}

float offsetToFreq(int k, float f0)
{
//  int n = freqToNote(f0) + k;
//  return noteToFreq(n) - f0;
  float n = 12 * log2(f0 / 440) + 49 + k;
  return pow(2, (n - 49) / 12.0) * 440 - f0;
}

int roundnotenumber(float n)
{
  if (n > 80)
    return floor(n + 0.3); // allow 70 cents sharp to still identify as the lower note
  else if (n > 60)
    return floor(n + 0.4); // here allow 60
  else if (n < 15)
    return floor(n + 0.6); // low base allow 60 cents flat
  else if (n < 3)
    return floor(n + 0.7); // lowest base allow 70 cents flat
  else
    return floor(n + 0.5); // otherwise just round off
};

static char tempnotestr[10] = {0};
const char *notestring(float n)
{
  static const char* notes[] = {"A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#"};

  int k = roundnotenumber(n);
  int i = (k + 11) % 12;
  if (i < 0)
    return "-";
  sprintf(tempnotestr, "%s%d", notes[(k + 11) % 12], (int)floor((k + 8) / 12));
  return tempnotestr;
}

float detuneCents(float f, int c)
{
  return f * pow(2, -1.0 * c / 1200);
}

int16_t movingaverage(int16_t newvalue, int16_t avg, int n)
{
  return (n * avg + newvalue) / (n + 1);
//  // (3 * avg + 1 * newdat) / 4
//  return (((avg << 2) - avg + newdat) + 2) >> 2;
}

float movingaveragefloat(float newvalue, float avg, int n)
{
  return (n * avg + newvalue) / (n + 1);
}
