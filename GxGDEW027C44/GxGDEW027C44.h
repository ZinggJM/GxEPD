/************************************************************************************
   class GxGDEW027C44 : Display class example for GDEW027C44 e-Paper from GoodDisplay.com

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

   note: for correct red color jumper J3 must be set on 0.47 side (towards FCP connector)
   
*/
#ifndef _GxGDEW027C44_H_
#define _GxGDEW027C44_H_

#include "../GxEPD.h"

#define GxGDEW027C44_WIDTH 176
#define GxGDEW027C44_HEIGHT 264

#define GxGDEW027C44_BUFFER_SIZE GxGDEW027C44_WIDTH * GxGDEW027C44_HEIGHT / 8

// mapping from DESTM32-S1 evaluation board to Wemos D1 mini

// D10 : MOSI -> D7
// D8  : CS   -> D8
// E14 : RST  -> D4
// E12 : nc   -> nc

// D9  : CLK  -> D5 SCK
// E15 : DC   -> D3
// E13 : BUSY -> D2
// E11 : BS   -> GND

class GxGDEW027C44 : public GxEPD
{
  public:
#if defined(ESP8266)
    GxGDEW027C44(GxIO& io, uint8_t rst = D4, uint8_t busy = D2);
#else
    GxGDEW027C44(GxIO& io, uint8_t rst = 9, uint8_t busy = 7);
#endif
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void init(void);
    void fillScreen(uint16_t color); // 0x0 black, >0x0 white, to buffer
    void update(void);
    // to full screen, filled with white if size is less, no update needed
    void drawPicture(const uint8_t* black_bitmap, const uint8_t* red_bitmap, uint32_t size);
    // to full screen, filled with white if size is less, no update needed
    void drawBitmap(const uint8_t* bitmap, uint32_t size);
    // to buffer, may be cropped, drawPixel() used, update needed
    void  drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);
  private:
    void _writeData(uint8_t data);
    void _writeCommand(uint8_t command);
    void _writeLUT();
    void _wakeUp();
    void _sleep();
    void _waitWhileBusy(const char* comment=0);
  private:
    uint8_t _black_buffer[GxGDEW027C44_BUFFER_SIZE];
    uint8_t _red_buffer[GxGDEW027C44_BUFFER_SIZE];
    GxIO& IO;
    uint8_t _rst;
    uint8_t _busy;
};

#define GxEPD_Class GxGDEW027C44

#define GxEPD_WIDTH GxGDEW027C44_WIDTH
#define GxEPD_HEIGHT GxGDEW027C44_HEIGHT
#define GxEPD_BitmapExamples <GxGDEW027C44/BitmapExamples.h>
#define GxEPD_BitmapExamplesQ "GxGDEW027C44/BitmapExamples.h"

#endif

