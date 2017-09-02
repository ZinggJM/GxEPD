/************************************************************************************
   class GxGDEW042T2 : Display class example for GDEW042T2 e-Paper from Dalian Good Display Co., Ltd.: www.good-display.com

   based on Demo Example from Good Display, now available on http://www.good-display.com/download_list/downloadcategoryid=34&isMode=false.html

   Author : J-M Zingg

   modified by :

   Version : 2.1

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

// mapping from Waveshare 4.2inch e-Paper to Wemos D1 mini
// BUSY -> D2, RST -> D4, DC -> D3, CS -> D8, CLK -> D5, DIN -> D7, GND -> GND, 3.3V -> 3.3V

// mapping example for AVR, UNO, NANO etc.
// BUSY -> 7, RST -> 9, DC -> 8, CS -> 10, CLK -> 13, DIN -> 11

#define GxGDEW042T2_BUFFER_SIZE (uint32_t(GxGDEW042T2_WIDTH) * uint32_t(GxGDEW042T2_HEIGHT) / 8)

// divisor for AVR, should be factor of GxGDEW042T2_HEIGHT
#define GxGDEW042T2_PAGES 20

#define GxGDEW042T2_PAGE_HEIGHT (GxGDEW042T2_HEIGHT / GxGDEW042T2_PAGES)
#define GxGDEW042T2_PAGE_SIZE (GxGDEW042T2_BUFFER_SIZE / GxGDEW042T2_PAGES)


class GxGDEW042T2 : public GxEPD
{
  public:
#if defined(ESP8266)
    GxGDEW042T2(GxIO& io, uint8_t rst = D4, uint8_t busy = D2);
#else
    GxGDEW042T2(GxIO& io, uint8_t rst = 9, uint8_t busy = 7);
#endif
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void init(void);
    void fillScreen(uint16_t color); // 0x0 black, >0x0 white, to buffer
    void update(void);
    // monochrome bitmap to buffer, may be cropped, drawPixel() used, update needed, signature like Adafruit_GFX
    void  drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);
    // to buffer, may be cropped, drawPixel() used, update needed, different signature, mode default for example bitmaps
    void  drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode = bm_invert);
    // to full screen, filled with white if size is less, no update needed
    void drawBitmap(const uint8_t *bitmap, uint32_t size)
    {
      drawBitmap(bitmap, size, bm_normal); // default for example bitmaps
    }
    void drawBitmap(const uint8_t *bitmap, uint32_t size, int16_t mode);
    void eraseDisplay(bool using_partial_update = false);
    // partial update
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true);
    // paged drawing, for limited RAM, drawCallback() is called GxGDEW042T2_PAGES times
    // each call of drawCallback() should draw the same
    void drawPaged(void (*drawCallback)(void));
    void drawCornerTest(uint8_t em = 0);
    // private methods kept available public
    void drawBitmapEM(const uint8_t *bitmap, uint32_t size, uint8_t em); // ram data entry mode
    void drawBitmapPU(const uint8_t *bitmap, uint32_t size, uint8_t em); // partial update mode
  private:
    void _setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye);
    void _wakeUp();
    void _sleep(void);
    void _waitWhileBusy(const char* comment = 0);
  private:
#if defined(__AVR)
    uint8_t _buffer[GxGDEW042T2_PAGE_SIZE];
#else
    uint8_t _buffer[GxGDEW042T2_BUFFER_SIZE];
#endif
    GxIO& IO;
    int16_t _current_page;
    bool _using_partial_mode;
    uint8_t _rst;
    uint8_t _busy;
};

#define GxEPD_Class GxGDEW042T2

#define GxEPD_WIDTH GxGDEW042T2_WIDTH
#define GxEPD_HEIGHT GxGDEW042T2_HEIGHT
#define GxEPD_BitmapExamples <GxGDEW042T2/BitmapExamples.h>
#define GxEPD_BitmapExamplesQ "GxGDEW042T2/BitmapExamples.h"

#endif

