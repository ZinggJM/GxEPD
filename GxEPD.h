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
#define GxEPD_RED       0xF800      /* 255,   0,   0 */

class GxEPD : public Adafruit_GFX
{
  public:
    // bitmap presentation modes may be partially implemented by subclasses
    enum bm_mode
    {
      bm_normal,
      bm_default, // for use for BitmapExamples
      bm_flip_h,
      bm_flip_v,
      bm_r90,
      bm_r180,
      bm_r270
    };
  public:
    GxEPD(int16_t w, int16_t h) : Adafruit_GFX(w, h) {};
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;
    virtual void init(void) = 0;
    virtual void fillScreen(uint16_t color) = 0; // to buffer
    virtual void update(void) = 0;
    // monochrome bitmap to buffer, may be cropped, drawPixel() used, update needed, Adafruit_GFX signature
    virtual void  drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) = 0;
    // to buffer, may be cropped, drawPixel() used, update needed, new signature, sublass may support some bm_modes
    virtual void  drawBitmap(const uint8_t *bitmap, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color, bm_mode m = bm_normal)
    {
      // fall back, not yet implemented in all subclasses
      drawBitmap(x, y, bitmap, w, h, color);
    };
    // monochrome or 4 gray or other to full screen, filled with white if size is less, no update needed
    virtual void drawPicture(const uint8_t *picture, uint32_t size) // b/w or grey is class specific
    {
      drawBitmap(picture, size); // default is monochrome
    };
    // monochrome to full screen, filled with white if size is less, no update needed
    virtual void drawBitmap(const uint8_t *bitmap, uint32_t size) = 0; // monochrome
};

#endif

