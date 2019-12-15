#ifndef _HINKE042A11_H_
#define _HINKE042A11_H_

#include "GxEPD.h"
//4.2"
#define HINKE042A11_WIDTH 400
#define HINKE042A11_HEIGHT 300

#define HINKE042A11_BUFFER_SIZE (uint32_t(HINKE042A11_WIDTH) * uint32_t(HINKE042A11_HEIGHT) / 8)

// divisor for AVR, should be factor of HINKE042A11_HEIGHT
#define HINKE042A11_PAGES 8

#define HINKE042A11_PAGE_HEIGHT (HINKE042A11_HEIGHT / HINKE042A11_PAGES)
#define HINKE042A11_PAGE_SIZE (HINKE042A11_BUFFER_SIZE / HINKE042A11_PAGES)

class HINKE042A11 : public GxEPD
{
  public:
#if defined(ESP8266)
    //HINKE042A11(GxIO& io, int8_t rst = D4, int8_t busy = D2);
    // use pin numbers, other ESP8266 than Wemos may not use Dx names
    HINKE042A11(GxIO& io, int8_t rst = 2, int8_t busy = 4);
#else
    HINKE042A11(GxIO& io, int8_t rst = 9, int8_t busy = 7);
#endif
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void init(uint32_t serial_diag_bitrate = 0); // = 0 : disabled
    void fillScreen(uint16_t color); // to buffer
    void update(void);
    // to buffer, may be cropped, drawPixel() used, update needed
    void drawExamplePicture(const uint8_t* black_bitmap, const uint8_t* red_bitmap, uint32_t black_size, uint32_t red_size);
    // to full screen, filled with white if size is less, no update needed, black  /white / red, general version
    void drawPicture(const uint8_t* black_bitmap, const uint8_t* red_bitmap, uint32_t black_size, uint32_t red_size, int16_t mode = bm_normal);
    // to full screen, filled with white if size is less, no update needed
     void powerDown();
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
    void _waitWhileBusy(const char* comment = 0);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);
  private:
#if defined(__AVR)
    uint8_t _black_buffer[HINKE042A11_PAGE_SIZE];
    uint8_t _red_buffer[HINKE042A11_PAGE_SIZE];
#else
    uint8_t _black_buffer[HINKE042A11_BUFFER_SIZE];
    uint8_t _red_buffer[HINKE042A11_BUFFER_SIZE];
#endif
    GxIO& IO;
    int16_t _current_page;
    bool _using_partial_mode;
    bool _diag_enabled;
    int8_t _rst;
    int8_t _busy;
};

#ifndef GxEPD_Class
#define GxEPD_Class HINKE042A11
#define GxEPD_WIDTH HINKE042A11_WIDTH
#define GxEPD_HEIGHT HINKE042A11_HEIGHT
#endif

#endif
