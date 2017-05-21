/************************************************************************************
   class GxGDEW042T2 : Display class example for GDEW042T2 e-Paper from GoodDisplay.com

   based on Demo Example from GoodDisplay.com, avalable with any order for such a display, no copyright notice.

   Author : J-M Zingg

   modified by :

   Version : 1.1

   Support: minimal, provided as example only, as is, no claim to be fit for serious use

   connection to the e-Paper display is through DESTM32-S2 connection board, available from GoodDisplay

   DESTM32-S2 pinout (top, component side view):
       |-------------------------------------------------
       |  VCC  |o o| VCC 5V
       |  GND  |o o| GND
       |  3.3  |o o| 3.3V
       |  nc   |o o| nc
       |  nc   |o o| nc
       |  nc   |o o| nc
       |  MOSI |o o| CLK
       |  DC   |o o| D/C
       |  RST  |o o| BUSY
       |  nc   |o o| BS
       |-------------------------------------------------
*/
#ifndef _GxGDEW042T2_H_
#define _GxGDEW042T2_H_

#include "../GxEPD.h"

#define GxGDEW042T2_WIDTH 400
#define GxGDEW042T2_HEIGHT 300

// my mapping from DESTM32-S1 evaluation board to Wemos D1 mini

// D10 : MOSI -> D7
// D8  : CS   -> D8
// E14 : RST  -> D4
// E12 : nc?  -> nc?

// D9  : CLK  -> D5
// E15 : DC   -> D3
// E13 : BUSY -> D2
// E11 : BS   -> GND

#if defined(ESP8266)
#define RST D4
#define BSY D2
#else
#define RST 9
#define BSY 7
#endif

#define GxGDEW042T2_BUFFER_SIZE GxGDEW042T2_WIDTH * GxGDEW042T2_HEIGHT / 8

class GxGDEW042T2 : public GxEPD
{
  public:
    GxGDEW042T2(GxIO& io, uint8_t rst = RST, uint8_t busy = BSY);
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void init(void);
    void fillScreen(uint16_t color); // 0x0 black, >0x0 white, to buffer
    void update(void);
    // to full screen, filled with white if size is less, no update needed
    void drawBitmap(const uint8_t *bitmap, uint16_t size);
    // to buffer, may be cropped, drawPixel() used, update needed
    void  drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);
    // GxGDEW042T2 may have 2 channels, commands 0x10 and 0x13, but no grey levels
    void greyTest(); // what do the 2 channels provide ? bw only
  private:
    void _writeLUT();
    void _wakeUp();
    void _sleep(void);
    void _waitWhileBusy(const char* comment = 0);
  private:
    uint8_t _buffer[GxGDEW042T2_BUFFER_SIZE];
    GxIO& IO;
    uint8_t _rst;
    uint8_t _busy;
};

#define GxEPD_Class GxGDEW042T2

#define GxEPD_WIDTH GxGDEW042T2_WIDTH
#define GxEPD_HEIGHT GxGDEW042T2_HEIGHT
#define GxEPD_BitmapExamples <GxGDEW042T2/BitmapExamples.h>
#define GxEPD_BitmapExamplesQ "GxGDEW042T2/BitmapExamples.h"

#endif

