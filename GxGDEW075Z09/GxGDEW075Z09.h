/************************************************************************************
    class GxGDEW075Z09 : Display class example for GDEW075Z09 e-Paper from Dalian Good Display Co., Ltd.: www.good-display.com

    based on Demo Example from Good Display, now available on http://www.good-display.com/download_list/downloadcategoryid=34&isMode=false.html

    Author : J-M Zingg

    Contributor : Noobidoo

    Version : 2.0

    Support: minimal, provided as example only, as is, no claim to be fit for serious use

    connection to the e-Paper display is through DESTM32-S2 connection board, available from Good Display

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
#ifndef _GxGDEW075Z09_H_
#define _GxGDEW075Z09_H_

#include "../GxEPD.h"

#define GxGDEW075Z09_WIDTH 640
#define GxGDEW075Z09_HEIGHT 384

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
#define GxGDEW075Z09_BUFFER_SIZE GxGDEW075Z09_WIDTH * GxGDEW075Z09_HEIGHT / 8
#define GxGDEW075Z09_OUTBUFFER_SIZE GxGDEW075Z09_WIDTH * GxGDEW075Z09_HEIGHT / 4
#else
#define GxGDEW075Z09_BUFFER_SIZE GxGDEW075Z09_WIDTH * GxGDEW075Z09_HEIGHT / 4
#define GxGDEW075Z09_OUTBUFFER_SIZE GxGDEW075Z09_BUFFER_SIZE
#endif
class GxGDEW075Z09 : public GxEPD
{
  public:
#if defined(ESP8266)
    GxGDEW075Z09(GxIO& io, uint8_t rst = D4, uint8_t busy = D2);
#else
    GxGDEW075Z09(GxIO& io, uint8_t rst = 9, uint8_t busy = 7);
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
    void eraseDisplay(bool using_partial_update){drawBitmap(0,0);};
    // these methods are not implemented
    // partial update
    // note: on this display, partial update to display RAM works, but refresh is full screen
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true){};
    // paged drawing, for limited RAM, drawCallback() is called GxGDEW075T8_PAGES times
    // each call of drawCallback() should draw the same
    void drawPaged(void (*drawCallback)(void)){};
    void drawCornerTest(uint8_t em = 0x01){};
  private:
    void _waitWhileBusy(const char* comment=0);
    void _wakeUp(bool partial);
    void _sleep();
  private:
    unsigned char _buffer[GxGDEW075Z09_BUFFER_SIZE];
    GxIO& IO;
    uint8_t _rst;
    uint8_t _busy;
};

#define GxEPD_Class GxGDEW075Z09

#define GxEPD_WIDTH GxGDEW075Z09_WIDTH
#define GxEPD_HEIGHT GxGDEW075Z09_HEIGHT
#define GxEPD_BitmapExamples <GxGDEW075Z09/BitmapExamples.h>
#define GxEPD_BitmapExamplesQ "GxGDEW075Z09/BitmapExamples.h"

#endif
