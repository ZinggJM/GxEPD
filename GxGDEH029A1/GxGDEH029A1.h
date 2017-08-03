/************************************************************************************
   class GxGDEH029A1 : Display class example for GDEH029A1 e-Paper from GoodDisplay.com

   based on Demo Example from GoodDisplay.com, avalable with any order for such a display, no copyright notice.

   Author : J-M Zingg

   modified by :

   Version : 2.0

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
#ifndef _GxGDEH029A1_H_
#define _GxGDEH029A1_H_

#include "../GxEPD.h"

// the physical number of pixels (for controller parameter)
#define GxGDEH029A1_X_PIXELS 128
#define GxGDEH029A1_Y_PIXELS 296

// the logical width and height of the display
#define GxGDEH029A1_WIDTH GxGDEH029A1_X_PIXELS
#define GxGDEH029A1_HEIGHT GxGDEH029A1_Y_PIXELS

#define GxGDEH029A1_BUFFER_SIZE (uint32_t(GxGDEH029A1_WIDTH) * uint32_t(GxGDEH029A1_HEIGHT) / 8)

// divisor for AVR, should be factor of GxGDEP015OC1_HEIGHT
#define GxGDEH029A1_PAGES 4

#define GxGDEH029A1_PAGE_HEIGHT (GxGDEH029A1_HEIGHT / GxGDEH029A1_PAGES)
#define GxGDEH029A1_PAGE_SIZE (GxGDEH029A1_BUFFER_SIZE / GxGDEH029A1_PAGES)

// mapping from DESTM32-S1 evaluation board to Wemos D1 mini

// D10 : MOSI -> D7
// D8  : CS   -> D8
// E14 : RST  -> D4
// E12 : nc   -> nc

// D9  : CLK  -> D5
// E15 : DC   -> D3
// E13 : BUSY -> D2
// E11 : BS   -> GND

class GxGDEH029A1 : public GxEPD
{
  public:
#if defined(ESP8266)
    GxGDEH029A1(GxIO& io, uint8_t rst = D4, uint8_t busy = D2);
#else
    GxGDEH029A1(GxIO& io, uint8_t rst = 9, uint8_t busy = 7);
#endif
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void init(void);
    void fillScreen(uint16_t color); // 0x0 black, >0x0 white, to buffer
    void update(void);
    // to buffer, may be cropped, drawPixel() used, update needed, old signature kept
    void  drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);
    // to buffer, may be cropped, drawPixel() used, update needed, new signature, mirror default for example bitmaps
    void  drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, bool mirror = true);
    // to full screen, filled with white if size is less, no update needed
    void drawBitmap(const uint8_t *bitmap, uint32_t size);
    void drawBitmap(const uint8_t *bitmap, uint32_t size, bool using_partial_update);
    void eraseDisplay(bool using_partial_update = false);
    // partial update
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    // paged drawing, for limited RAM, drawCallback() is called GxGDEP015OC1_PAGES times
    // each call of drawCallback() should draw the same
    void drawPaged(void (*drawCallback)(void));
  private:
    void _writeData(uint8_t data);
    void _writeCommand(uint8_t command);
    void _writeCommandData(const uint8_t* pCommandData, uint8_t datalen);
    void _SetRamPointer(uint8_t addrX, uint8_t addrY, uint8_t addrY1);
    void _SetRamArea(uint8_t Xstart, uint8_t Xend, uint8_t Ystart, uint8_t Ystart1, uint8_t Yend, uint8_t Yend1);
    void _PowerOn(void);
    void _PowerOff(void);
    void _waitWhileBusy(const char* comment=0);
    void _InitDisplay(void);
    void _Init_Full(void);
    void _Init_Part(void);
    void _Update_Full(void);
    void _Update_Part(void);
    void _drawCurrentPage();
  protected:
#if defined(__AVR)
    uint8_t _buffer[GxGDEH029A1_PAGE_SIZE];
#else
    uint8_t _buffer[GxGDEH029A1_BUFFER_SIZE];
#endif
  private:
    GxIO& IO;
    int16_t _current_page;
    bool _using_partial_mode;
    uint8_t _rst;
    uint8_t _busy;
};

#endif

#define GxEPD_Class GxGDEH029A1

#define GxEPD_WIDTH GxGDEH029A1_WIDTH
#define GxEPD_HEIGHT GxGDEH029A1_HEIGHT
#define GxEPD_BitmapExamples <GxGDEH029A1/BitmapExamples.h>
#define GxEPD_BitmapExamplesQ "GxGDEH029A1/BitmapExamples.h"

