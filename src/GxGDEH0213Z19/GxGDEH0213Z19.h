// class GxGDEH0213Z19 : Display class for GDEH0213Z19 e-Paper from Dalian Good Display Co., Ltd.: www.good-display.com
//
// based on Demo Example from Good Display, available here: http://www.good-display.com/download_detail/downloadsId=515.html
// Panel: GDEH0213Z19 : https://www.good-display.com/product/223.html
// Controller: UC8151D : https://v4.cecdn.yun300.cn/100001_1909185148/UC8151D.pdf
//
// Author : J-M Zingg
//
// Version : see library.properties
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// Library: https://github.com/ZinggJM/GxEPD

#ifndef _GxGDEH0213Z19_H_
#define _GxGDEH0213Z19_H_

#include "../GxEPD.h"

#define GxGDEH0213Z19_WIDTH 104
#define GxGDEH0213Z19_HEIGHT 212

#define GxGDEH0213Z19_BUFFER_SIZE (uint32_t(GxGDEH0213Z19_WIDTH) * uint32_t(GxGDEH0213Z19_HEIGHT) / 8)

// divisor for AVR, should be factor of GxGDEH0213Z19_HEIGHT
#define GxGDEH0213Z19_PAGES 8 // not a factor, workaround needed

#define GxGDEH0213Z19_PAGE_HEIGHT (GxGDEH0213Z19_HEIGHT / (GxGDEH0213Z19_PAGES - 1))
#define GxGDEH0213Z19_PAGE_SIZE (GxGDEH0213Z19_BUFFER_SIZE / (GxGDEH0213Z19_PAGES - 1))

class GxGDEH0213Z19 : public GxEPD
{
  public:
    static const bool usePartialUpdateWindow = false; // needs be false, controller issue
#if defined(ESP8266)
    //GxGDEH0213Z19(GxIO& io, int8_t rst = D4, int8_t busy = D2);
    // use pin numbers, other ESP8266 than Wemos may not use Dx names
    GxGDEH0213Z19(GxIO& io, int8_t rst = 2, int8_t busy = 4);
#else
    GxGDEH0213Z19(GxIO& io, int8_t rst = 9, int8_t busy = 7);
#endif
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void init(uint32_t serial_diag_bitrate = 0); // = 0 : disabled
    void fillScreen(uint16_t color); // to buffer
    void update(void);
    // to buffer, may be cropped, drawPixel() used, update needed
    void  drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode = bm_normal);
    // to full screen, filled with white if size is less, no update needed, black  /white / red, for example bitmaps
    void drawExamplePicture(const uint8_t* black_bitmap, const uint8_t* red_bitmap, uint32_t black_size, uint32_t red_size);
    // to full screen, filled with white if size is less, no update needed, black  /white / red, general version
    void drawPicture(const uint8_t* black_bitmap, const uint8_t* red_bitmap, uint32_t black_size, uint32_t red_size, int16_t mode = bm_normal);
    // to full screen, filled with white if size is less, no update needed
    void drawBitmap(const uint8_t *bitmap, uint32_t size, int16_t mode = bm_normal); // only bm_normal, bm_invert, bm_partial_update modes implemented
    void eraseDisplay(bool using_partial_update = false);
    // partial update of rectangle from buffer to screen, does not power off
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true);
    // partial update of rectangle at (xs,ys) from buffer to screen at (xd,yd), does not power off
    void updateToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h, bool using_rotation = true);
    // terminate cleanly updateWindow or updateToWindow before removing power or long delays
    void powerDown();
    // paged drawing, for limited RAM, drawCallback() is called GxGDEH0213Z19_PAGES times
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
    void _writeToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h, bool using_rotation = true);
    uint16_t _setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye);
    void _writeData(uint8_t data);
    void _writeCommand(uint8_t command);
    void _wakeUp();
    void _sleep();
    void _waitWhileBusy(const char* comment = 0);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);
  private:
#if defined(__AVR)
    uint8_t _black_buffer[GxGDEH0213Z19_PAGE_SIZE];
    uint8_t _red_buffer[GxGDEH0213Z19_PAGE_SIZE];
#else
    uint8_t _black_buffer[GxGDEH0213Z19_BUFFER_SIZE];
    uint8_t _red_buffer[GxGDEH0213Z19_BUFFER_SIZE];
#endif
    GxIO& IO;
    int16_t _current_page;
    bool _using_partial_mode;
    bool _diag_enabled;
    int8_t _rst;
    int8_t _busy;
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
#define GxEPD_Class GxGDEH0213Z19
#define GxEPD_WIDTH GxGDEH0213Z19_WIDTH
#define GxEPD_HEIGHT GxGDEH0213Z19_HEIGHT
#define GxEPD_BitmapExamples <GxGDEH0213Z19/BitmapExamples.h>
#define GxEPD_BitmapExamplesQ "GxGDEH0213Z19/BitmapExamples.h"
#endif

#endif
