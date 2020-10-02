#include <MIDI.h>
#include "USBHost_t36.h"

#include "Config.h"
#include "Controls.h"
#include "Sequencer.h"
#include "MIDIManager.h"
#include "Utility.h"
#include "LCD.h"

MIDI_CREATE_DEFAULT_INSTANCE();

#define NO_NOTE 0
#define BASE_NOTE 48

static byte currentNote = NO_NOTE;
static bool playingOsc1 = false;
static bool playingOsc2 = false;

static USBHost usbHost;
static USBHIDParser hid1(usbHost);
static USBHIDParser hid2(usbHost);
static USBHub hub1(usbHost);
static MIDIDevice usbmidi(usbHost);

static int clockCount = 0;

void managePlayState()
{
  bool play1 = (!poly->running || !seq1->isActive()) && !synth->droning && !poly->pause;
  bool play2 = (!poly->running || !seq2->isActive()) && !synth->droning && !poly->pause;
  if (!play1)
  {
    if (playingOsc1)
    {
      playingOsc1 = false;
      synth->cancelPlay(0);
    }
  }
  else
  {
    synth->setupPlay(0);
    playingOsc1 = true;
  }
  
  if (!play2)
  {
    if (playingOsc2)
    {
      playingOsc2 = false;
      synth->cancelPlay(1);
    }
  }
  else
  {
    synth->setupPlay(1);
    playingOsc2 = true;
  }
#if DEBUG_SERIAL  
//  Serial.print("poly->running = ");
//  Serial.print(poly->running);
//  Serial.print(", poly->pause = ");
//  Serial.print(poly->pause);
//  Serial.print(", synth->droning = ");
//  Serial.print(synth->droning);
//  Serial.print(", seq1->isActive() = ");
//  Serial.print(seq1->isActive());
//  Serial.print(", seq2->isActive() = ");
//  Serial.println(seq2->isActive());
//  Serial.print(" => play1 = ");
//  Serial.print(play1);
//  Serial.print(", play2 = ");
//  Serial.println(play2);
  Serial.print("managePlayState: playingOsc1 = ");
  Serial.print(playingOsc1);
  Serial.print(", playingOsc2 = ");
  Serial.println(playingOsc2);
#endif  
}

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
  char tempstr[50] = {0};
#if DEBUG_SERIAL  
  Serial.println("note on");
  sprintf(tempstr, "Osc %d = %s on. ", pitch, notestring((int)round(pitch - 8)));
  Serial.print(tempstr);
#endif  
  currentNote = pitch;
  
  if (playingOsc1)
  {
    if (isOff())
      switchOn();
    float delta = config->osc[0].freq - BASE_NOTE;
    float f = noteToFreq(currentNote + delta);
    synth->play(f, -1, 0);
#if DEBUG_SERIAL    
    Serial.print("O1 freq: ");
    Serial.print(f);
    Serial.print(", ");
#endif    
  }
  
  if (playingOsc2)
  {
    if (isOff())
      switchOn();
    float delta = config->osc[1].freq - BASE_NOTE;
    float f = noteToFreq(currentNote + delta);
    synth->play(f, -1, 1);
#if DEBUG_SERIAL    
    Serial.print("O2 freq: ");
    Serial.print(f);
    Serial.print(", ");
#endif    
  }

  bool blockTranspose = synth->droning || poly->pause;
  if (!blockTranspose && !playingOsc1 && !playingOsc2)
  {
    config->tuning.transpose = currentNote - BASE_NOTE;
    synth->globaloffset(config->tuning.transpose, true);
    textatrow(1, "Transpose", LCD_BLACK, LCD_WHITE);
    sprintf(tempstr, "%+2d", currentNote - BASE_NOTE);
    textatrow(2, tempstr, LCD_BLACK, LCD_WHITE);
    lastupdatemillis = millis();
#if DEBUG_SERIAL    
    Serial.print("Setting transpose to ");
    Serial.print(currentNote - BASE_NOTE);
#endif    
  }
#if DEBUG_SERIAL
  Serial.println();
#endif  
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
  if (pitch == currentNote)
  {
    currentNote = NO_NOTE;
    if (playingOsc1)
    {
      synth->noteoff(0);
    }
    if (playingOsc2)
    {
      synth->noteoff(1);
    }
#if DEBUG_SERIAL    
    Serial.println("Note off ");
#endif    
  }
}

unsigned long lastbeattime = 0;
float bpm = 200;
int lastupdatebpm = 0;
int samples = 0;

void handleClock()
{
#if DEBUG_SERIAL    
  Serial.println("MIDI clock");
#endif     
  clockCount++;
  samples++;
  unsigned long thisbeattime = micros();
  float newbpm = 60.0 * 1000 * 1000.0 / MIDI_CLOCKS_PER_BEAT / (thisbeattime - lastbeattime);
  if (samples > 20 && thisbeattime > lastbeattime)
  {
    bpm = movingaveragefloat(newbpm, bpm, bpm == 0 ? 0 : round(2 * bpm / 40));
#if DEBUG_SERIAL    
    Serial.print("Millis since last beat: ");
    Serial.print((thisbeattime - lastbeattime) / 1000.0);
    Serial.print(" => bpm of ");
    Serial.println(bpm);
#endif     
  }
  if (clockCount == MIDI_CLOCKS_PER_BEAT) // should be 24...
  {
    clockCount = 0;
    if (lastbeattime)
    {
      long rbpm = round(bpm);
//    if (lastupdatebpm != rbpm)
      if (rbpm > 0 && (lastupdatebpm < 60 || abs(lastupdatebpm - rbpm) > 2))
      {
        showTempo(rbpm);
        lastupdatebpm = rbpm;
      }
    }
    extClockFire();
  }
  lastbeattime = thisbeattime;
}

void handleStart()
{
#if DEBUG_SERIAL    
  Serial.println("MIDI start");
#endif     
  lastbeattime = 0;
  lastupdatebpm = 0;
  bpm = 0;
  clockCount = 0;
  samples = 0;
  buttonPressed(RUN, 1);
}

void handleContinue()
{
#if DEBUG_SERIAL    
  Serial.println("MIDI continue");
#endif     
  lastbeattime = 0;
  lastupdatebpm = 0;
  bpm = 0;
  samples = 0;
  buttonPressed(RUN, 1);
}

void handleStop()
{
#if DEBUG_SERIAL    
  Serial.println("MIDI stop");
#endif      
  buttonPressed(STOP, 1);
}

void resetMIDI()
{
  currentNote = NO_NOTE;
  synth->noteoff(0);
  synth->noteoff(1);
  stopAllMIDI();
}

void setupMIDI()
{
  DBG(useMIDIClock)
  MIDI.begin(MIDI_CHANNEL_OMNI);   
  MIDI.setHandleNoteOn(handleNoteOn);  // Put only the name of the function
  MIDI.setHandleNoteOff(handleNoteOff);
  if (useMIDIClock)
  {
    MIDI.setHandleClock(handleClock);
    MIDI.setHandleStart(handleStart);
    MIDI.setHandleContinue(handleContinue);
    MIDI.setHandleStop(handleStop);
  }
  
  usbHost.begin();
  usbmidi.setHandleNoteOn(handleNoteOn);
  usbmidi.setHandleNoteOff(handleNoteOff);
  if (useMIDIClock)
  {
    usbmidi.setHandleClock(handleClock);
    usbmidi.setHandleStart(handleStart);
    usbmidi.setHandleContinue(handleContinue);
    usbmidi.setHandleStop(handleStop);
  }
}

void checkMIDI()
{
  MIDI.read();
  usbHost.Task();
  usbmidi.read();
}

static int count = 0;

void sendMIDINoteOn(byte note, byte vel, byte chan)
{
  MIDI.sendNoteOn(note, vel, chan);
  usbmidi.sendNoteOn(note, vel, chan);
#if DEBUG_SERIAL 
  Serial.print(count / 12.0);
  Serial.print(": Note on: ");
  Serial.print(note);
  Serial.print(" with vel: ");
  Serial.print(vel);
  Serial.print(" on channel: ");
  Serial.println(chan);
#endif  
}

void sendMIDINoteOff(byte note, byte chan)
{
  MIDI.sendNoteOff(note, 0, chan);
  usbmidi.sendNoteOff(note, 0, chan);
#if DEBUG_SERIAL  
  Serial.print(count / 12.0);
  Serial.print(": Note off: ");
  Serial.print(note);
  Serial.print(" on channel: ");
  Serial.println(chan);
#endif  
}

void stopAllMIDI()
{
  MIDI.sendControlChange(123, 0, 1);
  MIDI.sendControlChange(123, 0, 2);
  usbmidi.sendControlChange(123, 0, 1);
  usbmidi.sendControlChange(123, 0, 2);
#if DEBUG_SERIAL  
  Serial.println("All MIDI notes off");
#endif  
}

#if DEBUG_SERIAL  
static unsigned long last;
static int sum = 0;
#endif

void sendMIDIClock()
{
  count++;
  if (useMIDIClock)
    return;
  MIDI.sendRealTime(midi::Clock);
  usbmidi.sendRealTime(usbmidi.Clock);
#if DEBUG_SERIAL  
  unsigned long now = micros();
  sum += (now - last);
  if (count == 12)
  {
    Serial.print("MIDI clock ");
    Serial.println(sum / 1000.0);
    sum = 0;
    count = 0;
  }
  last = now;
#endif
}

void sendMIDIClockStart()
{
  count = 0;
  if (useMIDIClock)
    return;
  MIDI.sendRealTime(midi::Start);
  usbmidi.sendRealTime(usbmidi.Start);
#if DEBUG_SERIAL  
  Serial.print("MIDI clock start ");
  last = micros();
  Serial.println(last);
  sum = 0;
#endif
}

void sendMIDIClockContinue()
{
  if (useMIDIClock)
    return;
  MIDI.sendRealTime(midi::Continue);
  usbmidi.sendRealTime(usbmidi.Continue);
}

void sendMIDIClockStop()
{
  if (useMIDIClock)
    return;
  MIDI.sendRealTime(midi::Stop);
  usbmidi.sendRealTime(usbmidi.Stop);
}
