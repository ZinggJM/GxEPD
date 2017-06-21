/************************************************************************************
   class GxGDEW075T8 : Display class example for GDEW075T8 e-Paper from GoodDisplay.com

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
#ifndef _GxGDEW075T8_H_
#define _GxGDEW075T8_H_

#include "../GxEPD.h"

#define GxGDEW075T8_WIDTH 640
#define GxGDEW075T8_HEIGHT 384

// my mapping from DESTM32-S1 evaluation board to Wemos D1 mini

// D10 : MOSI -> D7
// D8  : CS   -> D8
// E14 : RST  -> D4
// E12 : nc  -> nc

// D9  : CLK  -> D5 SCK
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

#define GxGDEW075T8_BUFFER_SIZE GxGDEW075T8_WIDTH * GxGDEW075T8_HEIGHT / 8

class GxGDEW075T8 : public GxEPD
{
  public:
    GxGDEW075T8(GxIO& io, uint8_t rst = RST, uint8_t busy = BSY);
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void init(void);
    void fillScreen(uint16_t color); // 0x0 black, >0x0 white, to buffer
    void update(void);
    // to full screen, filled with white if size is less, no update needed
    void drawBitmap(const uint8_t *bitmap, uint32_t size);
    // to buffer, may be cropped, drawPixel() used, update needed
    void  drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);

  private:
    void _waitWhileBusy(const char* comment=0);
    void _wakeUp(bool partial);
    void _sleep();

  private:
    unsigned char _buffer[GxGDEW075T8_BUFFER_SIZE];
    GxIO& IO;
    uint8_t _rst;
    uint8_t _busy;
};

#define GxEPD_Class GxGDEW075T8

#define GxEPD_WIDTH GxGDEW075T8_WIDTH
#define GxEPD_HEIGHT GxGDEW075T8_HEIGHT
#define GxEPD_BitmapExamples <GxGDEW075T8/BitmapExamples.h>
#define GxEPD_BitmapExamplesQ "GxGDEW075T8/BitmapExamples.h"

#endif

