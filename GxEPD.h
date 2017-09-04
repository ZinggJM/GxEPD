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
    enum bm_mode //BM_ModeSet
    {
      bm_normal = 0,
      bm_default = 1, // for use for BitmapExamples
      // these potentially can be combined
      bm_invert = (1 << 1),
      bm_flip_h = (1 << 2),
      bm_flip_v = (1 << 3),
      bm_r90 = (1 << 4),
      bm_r180 = (1 << 5),
      bm_r270 = bm_r90 | bm_r180,
      bm_partial_update = (1 << 6)
    };
  public:
    GxEPD(int16_t w, int16_t h) : Adafruit_GFX(w, h) {};
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;
    virtual void init(void) = 0;
    virtual void fillScreen(uint16_t color) = 0; // to buffer
    virtual void update(void) = 0;
    // monochrome bitmap to buffer, may be cropped, drawPixel() used, update needed, signature like Adafruit_GFX
    // still pure virtual because of a signature matching issue of the ESP compiler (matches only if declared in subclass)
    virtual void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) = 0;
    // to buffer, may be cropped, drawPixel() used, update needed, different signature, subclass may support some modes
    virtual void drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t m) = 0;
    // monochrome or 4 gray or other to full screen, filled with white if size is less, no update needed
    virtual void drawPicture(const uint8_t *picture, uint32_t size) // b/w or grey is class specific
    {
      drawBitmap(picture, size); // default is monochrome
    };
    // monochrome to full screen, filled with white if size is less, no update needed
    virtual void drawBitmap(const uint8_t *bitmap, uint32_t size) = 0; // monochrome
  protected:
    void drawBitmapBM(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t m);
};

#endif

