#define MY_USE_FB 0

#if MY_USE_FB
#define USE_FRAME_BUFFER
#endif

#include <string.h>
#include <stdio.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <ST7735_t3.h> // Hardware-specific library
//#include <ST7789_t3.h> // Hardware-specific library
//#include <SPI.h>

#include "LCD.h"

#include "st7735_t3_font_Arial.h"
#include "st7735_t3_font_ArialBold.h"
#include "st7735_t3_font_ArialBoldItalic.h"
#include "font_LiberationMono.h"

// LCD defines

#define TFT_MISO  12
#define TFT_MOSI  11  //a12
#define TFT_SCK   13  //a13
#define TFT_DC   16 
#define TFT_CS   14  
#define TFT_RST  17
#define TFT_LITE 15

#if MY_USE_FB
#ifdef USE_FRAME_BUFFER
#define UpdateScreen _ptft->updateScreenAsync 
#else
#define UpdateScreen _ptft->updateScreen
#endif
#endif

static char tempstr[5] = {0};

ST7735_t3 tft = ST7735_t3(TFT_CS, TFT_DC, /*TFT_MOSI, TFT_SCK, */TFT_RST);

#define LCD_WIDTH 160
#define LCD_HEIGHT 128
#define ROW_HEIGHT 16

void setupLCD(void) 
{  
  tft.initR(INITR_BLACKTAB);
//  tft.init(LCD_HEIGHT, LCD_WIDTH);

  pinMode(TFT_LITE, OUTPUT);
  digitalWrite(TFT_LITE, HIGH);

  tft.fillScreen(ST7735_BLACK);
  tft.invertDisplay(true);

  tft.setRotation(3);
#if MY_USE_FB
  if (!tft.useFrameBuffer(true))
    Serial.println("Framebuffer failed");
  else
    Serial.println("Framebuffer success");
#endif
  tft.setTextWrap(false);
  tft.setFont(Arial_14_Bold_Italic);
  tft.setTextColor(LCD_WHITE);
  tft.fillRect(0, 0, LCD_WIDTH, ROW_HEIGHT + 8, LCD_BLUE);
  tft.setCursor(22, 2);
  tft.print("EUCLIDEAN");

  textatrow(1, "", LCD_BLACK, LCD_WHITE);
  textatrow(2, "", LCD_BLACK, LCD_WHITE);
  showRhythm(0, &config->rhythm, true);
  showSequencerLabels(false, false);
  
#if MY_USE_FB
  tft.updateScreen();
#endif  
}

void dotextatrow(int r, const char *text, uint16_t fg, uint16_t bg, const ILI9341_t3_font_t font) 
{
  int16_t x = 0;
  int16_t y = r * ROW_HEIGHT + 4;
//  int16_t x1, y1;
//  uint16_t w, h;
//  
//  tft.getTextBounds(text, x, y, &x1, &y1, &w, &h);
  tft.setFont(font);
  tft.fillRect(x, y - 2, LCD_WIDTH, ROW_HEIGHT + 1, bg);
  tft.setCursor(x + 8, y);
  tft.setTextColor(fg);
  tft.print(text);
#if MY_USE_FB
  tft.updateScreen();
#endif  
}

void smallertextatrow(int r, const char *text, uint16_t fg, uint16_t bg)
{
  dotextatrow(r, text, fg, bg, Arial_9_Bold);
}

void textatrow(int r, const char *text, uint16_t fg, uint16_t bg) 
{
  dotextatrow(r, text, fg, bg, Arial_12_Bold);
}

void textathalfrow(int r, bool lower, const char *text, uint16_t fg, uint16_t bg, int shiftdn, int shiftin, bool monospace) 
{
  int16_t x = 0;
  int16_t y = r * ROW_HEIGHT + 4 + shiftdn;
  if (lower)
    y += ROW_HEIGHT / 2 + 5;
//  int16_t x1, y1;
//  uint16_t w, h;
//  
//  tft.getTextBounds(text, x, y, &x1, &y1, &w, &h);
  if (monospace)
    tft.setFont(LiberationMono_8);
  else
    tft.setFont(Arial_8);
  tft.fillRect(x, y - 1, LCD_WIDTH, ROW_HEIGHT / 2 + 3, bg);
  tft.setCursor(x + shiftin, y);
  tft.setTextColor(fg);
  tft.print(text);
#if MY_USE_FB
  tft.updateScreen();
#endif  
}

void showSequencerLabels(bool permuting1, bool permuting2)
{
  int16_t x = 0;
  int16_t y = 3 * ROW_HEIGHT + 5;
  tft.setFont(Arial_8);
  tft.fillRect(x, y - 1, 68, ROW_HEIGHT + 6, LCD_BLACK);
  tft.setTextColor(permuting1 ? LCD_GREEN : LCD_YELLOW);
  tft.setCursor(x + 2, y);
  tft.print("Sequencer 1: ");
  tft.setTextColor(permuting2 ? LCD_GREEN : LCD_YELLOW);
  tft.setCursor(x + 2, y + ROW_HEIGHT / 2 + 3);
  tft.print("Sequencer 2: ");
#if MY_USE_FB
  tft.updateScreen();
#endif
}

void showSequencer(int seqnum, const char *text, int i) 
{
  int16_t x = 68 + 22 * i;
  int16_t y = 3 * ROW_HEIGHT + 5;
  if (seqnum)
    y += ROW_HEIGHT / 2 + 3;

  tft.setFont(Arial_8);
  tft.fillRect(x, y - 1, i == 3 ? 30 : 24, ROW_HEIGHT / 2 + 3, LCD_BLACK);
  
  int16_t  x1, y1;
  uint16_t w, h;
  tft.getTextBounds(text, x, y, &x1, &y1, &w, &h);
  x += 21 - w;
  
  tft.setCursor(x, y);
  tft.setTextColor(LCD_YELLOW);
  tft.print(text);
#if MY_USE_FB
  tft.updateScreen();
#endif  
}

void showTempo(int bpm)
{
  tft.setFont(Arial_12_Bold);
  tft.setTextColor(LCD_BLACK);
  int16_t x = 0;
  int16_t y = 4 * ROW_HEIGHT + 14;
  
  tft.fillRect(x, y - 4, 105, ROW_HEIGHT + 2, LCD_YELLOW);
  char tempstr[30];
  if (bpm <= 0)
    sprintf(tempstr, "T: External");
  else
    sprintf(tempstr, "T:%4dbpm", bpm);
  tft.setCursor(x + 4, y - 2);
  tft.print(tempstr);  
#if MY_USE_FB
  tft.updateScreen();
#endif  
}

void showTuning(tuningCfg *tuning)
{ 
  tft.setFont(Arial_12_Bold);
  tft.setTextColor(LCD_BLACK);
  int16_t x = 2;
  int16_t y = 5 * ROW_HEIGHT + 16;
  
  tft.fillRect(x - 2, y - 4, 105, 2 * ROW_HEIGHT + 6, LCD_GREEN);

  char *qstr = (char *)malloc(30);
  qstr = strcpy(qstr, quantisationStr(tuning->scale, true));
  char *part;
  part = strtok(qstr," ");
  if (part)
  {
    tft.setCursor(x, y);
    tft.print(part);
    part = strtok(NULL, " ");
  }
  y += ROW_HEIGHT + 2;
  if (part)
  {
    tft.setCursor(x, y);
    tft.print(part);
  }  
  if (tuning->transpose)
  {
    tft.setCursor(x + 60, y);
    sprintf(tempstr, "%+2d", tuning->transpose);
    tft.print(tempstr);
  }
  free(qstr);
#if MY_USE_FB
  tft.updateScreen();
#endif  
}
  
void showRhythm(int active, rhythmCfg *rhythm, bool shift)
{ 
  tft.setFont(Arial_11_Bold);
  tft.setTextColor(LCD_BLACK);
  
  int16_t x = 105;
  int16_t dx = 18;
  int16_t y = 4 * ROW_HEIGHT + 10;
  int16_t dy = ROW_HEIGHT + 2;
  
  tft.fillRect(x, y, 3 * dx + 1, 3 * dy, LCD_ORANGE);

  if (active == 9)
  {
    if (shift)
    {
      tft.drawRect(x + dx, y, dx + 1, 3 * dy, LCD_CYAN);
      tft.drawRect(x + dx + 1, y + 1, dx - 1, 3 * dy - 2, LCD_CYAN);
    }
    else
    {
      for (int i = 0; i < 3; ++i)
      {
        tft.fillRect(x + dx + 1, y + i * dy, dx - 1, dy - 1, LCD_CYAN);
      }
    }
  }
  else
  {
    int r = active / 3;
    int c = active % 3;
    if (shift)
    {
      tft.drawRect(x + c * dx, y + r * dy, dx + 1, dy, LCD_CYAN);
      tft.drawRect(x + c * dx + 1, y + r * dy + 1, dx - 1, dy - 2, LCD_CYAN);
    }
    else
    {
      tft.fillRect(x + c * dx + 1, y + r * dy, dx - 1, dy - 1, LCD_CYAN);
    }
  }
  
  x += 2;
  y += 4;
  tft.setCursor(x, y);
  sprintf(tempstr, "%2d", rhythm->r1[0]);
  tft.print(tempstr);
  tft.setCursor(x + dx, y);
  sprintf(tempstr, "%2d", rhythm->r1[1]);
  tft.print(tempstr);
  tft.setCursor(x + 2 * dx, y);
  sprintf(tempstr, "%2d", rhythm->r1[2]);
  tft.print(tempstr);
  
  y += dy;
  tft.setCursor(x, y);
  sprintf(tempstr, "%2d", rhythm->r2[0]);
  tft.print(tempstr);
  tft.setCursor(x + dx, y);
  sprintf(tempstr, "%2d", rhythm->r2[1]);
  tft.print(tempstr);
  tft.setCursor(x + 2 * dx, y);
  sprintf(tempstr, "%2d", rhythm->r2[2]);
  tft.print(tempstr);

  y += dy;
  tft.setCursor(x, y);
  sprintf(tempstr, "%2d", rhythm->r3[0]);
  tft.print(tempstr);
  tft.setCursor(x + dx, y);
  sprintf(tempstr, "%2d", rhythm->r3[1]);
  tft.print(tempstr);
  tft.setCursor(x + 2 * dx, y);
  sprintf(tempstr, "%2d", rhythm->r3[2]);
  tft.print(tempstr);
  
#if MY_USE_FB
  tft.updateScreen();
#endif  
}

void showNextBeats(int *next, int n)
{
  int i, j;
  int x0 = 2;
  int dx = 10;
  int x1 = x0 + n * dx;
  int y0 = 5 * ROW_HEIGHT + 14;
  int y1 = y0 + 2 * ROW_HEIGHT;
  int dy = round((y1 - y0) / 3.0); 
  int x, y;
  
  tft.fillRect(x0, y0, x1 - x0, y1 - y0, LCD_WHITE);
  
  for (i = 0; i < 3; ++i)
  {
    x = x0;
    y = round(y0 + i * dy);
    tft.drawRect(x, y, x1 - x0, dy, LCD_BLACK);
    bool lastwasX = false;
    for (j = 0; j < n; ++j)
    {
      x = x0 + j * dx;
      int val = next[n * i + j];
      if (val == 2)
        tft.fillRect(x, y + 1, dx, dy - 2, LCD_BLUE);
      else if (val == 1)
        tft.fillRect(x + (lastwasX ? 1 : 0), y + 1, dx - (lastwasX ? 1 : 0), dy - 2, LCD_RED);
      else if (val == -1)
        tft.drawRect(x + 1, y + 2, dx - 2, dy - 4, LCD_BLUE);
      lastwasX = val == 1;
    }
  }
}

void showRhythmGraph(Polyrhythm *poly, int activemeter)
{
  int i, j, n;
  int x0 = 0;
  int x1 = LCD_WIDTH;
  int y0 = ROW_HEIGHT;
  int y1 = 3 * ROW_HEIGHT + 2;
  int x, y;
  
  tft.fillRect(x0, y0 + 2, x1 - x0, y1 - y0, LCD_WHITE);
  
  x0 += 2;
  y0 += 2;

  int maxBeat = 0;
  for (i = 0; i < 3; ++i)
    if (poly->getPattern(i).length > maxBeat)
      maxBeat = poly->getPattern(i).length;
  
  float dx = (x1 - x0) / maxBeat;
  float dy = (y1 - y0) / 3.0; 
  
  for (i = 0; i < 3; ++i)
  {
    x = x0;
    y = round(y0 + i * dy);
    char *rs = poly->getRhythmString(i);
    rs += 3;
    n = strlen(rs);
    if (activemeter == 9 || activemeter / 3 == i)
      tft.fillRect(x, y, round(n * dx), round(dy), LCD_CYAN);
    tft.drawRect(x, y, round(n * dx), round(dy), LCD_BLACK);
    bool lastwasX = false;
    for (j = 0; j < n; ++j)
    {
      x = x0 + round(j * dx);
      if (rs[j] == 'X')
        tft.fillRect(x + (lastwasX ? 1 : 0), y + 1, round(dx) - (lastwasX ? 1 : 0), round(dy) - 2, LCD_RED);
//      else
//        tft.drawRect(x, y, round(dx), round(dy), LCD_GREY);
      lastwasX = rs[j] == 'X';
    }
  }
}
