#include "GxEPD.h"

#if defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

void  GxEPD::drawBitmapBM(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode)
{
  uint16_t inverse_color = (color != GxEPD_WHITE) ? GxEPD_WHITE : GxEPD_BLACK;
  uint16_t fg_color = (mode & bm_invert) ? inverse_color : color;
  uint16_t bg_color = (mode & bm_invert) ? color : inverse_color;
  // taken from Adafruit_GFX.cpp, modified
  int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
  uint8_t byte = 0;
  for (int16_t j = 0; j < h; j++)
  {
    for (int16_t i = 0; i < w; i++ )
    {
      if (i & 7) byte <<= 1;
      else
      {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
        byte = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
#else
        byte = bitmap[j * byteWidth + i / 8];
#endif
      }
      // keep using overwrite mode
      uint16_t pixelcolor = (byte & 0x80) ? fg_color  : bg_color;
      uint16_t xd = x + i;
      uint16_t yd = y + j;
      if (mode & bm_flip_x) xd = x + w - i;
      if (mode & bm_flip_y) yd = y + h - j;
      drawPixel(xd, yd, pixelcolor);
    }
  }
}


