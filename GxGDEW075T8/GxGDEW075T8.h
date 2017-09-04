/************************************************************************************
   class GxGDEW075T8 : Display class example for GDEW075T8 e-Paper from Dalian Good Display Co., Ltd.: www.good-display.com

   based on Demo Example from Good Display, now available on http://www.good-display.com/download_list/downloadcategoryid=34&isMode=false.html

   Author : J-M Zingg

   modified by :

   Version : 2.0

   Support: limited, provided as example, no claim to be fit for serious use

   connection to the e-Paper display is through DESTM32-S2 connection board, available from Good Display

   DESTM32-S2 pinout (top, component side view):
       |-------------------------------------------------
       |  VCC  |o o| VCC 5V, not needed
       |  GND  |o o| GND
       |  3.3  |o o| 3.3V
       |  nc   |o o| nc
       |  nc   |o o| nc
       |  nc   |o o| nc
       |  MOSI |o o| CLK=SCK
       | SS=DC |o o| D/C=RS    // Slave Select = Device Connect |o o| Data/Command = Register Select
       |  RST  |o o| BUSY
       |  nc   |o o| BS, connect to GND
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

#define GxGDEW075T8_BUFFER_SIZE (uint32_t(GxGDEW075T8_WIDTH) * uint32_t(GxGDEW075T8_HEIGHT) / 8)

// divisor for AVR, should be factor of GxGDEW075T8_HEIGHT
#define GxGDEW075T8_PAGES 24

#define GxGDEW075T8_PAGE_HEIGHT (GxGDEW075T8_HEIGHT / GxGDEW075T8_PAGES)
#define GxGDEW075T8_PAGE_SIZE (GxGDEW075T8_BUFFER_SIZE / GxGDEW075T8_PAGES)

class GxGDEW075T8 : public GxEPD
{
  public:
#if defined(ESP8266)
    GxGDEW075T8(GxIO& io, uint8_t rst = D4, uint8_t busy = D2);
#else
    GxGDEW075T8(GxIO& io, uint8_t rst = 9, uint8_t busy = 7);
#endif
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void init(void);
    void fillScreen(uint16_t color); // 0x0 black, >0x0 white, to buffer
    void update(void);
    // monochrome bitmap to buffer, may be cropped, drawPixel() used, update needed, signature like Adafruit_GFX
    // still needed here because of a signature matching issue of the ESP compiler (matches only if declared in subclass)
    void  drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);
    // to buffer, may be cropped, drawPixel() used, update needed, different signature, mode default for example bitmaps
    void  drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode = bm_normal);
    // to full screen, filled with white if size is less, no update needed
    void drawBitmap(const uint8_t *bitmap, uint32_t size);
    void drawBitmap(const uint8_t *bitmap, uint32_t size, bool using_partial_update);
    void eraseDisplay(bool using_partial_update = false);
    // partial update
    // note: on this display, partial update to display RAM works, but refresh is full screen
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true);
    // paged drawing, for limited RAM, drawCallback() is called GxGDEW075T8_PAGES times
    // each call of drawCallback() should draw the same
    void drawPaged(void (*drawCallback)(void));
    void drawCornerTest(uint8_t em = 0x01);
  private:
    void _setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye);
    void _wakeUp();
    void _sleep();
    void _waitWhileBusy(const char* comment = 0);
    void _send8pixel(uint8_t data);
  private:
#if defined(__AVR)
    uint8_t _buffer[GxGDEW075T8_PAGE_SIZE];
#else
    uint8_t _buffer[GxGDEW075T8_BUFFER_SIZE];
#endif
    GxIO& IO;
    int16_t _current_page;
    bool _using_partial_mode;
    uint8_t _rst;
    uint8_t _busy;
};

#define GxEPD_Class GxGDEW075T8

#define GxEPD_WIDTH GxGDEW075T8_WIDTH
#define GxEPD_HEIGHT GxGDEW075T8_HEIGHT
#define GxEPD_BitmapExamples <GxGDEW075T8/BitmapExamples.h>
#define GxEPD_BitmapExamplesQ "GxGDEW075T8/BitmapExamples.h"

#endif

