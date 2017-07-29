/************************************************************************************
   class GxGDEH029A1 : Display class example for GDEH029A1 e-Paper from GoodDisplay.com

   based on Demo Example from GoodDisplay.com, avalable with any order for such a display, no copyright notice.

   Author : J-M Zingg

   modified by :

   Version : 1.1

   Support: minimal, provided as example only, as is, no claim to be fit for serious use

   connection to the e-Paper display is through DESTM32-S2 connection board, available from GoodDisplay

   DESTM32-S2 pinout (top, component side view):
       |-------------------------------------------------
       |  VCC  |o o| VCC 5V
       |  GND  |o o| GND
       |  3.3  |o o| 3.3V
       |  nc   |o o| nc
       |  nc   |o o| nc
       |  nc   |o o| nc
       |  MOSI |o o| CLK
       |  DC   |o o| D/C
       |  RST  |o o| BUSY
       |  nc   |o o| BS
       |-------------------------------------------------
*/
#ifndef _GxGDEH029A1_H_
#define _GxGDEH029A1_H_

#include "../GxEPD.h"

// the physical number of pixels (for controller parameter)
#define GxGDEH029A1_X_PIXELS 128
#define GxGDEH029A1_Y_PIXELS 296

// the logical width and height of the display
#define GxGDEH029A1_WIDTH GxGDEH029A1_X_PIXELS
#define GxGDEH029A1_HEIGHT GxGDEH029A1_Y_PIXELS

#define GxGDEH029A1_BUFFER_SIZE GxGDEH029A1_WIDTH * GxGDEH029A1_HEIGHT / 8

// mapping from DESTM32-S1 evaluation board to Wemos D1 mini

// D10 : MOSI -> D7
// D8  : CS   -> D8
// E14 : RST  -> D4
// E12 : nc   -> nc

// D9  : CLK  -> D5
// E15 : DC   -> D3
// E13 : BUSY -> D2
// E11 : BS   -> GND

class GxGDEH029A1 : public GxEPD
{
  public:
#if defined(ESP8266)
    GxGDEH029A1(GxIO& io, uint8_t rst = D4, uint8_t busy = D2);
#else
    GxGDEH029A1(GxIO& io, uint8_t rst = 9, uint8_t busy = 7);
#endif
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void init(void);
    void fillScreen(uint16_t color); // 0x0 black, >0x0 white, to buffer
    void update(void);
	void updatePartial(uint16_t y1, uint16_t y2);
    // to full screen, filled with white if size is less, no update needed
    void drawBitmap(const uint8_t *bitmap, uint32_t size);
    // to buffer, may be cropped, drawPixel() used, update needed
    void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);
  private:
    void _writeData(uint8_t data);
    void _writeCommand(uint8_t command);
    void _writeCommandData(uint8_t *pCommandData, uint8_t datalen);
    void _SetRamPointer(uint8_t addrX, uint8_t addrY, uint8_t addrY1);
    void _SetRamArea(uint8_t Xstart, uint8_t Xend, uint8_t Ystart, uint8_t Ystart1, uint8_t Yend, uint8_t Yend1);
    void _writeLUT(uint8_t *LUTvalue);
    void _PowerOn(void);
    void _PowerOff(void);
    void _wakeUp(bool fullUpdate = true);
    void _waitWhileBusy(const char* comment=0);
  protected:
    uint8_t _buffer[GxGDEH029A1_BUFFER_SIZE];

  private:
    GxIO& IO;
    uint8_t _rst;
    uint8_t _busy;
};

#endif

#define GxEPD_Class GxGDEH029A1

#define GxEPD_WIDTH GxGDEH029A1_WIDTH
#define GxEPD_HEIGHT GxGDEH029A1_HEIGHT
#define GxEPD_BitmapExamples <GxGDEH029A1/BitmapExamples.h>
#define GxEPD_BitmapExamplesQ "GxGDEH029A1/BitmapExamples.h"

