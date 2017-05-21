#ifndef _GxEPD_H_
#define _GxEPD_H_

#include <Arduino.h>
#include <SPI.h>
#include "GxIO/GxIO.h"
#include <Adafruit_GFX.h>

// the only colors supported by this display, all other values are mapped to white in this display class
#define GxEPD_BLACK 0x0000
#define GxEPD_WHITE 0xFFFF

class GxEPD : public Adafruit_GFX
{
  public:
    GxEPD(int16_t w, int16_t h) : Adafruit_GFX(w, h) {};
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;
    virtual void init(void) = 0;
    virtual void fillScreen(uint16_t color) = 0; // 0x0 black, >0x0 white, to buffer
    virtual void update(void) = 0;
    // to full screen, filled with white if size is less, no update needed
    virtual void drawBitmap(const uint8_t *bitmap, uint16_t size) = 0;
    // to buffer, may be cropped, drawPixel() used, update needed
    virtual void  drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) = 0;
};

#endif

