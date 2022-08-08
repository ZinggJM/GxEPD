// class GxGDEY027T91 : Display class for GDEY027T91 e-Paper from Dalian Good Display Co., Ltd.: https://www.good-display.com
//
// based on Demo Example from Good Display, available here: http://www.e-paper-display.com/download_detail/downloadsId=806.html
// Panel: GDEY027T91 : https://www.good-display.com/product/432.html
// Controller : SSD1680 : https://www.good-display.com/companyfile/101.html
//
// Author : J-M Zingg
//
// Version : see library.properties
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// Library: https://github.com/ZinggJM/GxEPD

#ifndef _GxGDEY027T91_H_
#define _GxGDEY027T91_H_

#include <Arduino.h>
#include "../GxEPD.h"

// the physical number of pixels (for controller parameter)
#define GxGDEY027T91_X_PIXELS 176
#define GxGDEY027T91_Y_PIXELS 264

#define GxGDEY027T91_WIDTH GxGDEY027T91_X_PIXELS
#define GxGDEY027T91_HEIGHT GxGDEY027T91_Y_PIXELS

#define GxGDEY027T91_BUFFER_SIZE (uint32_t(GxGDEY027T91_WIDTH) * uint32_t(GxGDEY027T91_HEIGHT) / 8)

// divisor for AVR, should be factor of GxGDEY027T91_HEIGHT
#define GxGDEY027T91_PAGES 8

#define GxGDEY027T91_PAGE_HEIGHT (GxGDEY027T91_HEIGHT / GxGDEY027T91_PAGES)
#define GxGDEY027T91_PAGE_SIZE (GxGDEY027T91_BUFFER_SIZE / GxGDEY027T91_PAGES)

class GxGDEY027T91 : public GxEPD
{
  public:
#if defined(ESP8266)
    GxGDEY027T91(GxIO& io, int8_t rst = 2, int8_t busy = 4);
#else
    GxGDEY027T91(GxIO& io, int8_t rst = 9, int8_t busy = 7);
#endif
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void init(uint32_t serial_diag_bitrate = 0); // = 0 : disabled
    void fillScreen(uint16_t color); // 0x0 black, >0x0 white, to buffer
    void update(void);
    // to buffer, may be cropped, drawPixel() used, update needed
    void  drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode = bm_normal);
    // to full screen, filled with white if size is less, no update needed
    void drawBitmap(const uint8_t *bitmap, uint32_t size, int16_t mode = bm_normal); // only bm_normal, bm_invert, bm_partial_update modes implemented
    void eraseDisplay(bool using_partial_update = false);
    // partial update of rectangle from buffer to screen, does not power off
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true);
    // partial update of rectangle at (xs,ys) from buffer to screen at (xd,yd), does not power off
    void updateToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h, bool using_rotation = true);
    // terminate cleanly updateWindow or updateToWindow before removing power or long delays
    void powerDown();
    // paged drawing, for limited RAM, drawCallback() is called GxGDEY027T91_PAGES times
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
    void _writeToWindow(uint8_t command, uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h);
    void _writeData(uint8_t data);
    void _writeCommand(uint8_t command);
    void _writeCommandData(const uint8_t* pCommandData, uint8_t datalen);
    void _SetRamPointer(uint8_t addrX, uint8_t addrY, uint8_t addrY1);
    void _SetRamArea(uint8_t Xstart, uint8_t Xend, uint8_t Ystart, uint8_t Ystart1, uint8_t Yend, uint8_t Yend1);
    void _PowerOn(void);
    void _PowerOff(void);
    void _waitWhileBusy(const char* comment, uint16_t busy_time);
    void _setRamDataEntryMode(uint8_t em);
    void _InitDisplay(uint8_t em);
    void _Init_Full(uint8_t em);
    void _Init_Part(uint8_t em);
    void _Update_Full(void);
    void _Update_Part(void);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);
  protected:
#if defined(__AVR)
    uint8_t _buffer[GxGDEY027T91_PAGE_SIZE];
#else
    uint8_t _buffer[GxGDEY027T91_BUFFER_SIZE];
#endif
  private:
    GxIO& IO;
    int16_t _current_page;
    bool _using_partial_mode;
    bool _diag_enabled;
    bool _power_is_on;
    int8_t _rst;
    int8_t _busy;
    static const uint16_t power_on_time = 100; // ms, e.g. 92440us
    static const uint16_t power_off_time = 150; // ms, e.g. 142212us
    static const uint16_t full_refresh_time = 2200; // ms, e.g. 2097292us
    static const uint16_t partial_refresh_time = 500; // ms, e.g. 460124us
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
#define GxEPD_Class GxGDEY027T91
#define GxEPD_WIDTH GxGDEY027T91_WIDTH
#define GxEPD_HEIGHT GxGDEY027T91_HEIGHT
#define GxEPD_BitmapExamples <GxGDEY027T91/BitmapExamples.h>
#define GxEPD_BitmapExamplesQ "GxGDEY027T91/BitmapExamples.h"
#endif

#endif
