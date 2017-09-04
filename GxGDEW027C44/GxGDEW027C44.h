/************************************************************************************
   class GxGDEW027C44 : Display class example for GDEW027C44 e-Paper from Dalian Good Display Co., Ltd.: www.good-display.com

   based on Demo Example from Good Display, now available on http://www.good-display.com/download_list/downloadcategoryid=34&isMode=false.html

   Author : J-M Zingg

   modified by :

   Version : 2.1

   Support: limited, provided as example, no claim to be fit for serious use

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

// mapping from DESTM32-S1 evaluation board to Wemos D1 mini

// D10 : MOSI -> D7
// D8  : CS   -> D8
// E14 : RST  -> D4
// E12 : nc   -> nc

// D9  : CLK  -> D5 SCK
// E15 : DC   -> D3
// E13 : BUSY -> D2
// E11 : BS   -> GND

// mapping from Waveshare 2.9inch e-Paper to Wemos D1 mini
// BUSY -> D2, RST -> D4, DC -> D3, CS -> D8, CLK -> D5, DIN -> D7, GND -> GND, 3.3V -> 3.3V

// mapping example for AVR, UNO, NANO etc.
// BUSY -> 7, RST -> 9, DC -> 8, C S-> 10, CLK -> 13, DIN -> 11

#define GxGDEW027C44_BUFFER_SIZE (uint32_t(GxGDEW027C44_WIDTH) * uint32_t(GxGDEW027C44_HEIGHT) / 8)

// divisor for AVR, should be factor of GxGDEW027C44_HEIGHT
#define GxGDEW027C44_PAGES 4

#define GxGDEW027C44_PAGE_HEIGHT (GxGDEW027C44_HEIGHT / GxGDEW027C44_PAGES)
#define GxGDEW027C44_PAGE_SIZE (GxGDEW027C44_BUFFER_SIZE / GxGDEW027C44_PAGES)

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
    // monochrome bitmap to buffer, may be cropped, drawPixel() used, update needed, signature like Adafruit_GFX
    // still needed here because of a signature matching issue of the ESP compiler (matches only if declared in subclass)
    void  drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);
    // to buffer, may be cropped, drawPixel() used, update needed, different signature, mode default for example bitmaps
    void  drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode = bm_normal);
    // to full screen, filled with white if size is less, no update needed
    void drawPicture(const uint8_t* black_bitmap, const uint8_t* red_bitmap, uint32_t size);
    // to full screen, filled with white if size is less, no update needed
    void drawBitmap(const uint8_t *bitmap, uint32_t size)
    {
      drawBitmap(bitmap, size, bm_normal); // default for example bitmaps
    }
    void drawBitmap(const uint8_t *bitmap, uint32_t size, int16_t mode); // mode support not implemented
    void eraseDisplay(bool using_partial_update = false); // parameter ignored
    // partial update, not implemented
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true){};
    // paged drawing, for limited RAM, drawCallback() is called GxGDEH029A1_PAGES times
    // each call of drawCallback() should draw the same
    void drawPaged(void (*drawCallback)(void));
    void drawCornerTest(uint8_t em = 0x01);
  private:
    void _writeData(uint8_t data);
    void _writeCommand(uint8_t command);
    void _writeLUT();
    void _wakeUp();
    void _sleep();
    void _waitWhileBusy(const char* comment=0);
  private:
#if defined(__AVR)
    uint8_t _black_buffer[GxGDEW027C44_PAGE_SIZE];
    uint8_t _red_buffer[GxGDEW027C44_PAGE_SIZE];
#else
    uint8_t _black_buffer[GxGDEW027C44_BUFFER_SIZE];
    uint8_t _red_buffer[GxGDEW027C44_BUFFER_SIZE];
#endif
    GxIO& IO;
    int16_t _current_page;
    uint8_t _rst;
    uint8_t _busy;
};

#define GxEPD_Class GxGDEW027C44

#define GxEPD_WIDTH GxGDEW027C44_WIDTH
#define GxEPD_HEIGHT GxGDEW027C44_HEIGHT
#define GxEPD_BitmapExamples <GxGDEW027C44/BitmapExamples.h>
#define GxEPD_BitmapExamplesQ "GxGDEW027C44/BitmapExamples.h"

#endif

