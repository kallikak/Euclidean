#pragma once

#include "config.h"
#include "Controls.h"

//#define LCD_BLACK      0x0000
//#define LCD_WHITE      0xFFFF
//#define LCD_RED        0xF800
//#define LCD_GREEN      0x07E0
//#define LCD_BLUE       0x001F
//#define LCD_CYAN       0x07FF
//#define LCD_MAGENTA    0xF81F     // 11111 000000 11111
//#define LCD_YELLOW     0xFFE0     // 11111 111111 00000
//#define LCD_ORANGE     0xFC00
//#define LCD_PINK       0xF81F

#define LCD_WHITE      0x0000
#define LCD_BLACK      0xFFFF
#define LCD_GREY       0x4444
#define LCD_CYAN       0xF800
#define LCD_RED        0x07FF
#define LCD_BLUE       0xFFE0
#define LCD_YELLOW     0x001F 
#define LCD_MAGENTA    0x07E0
#define LCD_GREEN      0xF81F
#define LCD_ORANGE     0x02FF
#define LCD_PINK       0x07E0

void setupLCD(void);

void showRhythm(int active, rhythmCfg *rhythm, bool shift);
void showTuning(tuningCfg *tuning);
void showTempo(int bpm);
void showRhythmGraph(Polyrhythm *poly, int activemeter);
void showNextBeats(int *next, int n);
void showSequencerLabels(bool permuting1, bool permuting2);
void showSequencer(int seqnum, const char *text, int i);
void textatrow(int r, const char *text, uint16_t fg, uint16_t bg);
void smallertextatrow(int r, const char *text, uint16_t fg, uint16_t bg);
void textathalfrow(int r, bool lower, const char *text, uint16_t fg, uint16_t bg, int shiftdn, int shiftin, bool monospace = false);
