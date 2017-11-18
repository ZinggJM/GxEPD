/************************************************************************************
   class GxGDEW0154Z04 : Display class example for GDEW0154Z04 e-Paper from Dalian Good Display Co., Ltd.: www.good-display.com

   based on Demo Example from Good Display, available here: http://www.good-display.com/download_detail/downloadsId=528.html

   Author : J-M Zingg

   Version : 2.3

   Support: limited, provided as example, no claim to be fit for serious use

   Controller: IL0376F : http://www.good-display.com/download_detail/downloadsId=541.html

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

   note: for correct red color jumper J3 must be set on 0.47 side (towards FCP connector)

*/
#ifndef _GxGDEW0154Z04_H_
#define _GxGDEW0154Z04_H_

#include "../GxEPD.h"

#define GxGDEW0154Z04_WIDTH 200
#define GxGDEW0154Z04_HEIGHT 200

#define GxGDEW0154Z04_BUFFER_SIZE (uint32_t(GxGDEW0154Z04_WIDTH) * uint32_t(GxGDEW0154Z04_HEIGHT) / 8)

// divisor for AVR, should be factor of GxGDEW0154Z04_HEIGHT
#define GxGDEW0154Z04_PAGES 10

#define GxGDEW0154Z04_PAGE_HEIGHT (GxGDEW0154Z04_HEIGHT / GxGDEW0154Z04_PAGES)
#define GxGDEW0154Z04_PAGE_SIZE (GxGDEW0154Z04_BUFFER_SIZE / GxGDEW0154Z04_PAGES)

// mapping suggestion from Waveshare 1.54inch e-Paper to Wemos D1 mini
// BUSY -> D2, RST -> D4, DC -> D3, CS -> D8, CLK -> D5, DIN -> D7, GND -> GND, 3.3V -> 3.3V

// mapping suggestion for ESP32, e.g. LOLIN32, see .../variants/.../pins_arduino.h for your board
// NOTE: there are variants with different pins for SPI ! CHECK SPI PINS OF YOUR BOARD
// BUSY -> 4, RST -> 16, DC -> 17, CS -> SS(5), CLK -> SCK(18), DIN -> MOSI(23), GND -> GND, 3.3V -> 3.3V

// mapping suggestion for AVR, UNO, NANO etc.
// BUSY -> 7, RST -> 9, DC -> 8, CS-> 10, CLK -> 13, DIN -> 11

class GxGDEW0154Z04 : public GxEPD
{
  public:
#if defined(ESP8266)
    //GxGDEW0154Z04(GxIO& io, int8_t rst = D4, int8_t busy = D2);
    // use pin numbers, other ESP8266 than Wemos may not use Dx names
    GxGDEW0154Z04(GxIO& io, int8_t rst = 2, int8_t busy = 4);
#else
    GxGDEW0154Z04(GxIO& io, int8_t rst = 9, int8_t busy = 7);
#endif
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void init(void);
    void fillScreen(uint16_t color); // to buffer
    void update(void);
    // to buffer, may be cropped, drawPixel() used, update needed
    void  drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode = bm_normal);
    // to full screen, filled with white if size is less, no update needed, 4 gray levels + red, for example
    void drawExamplePicture(const uint8_t* black_bitmap, const uint8_t* red_bitmap, uint32_t black_size, uint32_t red_size);
    // to full screen, filled with white if size is less, no update needed, black  /white / red, general version
    void drawPicture(const uint8_t* black_bitmap, const uint8_t* red_bitmap, uint32_t black_size, uint32_t red_size, int16_t mode = bm_normal);
    // to full screen, filled with white if size is less, no update needed
    void drawBitmap(const uint8_t *bitmap, uint32_t size, int16_t mode = bm_normal); // only bm_normal, bm_invert mode implemented
    void eraseDisplay(bool using_partial_update = false); // parameter ignored
    // paged drawing, for limited RAM, drawCallback() is called GxGDEW0154Z04_PAGES times
    // each call of drawCallback() should draw the same
    void drawPaged(void (*drawCallback)(void));
    void drawPaged(void (*drawCallback)(uint32_t), uint32_t);
    void drawPaged(void (*drawCallback)(const void*), const void*);
    void drawPaged(void (*drawCallback)(const void*, const void*), const void*, const void*);
    void drawCornerTest(uint8_t em = 0x01);
  private:
    template <typename T> static inline void
    swap(T& a, T& b)
    {
      T t = a;
      a = b;
      b = t;
    }
    void _writeData(uint8_t data);
    void _writeCommand(uint8_t command);
    void _writeLUT();
    void _wakeUp();
    void _sleep();
    void _waitWhileBusy(const char* comment = 0);
  private:
#if defined(__AVR)
    uint8_t _black_buffer[GxGDEW0154Z04_PAGE_SIZE];
    uint8_t _red_buffer[GxGDEW0154Z04_PAGE_SIZE];
#else
    uint8_t _black_buffer[GxGDEW0154Z04_BUFFER_SIZE];
    uint8_t _red_buffer[GxGDEW0154Z04_BUFFER_SIZE];
#endif
    GxIO& IO;
    int16_t _current_page;
    int8_t _rst;
    int8_t _busy;
    static const uint8_t lut_vcom0[];
    static const uint8_t lut_w[];
    static const uint8_t lut_b[];
    static const uint8_t lut_g1[];
    static const uint8_t lut_g2[];
    static const uint8_t lut_vcom1[];
    static const uint8_t lut_red0[];
    static const uint8_t lut_red1[];
    static const uint8_t bw2grey[];
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
#define GxEPD_Class GxGDEW0154Z04
#define GxEPD_WIDTH GxGDEW0154Z04_WIDTH
#define GxEPD_HEIGHT GxGDEW0154Z04_HEIGHT
#define GxEPD_BitmapExamples <GxGDEW0154Z04/BitmapExamples.h>
#define GxEPD_BitmapExamplesQ "GxGDEW0154Z04/BitmapExamples.h"
#endif

#endif

