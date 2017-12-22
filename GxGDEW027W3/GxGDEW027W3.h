/************************************************************************************
   class GxGDEW027W3 : Display class example for GDEW027W3 e-Paper from Dalian Good Display Co., Ltd.: www.good-display.com

   based on Demo Example from Good Display, available here: http://www.good-display.com/download_detail/downloadsId=520.html

   Author : J-M Zingg

   Version : 2.3

   Support: limited, provided as example, no claim to be fit for serious use

   Controller: IL91874 : http://www.good-display.com/download_detail/downloadsId=539.html

   connection to the e-Paper display is through DESTM32-S2 connection board, available from Good Display

   DESTM32-S2 pinout (top, component side view):
         |-------------------------------------------------
         |  VCC  |o o| VCC 5V  not needed
         |  GND  |o o| GND
         |  3.3  |o o| 3.3     3.3V
         |  nc   |o o| nc
         |  nc   |o o| nc
         |  nc   |o o| nc
   MOSI  |  DIN  |o o| CLK     SCK
   SS    |  CS   |o o| DC      e.g. D3
   D4    |  RST  |o o| BUSY    e.g. D2
         |  nc   |o o| BS      GND
         |-------------------------------------------------
*/
#ifndef _GxGDEW027W3_H_
#define _GxGDEW027W3_H_

#include "../GxEPD.h"

#define GxGDEW027W3_WIDTH 176
#define GxGDEW027W3_HEIGHT 264

#define GxGDEW027W3_BUFFER_SIZE (uint32_t(GxGDEW027W3_WIDTH) * uint32_t(GxGDEW027W3_HEIGHT) / 8)

// divisor for AVR, should be factor of GxGDEW027W3_HEIGHT
#define GxGDEW027W3_PAGES 6

#define GxGDEW027W3_PAGE_HEIGHT (GxGDEW027W3_HEIGHT / GxGDEW027W3_PAGES)
#define GxGDEW027W3_PAGE_SIZE (GxGDEW027W3_BUFFER_SIZE / GxGDEW027W3_PAGES)

// mapping suggestion from Waveshare 2.7inch e-Paper to Wemos D1 mini
// BUSY -> D2, RST -> D4, DC -> D3, CS -> D8, CLK -> D5, DIN -> D7, GND -> GND, 3.3V -> 3.3V

// mapping suggestion from Waveshare 2.7inch e-Paper to generic ESP8266
// BUSY -> GPIO4, RST -> GPIO2, DC -> GPIO0, CS -> GPIO15, CLK -> GPIO14, DIN -> GPIO13, GND -> GND, 3.3V -> 3.3V

// mapping suggestion for ESP32, e.g. LOLIN32, see .../variants/.../pins_arduino.h for your board
// NOTE: there are variants with different pins for SPI ! CHECK SPI PINS OF YOUR BOARD
// BUSY -> 4, RST -> 16, DC -> 17, CS -> SS(5), CLK -> SCK(18), DIN -> MOSI(23), GND -> GND, 3.3V -> 3.3V

// mapping suggestion for AVR, UNO, NANO etc.
// BUSY -> 7, RST -> 9, DC -> 8, CS-> 10, CLK -> 13, DIN -> 11

class GxGDEW027W3 : public GxEPD
{
  public:
#if defined(ESP8266)
    //GxGDEW027W3(GxIO& io, int8_t rst = D4, int8_t busy = D2);
    // use pin numbers, other ESP8266 than Wemos may not use Dx names
    GxGDEW027W3(GxIO& io, int8_t rst = 2, int8_t busy = 4);
#else
    GxGDEW027W3(GxIO& io, int8_t rst = 9, int8_t busy = 7);
#endif
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void init(void);
    void fillScreen(uint16_t color); // to buffer
    void update(void);
    // to buffer, may be cropped, drawPixel() used, update needed
    void  drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode = bm_invert);
    // to full screen, filled with white if size is less, no update needed
    void drawBitmap(const uint8_t *bitmap, uint32_t size, int16_t mode = bm_normal); // only bm_normal, bm_invert, bm_partial_update modes implemented
    void eraseDisplay(bool using_partial_update = false);
    // partial update of rectangle from buffer to screen, does not power off
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true);
    // partial update of rectangle at (xs,ys) from buffer to screen at (xd,yd), does not power off
    void updateToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h, bool using_rotation = true);
    // terminate cleanly, not needed as all screen drawing methods of this class do power down
    void powerDown();
    // paged drawing, for limited RAM, drawCallback() is called GxGDEW027W3_PAGES times
    // each call of drawCallback() should draw the same
    void drawPaged(void (*drawCallback)(void));
    void drawPaged(void (*drawCallback)(uint32_t), uint32_t);
    void drawPaged(void (*drawCallback)(const void*), const void*);
    void drawPaged(void (*drawCallback)(const void*, const void*), const void*, const void*);
    // paged drawing to screen rectangle at (x,y) using partial update
    void drawPagedToWindow(void (*drawCallback)(void), uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    void drawPagedToWindow(void (*drawCallback)(uint32_t), uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t);
    void drawPagedToWindow(void (*drawCallback)(const void*), uint16_t x, uint16_t y, uint16_t w, uint16_t h, const void*);
    void drawPagedToWindow(void (*drawCallback)(const void*, const void*), uint16_t x, uint16_t y, uint16_t w, uint16_t h, const void*, const void*);
    void drawCornerTest(uint8_t em = 0x01);
  private:
    template <typename T> static inline void
    swap(T& a, T& b)
    {
      T t = a;
      a = b;
      b = t;
    }
    void _writeToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h);
    void _setPartialRamArea(uint8_t command, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    void _refreshWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    void _writeData(uint8_t data);
    void _writeCommand(uint8_t command);
    void _writeLUT();
    void _wakeUp();
    void _sleep();
    void _waitWhileBusy(const char* comment = 0);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);
  private:
#if defined(__AVR)
    uint8_t _buffer[GxGDEW027W3_PAGE_SIZE];
#else
    uint8_t _buffer[GxGDEW027W3_BUFFER_SIZE];
#endif
    GxIO& IO;
    int16_t _current_page;
    bool _using_partial_mode;
    int8_t _rst;
    int8_t _busy;
    static const uint8_t lut_20_vcomDC[];
    static const uint8_t lut_21_ww[];
    static const uint8_t lut_22_bw[];
    static const uint8_t lut_23_wb[];
    static const uint8_t lut_24_bb[];
#if defined(ESP8266) || defined(ESP32)
  public:
    // the compiler of these packages has a problem with signature matching to base classes
    void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color)
    {
      Adafruit_GFX::drawBitmap(x, y, bitmap, w, h, color);
    };
#endif
};

#ifndef GxEPD_Class
#define GxEPD_Class GxGDEW027W3
#define GxEPD_WIDTH GxGDEW027W3_WIDTH
#define GxEPD_HEIGHT GxGDEW027W3_HEIGHT
#define GxEPD_BitmapExamples <GxGDEW027W3/BitmapExamples.h>
#define GxEPD_BitmapExamplesQ "GxGDEW027W3/BitmapExamples.h"
#endif

#endif

