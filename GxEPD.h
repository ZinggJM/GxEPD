#ifndef _GxEPD_H_
#define _GxEPD_H_

#include <Arduino.h>
#include <SPI.h>
#include "GxIO/GxIO.h"
#include <Adafruit_GFX.h>

// the only colors supported by any of these displays; mapping of other colors is class specific
#define GxEPD_BLACK     0x0000
#define GxEPD_DARKGREY  0x7BEF      /* 128, 128, 128 */
#define GxEPD_LIGHTGREY 0xC618      /* 192, 192, 192 */
#define GxEPD_WHITE     0xFFFF

class GxEPD : public Adafruit_GFX
{
  public:
    GxEPD(int16_t w, int16_t h) : Adafruit_GFX(w, h) {};
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;
    virtual void init(void) = 0;
    virtual void fillScreen(uint16_t color) = 0; // to buffer
    virtual void update(void) = 0;
    // to full screen, filled with white if size is less, no update needed
    virtual void drawBitmap(const uint8_t *bitmap, uint32_t size) = 0; // b/w or grey is class specific
    // b/w bitmap to buffer, may be cropped, drawPixel() used, update needed
    virtual void  drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) = 0;
};

#endif

