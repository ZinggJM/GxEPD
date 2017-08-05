/************************************************************************************
   class GxGDEH029A1 : Display class example for GDEH029A1 e-Paper from GoodDisplay.com

   based on Demo Example from GoodDisplay.com, avalable with any order for such a display, no copyright notice.

   Author : J-M Zingg

   modified by :

   Version : 2.0

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

#include "GxGDEH029A1.h"

#if defined(__AVR)
#include <avr/pgmspace.h>
#endif

#define xPixelsPar (GxGDEH029A1_X_PIXELS -1 )
#define yPixelsPar (GxGDEH029A1_Y_PIXELS -1 )

const uint8_t LUTDefault_full[] =
{
  0x32,  // command
  0x02, 0x02, 0x01, 0x11, 0x12, 0x12, 0x22, 0x22, 0x66, 0x69, 0x69, 0x59, 0x58, 0x99, 0x99,
  0x88, 0x00, 0x00, 0x00, 0x00, 0xF8, 0xB4, 0x13, 0x51, 0x35, 0x51, 0x51, 0x19, 0x01, 0x00
};

const uint8_t LUTDefault_part[] =
{
  0x32,  // command
  0x10, 0x18, 0x18, 0x08, 0x18, 0x18, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x14, 0x44, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t GDOControl[] = {0x01, yPixelsPar % 256, yPixelsPar / 256, 0x00}; //for 2.9inch
uint8_t softstart[] = {0x0c, 0xd7, 0xd6, 0x9d};
uint8_t VCOMVol[] = {0x2c, 0xa8};  // VCOM 7c
uint8_t DummyLine[] = {0x3a, 0x1a}; // 4 dummy line per gate
uint8_t Gatetime[] = {0x3b, 0x08};  // 2us per line
uint8_t RamDataEntryMode[] = {0x11, 0x01};  // Ram data entry mode

GxGDEH029A1::GxGDEH029A1(GxIO& io, uint8_t rst, uint8_t busy) :
  GxEPD(GxGDEH029A1_WIDTH, GxGDEH029A1_HEIGHT),
  IO(io), _rst(rst), _busy(busy),
  _current_page(-1), _using_partial_mode(false)
{
}

template <typename T> static inline void
swap(T& a, T& b)
{
  T t = a;
  a = b;
  b = t;
}

void GxGDEH029A1::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GxGDEH029A1_WIDTH - x - 1;
      break;
    case 2:
      x = GxGDEH029A1_WIDTH - x - 1;
      y = GxGDEH029A1_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GxGDEH029A1_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GxGDEH029A1_WIDTH / 8;
  if (_current_page < 1)
  {
    if (i >= sizeof(_buffer)) return;
  }
  else
  {
    if (i < GxGDEH029A1_PAGE_SIZE * _current_page) return;
    if (i >= GxGDEH029A1_PAGE_SIZE * (_current_page + 1)) return;
    i -= GxGDEH029A1_PAGE_SIZE * _current_page;
  }

  if (!color)
    _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
  else
    _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
}

void GxGDEH029A1::init(void)
{
  IO.init();
  IO.setFrequency(4000000); // 4MHz : 250ns > 150ns min RD cycle
  digitalWrite(_rst, HIGH);
  pinMode(_rst, OUTPUT);
  pinMode(_busy, INPUT);
  fillScreen(GxEPD_WHITE);
}

void GxGDEH029A1::fillScreen(uint16_t color)
{
  uint8_t data = (color == GxEPD_BLACK) ? 0xFF : 0x00;
  for (uint16_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = data;
  }
}

void GxGDEH029A1::update(void)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _Init_Full();
  _writeCommand(0x24);
  for (uint16_t y = 0; y < GxGDEH029A1_HEIGHT; y++)
  {
    for (uint16_t x = GxGDEH029A1_WIDTH / 8; x > 0; x--)
    {
      uint16_t idx = y * (GxGDEH029A1_WIDTH / 8) + x - 1;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
      uint8_t mirror = 0x00;
      for (uint8_t i = 0; i < 8; i++)
      {
        mirror |= ((data >> i) & 0x01) << (7 - i);
      }
      _writeData(~mirror);
    }
  }
  _Update_Full();
  _PowerOff();
}

void  GxGDEH029A1::drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color)
{
  drawBitmap(bitmap, x, y, w, h, color);
}

void  GxGDEH029A1::drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, bool mirror)
{
  if (mirror)
  {
    for (uint16_t x1 = x; x1 < x + w; x1++)
    {
      for (uint16_t y1 = y; y1 < y + h; y1++)
      {
        uint32_t i = (w - (x1 - x)- 1) / 8 + uint32_t(y1 - y) * uint32_t(w) / 8;
#if defined(__AVR)
        uint16_t pixelcolor = (pgm_read_byte(bitmap + i) & (0x01 << (x1 - x) % 8)) ? GxEPD_WHITE  : color;
#else
        uint16_t pixelcolor = (bitmap[i] & (0x01 << (x1 - x) % 8)) ? GxEPD_WHITE  : color;
#endif
        drawPixel(x1, y1, pixelcolor);
      }
    }
  }
  else
  {
    for (uint16_t x1 = x; x1 < x + w; x1++)
    {
      for (uint16_t y1 = y; y1 < y + h; y1++)
      {
        uint32_t i = (x1 - x) / 8 + uint32_t(y1 - y) * uint32_t(w) / 8;
#if defined(__AVR)
        uint16_t pixelcolor = (pgm_read_byte(bitmap + i) & (0x80 >> (x1 - x) % 8)) ? GxEPD_WHITE  : color;
#else
        uint16_t pixelcolor = (bitmap[i] & (0x80 >> (x1 - x) % 8)) ? GxEPD_WHITE  : color;
#endif
        drawPixel(x1, y1, pixelcolor);
      }
    }
  }
}

void GxGDEH029A1::drawBitmap(const uint8_t *bitmap, uint32_t size)
{
  drawBitmap(bitmap, size, false);
}

void GxGDEH029A1::drawBitmap(const uint8_t *bitmap, uint32_t size, bool using_partial_update)
{
  if (using_partial_update)
  {
    _using_partial_mode = true; // remember
    _Init_Part();
    // set full screen
    _SetRamArea(0x00, xPixelsPar / 8, yPixelsPar % 256, yPixelsPar / 256, 0x00, 0x00);  // X-source area,Y-gate area
    _SetRamPointer(0x00, yPixelsPar % 256, yPixelsPar / 256); // set ram
    _writeCommand(0x24);
    for (uint32_t i = 0; i < GxGDEH029A1_BUFFER_SIZE; i++)
    {
#if defined(__AVR)
      _writeData((i < size) ? pgm_read_byte(bitmap + i) : 0xFF);
#else
      _writeData((i < size) ? bitmap[i] : 0xFF);
#endif
    }
    _Update_Part();
    _PowerOff();
  }
  else
  {
    _using_partial_mode = false; // remember
    _Init_Full();
    _writeCommand(0x24);
    for (uint32_t i = 0; i < GxGDEH029A1_BUFFER_SIZE; i++)
    {
#if defined(__AVR)
      _writeData((i < size) ? pgm_read_byte(bitmap + i) : 0xFF);
#else
      _writeData((i < size) ? bitmap[i] : 0xFF);
#endif
    }
    _Update_Full();
    _PowerOff();
  }
}

void GxGDEH029A1::eraseDisplay(bool using_partial_mode)
{
  if (using_partial_mode)
  {
    _using_partial_mode = true; // remember
    _Init_Part();
    _writeCommand(0x24);
    for (uint32_t i = 0; i < GxGDEH029A1_BUFFER_SIZE; i++)
    {
      _writeData(0xFF);
    }
    _Update_Part();
    _PowerOff();
  }
  else
  {
    _Init_Full();
    _writeCommand(0x24);
    for (uint32_t i = 0; i < GxGDEH029A1_BUFFER_SIZE; i++)
    {
      _writeData(0xFF);
    }
    _Update_Full();
    _PowerOff();
  }
}

void GxGDEH029A1::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  //fillScreen(0x0);
  if (x >= GxGDEH029A1_WIDTH) return;
  if (y >= GxGDEH029A1_HEIGHT) return;
  if (using_rotation)
  {
    switch (getRotation())
    {
      case 1:
        swap(x, y);
        swap(w, h);
        x = GxGDEH029A1_WIDTH - x - w - 1;
        break;
      case 2:
        x = GxGDEH029A1_WIDTH - x - w - 1;
        y = GxGDEH029A1_HEIGHT - y - h - 1;
        break;
      case 3:
        swap(x, y);
        swap(w, h);
        y = GxGDEH029A1_HEIGHT - y  - h - 1;
        break;
    }
  }
  uint16_t xe = min(GxGDEH029A1_WIDTH, x + w) - 1;
  uint16_t ye = min(GxGDEH029A1_HEIGHT, y + h) - 1;
  uint16_t xs_bx = x / 8;
  uint16_t xe_bx = (xe + 7) / 8;
  uint16_t p_xs = (GxGDEH029A1_WIDTH - xe - 1) / 8;
  uint16_t p_xe = (GxGDEH029A1_WIDTH - x - 1) / 8;
  uint16_t p_ys = (GxGDEH029A1_HEIGHT - y - 1);
  uint16_t p_ye = (GxGDEH029A1_HEIGHT - ye - 1);
  _Init_Part();
  _SetRamArea(p_xs, p_xe, p_ys % 256, p_ys / 256, p_ye % 256, p_ye / 256); // X-source area,Y-gate area
  _SetRamPointer(p_xs, p_ys % 256, p_ys / 256); // set ram
  _waitWhileBusy();
  _writeCommand(0x24);
  for (int16_t y1 = y; y1 <= ye; y1++)
  {
    for (int16_t x1 = xe_bx; x1 > xs_bx; x1--)
    {
      uint16_t idx = y1 * (GxGDEH029A1_WIDTH / 8) + x1 - 1;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
      uint8_t mirror = 0x00;
      for (uint8_t i = 0; i < 8; i++)
      {
        mirror |= ((data >> i) & 0x01) << (7 - i);
      }
      _writeData(~mirror);
    }
  }
  _Update_Part();
  delay(300);
  _SetRamArea(p_xs, p_xe, p_ys % 256, p_ys / 256, p_ye % 256, p_ye / 256); // X-source area,Y-gate area
  _SetRamPointer(p_xs, p_ys % 256, p_ys / 256); // set ram
  _waitWhileBusy();
  _writeCommand(0x24);
  for (int16_t y1 = y; y1 <= ye; y1++)
  {
    for (int16_t x1 = xe_bx; x1 > xs_bx; x1--)
    {
      uint16_t idx = y1 * (GxGDEH029A1_WIDTH / 8) + x1 - 1;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
      uint8_t mirror = 0x00;
      for (uint8_t i = 0; i < 8; i++)
      {
        mirror |= ((data >> i) & 0x01) << (7 - i);
      }
      _writeData(~mirror);
    }
  }
  delay(300);
  _PowerOff();
}

void GxGDEH029A1::_writeCommand(uint8_t command)
{
  //while (digitalRead(_busy));
  if (digitalRead(_busy))
  {
    String str = String("command 0x") + String(command, HEX);
    _waitWhileBusy(str.c_str());
  }
  IO.writeCommandTransaction(command);
}

void GxGDEH029A1::_writeData(uint8_t data)
{
  IO.writeDataTransaction(data);
}

void GxGDEH029A1::_writeCommandData(const uint8_t* pCommandData, uint8_t datalen)
{
  //while (digitalRead(_busy)); // wait
  if (digitalRead(_busy))
  {
    String str = String("command 0x") + String(pCommandData[0], HEX);
    _waitWhileBusy(str.c_str());
  }
  IO.startTransaction();
  IO.writeCommand(*pCommandData++);
  for (uint8_t i = 0; i < datalen - 1; i++)	// sub the command
  {
    IO.writeData(*pCommandData++);
  }
  IO.endTransaction();

}

void GxGDEH029A1::_waitWhileBusy(const char* comment)
{
  unsigned long start = micros();
  while (1)
  {
    if (!digitalRead(_busy)) break;
    delay(1);
  }
  if (comment)
  {
    unsigned long elapsed = micros() - start;
    //Serial.print(comment);
    //Serial.print(" : ");
    //Serial.println(elapsed);
  }
}

void GxGDEH029A1::_SetRamArea(uint8_t Xstart, uint8_t Xend, uint8_t Ystart, uint8_t Ystart1, uint8_t Yend, uint8_t Yend1)
{
  _writeCommand(0x44);
  _writeData(Xstart);
  _writeData(Xend);
  _writeCommand(0x45);
  _writeData(Ystart);
  _writeData(Ystart1);
  _writeData(Yend);
  _writeData(Yend1);
}

void GxGDEH029A1::_SetRamPointer(uint8_t addrX, uint8_t addrY, uint8_t addrY1)
{
  _writeCommand(0x4e);
  _writeData(addrX);
  _writeCommand(0x4f);
  _writeData(addrY);
  _writeData(addrY1);
}

void GxGDEH029A1::_PowerOn(void)
{
  _writeCommand(0x22);
  _writeData(0xc0);
  _writeCommand(0x20);
}

void GxGDEH029A1::_PowerOff(void)
{
  _writeCommand(0x22);
  _writeData(0xc3);
  _writeCommand(0x20);
}

void GxGDEH029A1::_InitDisplay(void)
{
  _writeCommandData(GDOControl, sizeof(GDOControl));  // Pannel configuration, Gate selection
  _writeCommandData(softstart, sizeof(softstart));  // X decrease, Y decrease
  _writeCommandData(VCOMVol, sizeof(VCOMVol));    // VCOM setting
  _writeCommandData(DummyLine, sizeof(DummyLine));  // dummy line per gate
  _writeCommandData(Gatetime, sizeof(Gatetime));    // Gate time setting
  _writeCommandData(RamDataEntryMode, sizeof(RamDataEntryMode));  // X increase, Y decrease
  _SetRamArea(0x00, xPixelsPar / 8, yPixelsPar % 256, yPixelsPar / 256, 0x00, 0x00);  // X-source area,Y-gate area
  _SetRamPointer(0x00, yPixelsPar % 256, yPixelsPar / 256); // set ram
}

void GxGDEH029A1::_Init_Full(void)
{
  _InitDisplay();
  _writeCommandData(LUTDefault_full, sizeof(LUTDefault_full));
  _PowerOn();
}

void GxGDEH029A1::_Init_Part(void)
{
  _InitDisplay();
  _writeCommandData(LUTDefault_part, sizeof(LUTDefault_part));
  _PowerOn();
}

void GxGDEH029A1::_Update_Full(void)
{
  _writeCommand(0x22);
  _writeData(0xc4);
  _writeCommand(0x20);
  _writeCommand(0xff);
}

void GxGDEH029A1::_Update_Part(void)
{
  _writeCommand(0x22);
  _writeData(0x04);
  _writeCommand(0x20);
  _writeCommand(0xff);
}

void GxGDEH029A1::_drawCurrentPage()
{
  uint16_t x = 0;
  uint16_t xe = GxGDEH029A1_WIDTH - 1;
  uint16_t y = GxGDEH029A1_PAGE_HEIGHT * _current_page;
  uint16_t ye = y + GxGDEH029A1_PAGE_HEIGHT - 1;
  uint16_t p_xs = (GxGDEH029A1_WIDTH - xe - 1) / 8;
  uint16_t p_xe = (GxGDEH029A1_WIDTH - x - 1) / 8;
  uint16_t p_ys = (GxGDEH029A1_HEIGHT - y - 1);
  uint16_t p_ye = (GxGDEH029A1_HEIGHT - ye - 1);
  _SetRamArea(p_xs, p_xe, p_ys % 256, p_ys / 256, p_ye % 256, p_ye / 256); // X-source area,Y-gate area
  _SetRamPointer(p_xs, p_ys % 256, p_ys / 256); // set ram
  _waitWhileBusy();
  _writeCommand(0x24);
  for (int16_t y1 = 0; y1 < GxGDEH029A1_PAGE_HEIGHT; y1++)
  {
    for (int16_t x1 = GxGDEH029A1_WIDTH / 8; x1 > 0; x1--)
    {
      uint16_t idx = y1 * (GxGDEH029A1_WIDTH / 8) + x1 - 1;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
      uint8_t mirror = 0x00;
      for (uint8_t i = 0; i < 8; i++)
      {
        mirror |= ((data >> i) & 0x01) << (7 - i);
      }
      _writeData(~mirror);
    }
  }
  _Update_Part();
  delay(300);
#if 1 // this is required
  _SetRamArea(p_xs, p_xe, p_ys % 256, p_ys / 256, p_ye % 256, p_ye / 256); // X-source area,Y-gate area
  _SetRamPointer(p_xs, p_ys % 256, p_ys / 256); // set ram
  _waitWhileBusy();
  _writeCommand(0x24);
  for (int16_t y1 = 0; y1 < GxGDEH029A1_PAGE_HEIGHT; y1++)
  {
    for (int16_t x1 = GxGDEH029A1_WIDTH / 8; x1 > 0; x1--)
    {
      uint16_t idx = y1 * (GxGDEH029A1_WIDTH / 8) + x1 - 1;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
      uint8_t mirror = 0x00;
      for (uint8_t i = 0; i < 8; i++)
      {
        mirror |= ((data >> i) & 0x01) << (7 - i);
      }
      _writeData(~mirror);
    }
  }
  delay(300);
#endif
}

void GxGDEH029A1::drawPaged(void (*drawCallback)(void))
{
  if (!_using_partial_mode)
  {
    eraseDisplay(false);
    eraseDisplay(true);
  }
  _using_partial_mode = true;
  _Init_Part();
  for (_current_page = 0; _current_page < GxGDEH029A1_PAGES; _current_page++)
  {
    fillScreen(0xFF);
    drawCallback();
    //fillScreen(0x00);
    _drawCurrentPage();
    //delay(2000);
  }
  _current_page = -1;
  _PowerOff();
}

