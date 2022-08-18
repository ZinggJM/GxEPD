// class GxGDEH0154Z90 : Display class for GDEH0154Z90 e-Paper from Dalian Good Display Co., Ltd.: www.good-display.com
//
// based on Demo Example from Good Display, available here: http://www.good-display.com/download_detail/downloadsId=515.html
// Panel: GDEH0154Z90 : https://www.good-display.com/product/285.html
// Controller: SSD1681 : https://v4.cecdn.yun300.cn/100001_1909185148/SSD1681%20V0.13%20Spec.pdf
//
// Author : J-M Zingg
//
// Version : see library.properties
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// Library: https://github.com/ZinggJM/GxEPD

#include "GxGDEH0154Z90.h"

//#define DISABLE_DIAGNOSTIC_OUTPUT

#if defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

// Partial Update Delay, may have an influence on degradation
#define GxGDEH0154Z90_PU_DELAY 500

GxGDEH0154Z90::GxGDEH0154Z90(GxIO& io, int8_t rst, int8_t busy)
  : GxEPD(GxGDEH0154Z90_WIDTH, GxGDEH0154Z90_HEIGHT), IO(io),
    _current_page(-1), _power_is_on(false), _using_partial_mode(false), _hibernating(false), _diag_enabled(false),
    _rst(rst), _busy(busy)
{
}

void GxGDEH0154Z90::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GxGDEH0154Z90_WIDTH - x - 1;
      break;
    case 2:
      x = GxGDEH0154Z90_WIDTH - x - 1;
      y = GxGDEH0154Z90_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GxGDEH0154Z90_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GxGDEH0154Z90_WIDTH / 8;
  if (_current_page < 1)
  {
    if (i >= sizeof(_black_buffer)) return;
  }
  else
  {
    y -= _current_page * GxGDEH0154Z90_PAGE_HEIGHT;
    if ((y < 0) || (y >= GxGDEH0154Z90_PAGE_HEIGHT)) return;
    i = x / 8 + y * GxGDEH0154Z90_WIDTH / 8;
  }

  _black_buffer[i] = (_black_buffer[i] & (0xFF ^ (1 << (7 - x % 8)))); // white
  _red_buffer[i] = (_red_buffer[i] & (0xFF ^ (1 << (7 - x % 8)))); // white
  if (color == GxEPD_WHITE) return;
  else if (color == GxEPD_BLACK) _black_buffer[i] = (_black_buffer[i] | (1 << (7 - x % 8)));
  else if (color == GxEPD_RED) _red_buffer[i] = (_red_buffer[i] | (1 << (7 - x % 8)));
  else
  {
    if ((color & 0xF100) > (0xF100 / 2)) _red_buffer[i] = (_red_buffer[i] | (1 << (7 - x % 8)));
    else if ((((color & 0xF100) >> 11) + ((color & 0x07E0) >> 5) + (color & 0x001F)) < 3 * 255 / 2)
    {
      _black_buffer[i] = (_black_buffer[i] | (1 << (7 - x % 8)));
    }
  }
}


void GxGDEH0154Z90::init(uint32_t serial_diag_bitrate)
{
  if (serial_diag_bitrate > 0)
  {
    Serial.begin(serial_diag_bitrate);
    _diag_enabled = true;
  }
  IO.init();
  IO.setFrequency(4000000); // 4MHz
  if (_rst >= 0)
  {
    digitalWrite(_rst, HIGH);
    pinMode(_rst, OUTPUT);
  }
  pinMode(_busy, INPUT);
  fillScreen(GxEPD_WHITE);
  _current_page = -1;
  _using_partial_mode = false;
}

void GxGDEH0154Z90::fillScreen(uint16_t color)
{
  uint8_t black = 0x00;
  uint8_t red = 0x00;
  if (color == GxEPD_WHITE);
  else if (color == GxEPD_BLACK) black = 0xFF;
  else if (color == GxEPD_RED) red = 0xFF;
  else if ((color & 0xF100) > (0xF100 / 2))  red = 0xFF;
  else if ((((color & 0xF100) >> 11) + ((color & 0x07E0) >> 5) + (color & 0x001F)) < 3 * 255 / 2) black = 0xFF;
  for (uint16_t x = 0; x < sizeof(_black_buffer); x++)
  {
    _black_buffer[x] = black;
    _red_buffer[x] = red;
  }
}

void GxGDEH0154Z90::update(void)
{
  if (_current_page != -1) return;
  _Init_Full();
  _writeCommand(0x24);
  for (uint32_t i = 0; i < GxGDEH0154Z90_BUFFER_SIZE; i++)
  {
    _writeData((i < sizeof(_black_buffer)) ? ~_black_buffer[i] : 0xFF); // white is 0x00 in buffer, 0xFF on device
  }
  _writeCommand(0x26);
  for (uint32_t i = 0; i < GxGDEH0154Z90_BUFFER_SIZE; i++)
  {
    _writeData((i < sizeof(_red_buffer)) ? _red_buffer[i] : 0x00); // white is 0x00 in buffer, 0x00 on device
  }
  _Update_Full();
  _PowerOff();
}

void  GxGDEH0154Z90::drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode)
{
  if (mode & bm_default) mode |= bm_invert;
  drawBitmapBM(bitmap, x, y, w, h, color, mode);
}

void GxGDEH0154Z90::drawExamplePicture(const uint8_t* black_bitmap, const uint8_t* red_bitmap, uint32_t black_size, uint32_t red_size)
{
  //drawPicture(black_bitmap, red_bitmap, black_size, red_size, bm_invert_red);
  drawPicture(black_bitmap, red_bitmap, black_size, red_size, bm_normal);
}

void GxGDEH0154Z90::drawPicture(const uint8_t* black_bitmap, const uint8_t* red_bitmap, uint32_t black_size, uint32_t red_size, int16_t mode)
{
  if (_current_page != -1) return;
  _Init_Full();
  _writeCommand(0x24);
  for (uint32_t i = 0; i < GxGDEH0154Z90_BUFFER_SIZE; i++)
  {
    uint8_t data = 0xFF; // white is 0xFF in bitmap
    if (i < black_size)
    {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
      data = pgm_read_byte(&black_bitmap[i]);
#else
      data = black_bitmap[i];
#endif
      if (mode & bm_invert) data = ~data;
    }
    _writeData(data); // white is 0xFF on device
  }
  _writeCommand(0x26);
  for (uint32_t i = 0; i < GxGDEH0154Z90_BUFFER_SIZE; i++)
  {
    uint8_t data = 0xFF; // white is 0xFF in bitmap
    if (i < red_size)
    {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
      data = pgm_read_byte(&red_bitmap[i]);
#else
      data = red_bitmap[i];
#endif
      if (mode & bm_invert_red) data = ~data;
    }
    _writeData(~data); // white is 0x00 on device
  }
  _Update_Full();
  _PowerOff();
}

void GxGDEH0154Z90::drawBitmap(const uint8_t* bitmap, uint32_t size, int16_t mode)
{
  if (_current_page != -1) return;
  // example bitmaps are normal on b/w, but inverted on red
  if (mode & bm_default) mode |= bm_normal;
  if (mode & bm_partial_update)
  {
    _Init_Part();
    _setPartialRamArea(0, 0, GxGDEH0154Z90_WIDTH - 1, GxGDEH0154Z90_HEIGHT - 1);
    _writeCommand(0x24);
    for (uint32_t i = 0; i < GxGDEH0154Z90_BUFFER_SIZE; i++)
    {
      uint8_t data = 0xFF; // white is 0xFF in bitmap
      if (i < size)
      {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
        data = pgm_read_byte(&bitmap[i]);
#else
        data = bitmap[i];
#endif
        if (mode & bm_invert) data = ~data;
      }
      _writeData(data); // white is 0xFF on device
    }
    _writeCommand(0x26);
    for (uint32_t i = 0; i < GxGDEH0154Z90_BUFFER_SIZE; i++)
    {
      _writeData(0x00); // white is 0x00 on device
    }
    _Update_Part();
  }
  else
  {
    _Init_Full();
    _writeCommand(0x24);
    for (uint32_t i = 0; i < GxGDEH0154Z90_BUFFER_SIZE; i++)
    {
      uint8_t data = 0xFF; // white is 0xFF in bitmap
      if (i < size)
      {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
        data = pgm_read_byte(&bitmap[i]);
#else
        data = bitmap[i];
#endif
        if (mode & bm_invert) data = ~data;
      }
      _writeData(data); // white is 0xFF on device
    }
    _writeCommand(0x26);
    for (uint32_t i = 0; i < GxGDEH0154Z90_BUFFER_SIZE; i++)
    {
      _writeData(0x00); // white is 0x00 on device
    }
    _Update_Full();
    _PowerOff();
  }
}

void GxGDEH0154Z90::eraseDisplay(bool using_partial_update)
{
  if (_current_page != -1) return;
  if (using_partial_update)
  {
    _Init_Part();
    _setPartialRamArea(0, 0, GxGDEH0154Z90_WIDTH - 1, GxGDEH0154Z90_HEIGHT - 1);
    _writeCommand(0x24);
    for (uint32_t i = 0; i < GxGDEH0154Z90_BUFFER_SIZE * 2; i++)
    {
      _writeData(0xFF); // white is 0xFF on device
    }
    _writeCommand(0x26);
    for (uint32_t i = 0; i < GxGDEH0154Z90_BUFFER_SIZE; i++)
    {
      _writeData(0x00); // white is 0x00 on device
    }
    _Update_Part();
  }
  else
  {
    _Init_Full();
    _writeCommand(0x24);
    for (uint32_t i = 0; i < GxGDEH0154Z90_BUFFER_SIZE * 2; i++)
    {
      _writeData(0xFF); // white is 0xFF on device
    }
    _writeCommand(0x26);
    for (uint32_t i = 0; i < GxGDEH0154Z90_BUFFER_SIZE; i++)
    {
      _writeData(0x00); // white is 0x00 on device
    }
    _Update_Full();
    _PowerOff();
  }
}

void GxGDEH0154Z90::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  if (_current_page != -1) return;
  if (using_rotation) _rotate(x, y, w, h);
  if (x >= GxGDEH0154Z90_WIDTH) return;
  if (y >= GxGDEH0154Z90_HEIGHT) return;
  // x &= 0xFFF8; // byte boundary, not here, use encompassing rectangle
  uint16_t xe = gx_uint16_min(GxGDEH0154Z90_WIDTH, x + w) - 1;
  uint16_t ye = gx_uint16_min(GxGDEH0154Z90_HEIGHT, y + h) - 1;
  // x &= 0xFFF8; // byte boundary, not needed here
  uint16_t xs_bx = x / 8;
  uint16_t xe_bx = (xe + 7) / 8;
  _Init_Part();
  _setPartialRamArea(x, y, xe, ye);
  _writeCommand(0x24);
  for (int16_t y1 = y; y1 <= ye; y1++)
  {
    for (int16_t x1 = xs_bx; x1 < xe_bx; x1++)
    {
      uint16_t idx = y1 * (GxGDEH0154Z90_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0x00; // white is 0x00 in buffer
      _writeData(~data); // white is 0xFF on device
    }
  }
  _writeCommand(0x26);
  for (int16_t y1 = y; y1 <= ye; y1++)
  {
    for (int16_t x1 = xs_bx; x1 < xe_bx; x1++)
    {
      uint16_t idx = y1 * (GxGDEH0154Z90_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_red_buffer)) ? _red_buffer[idx] : 0x00; // white is 0x00 in buffer
      _writeData(data); // white is 0xFF on device
    }
  }
  _Update_Part();
  delay(GxGDEH0154Z90_PU_DELAY); // don't stress this display
}

void GxGDEH0154Z90::updateToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h, bool using_rotation)
{
  _Init_Part();
  _writeToWindow(xs, ys, xd, yd, w, h, using_rotation);
  _Update_Part();
  delay(GxGDEH0154Z90_PU_DELAY); // don't stress this display
}

void GxGDEH0154Z90::_writeToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h, bool using_rotation)
{
  if (using_rotation)
  {
    switch (getRotation())
    {
      case 1:
        swap(xs, ys);
        swap(xd, yd);
        swap(w, h);
        xs = GxGDEH0154Z90_WIDTH - xs - w;
        xd = GxGDEH0154Z90_WIDTH - xd - w;
        break;
      case 2:
        xs = GxGDEH0154Z90_WIDTH - xs - w;
        ys = GxGDEH0154Z90_HEIGHT - ys - h;
        xd = GxGDEH0154Z90_WIDTH - xd - w;
        yd = GxGDEH0154Z90_HEIGHT - yd - h;
        break;
      case 3:
        swap(xs, ys);
        swap(xd, yd);
        swap(w, h);
        ys = GxGDEH0154Z90_HEIGHT - ys  - h;
        yd = GxGDEH0154Z90_HEIGHT - yd  - h;
        break;
    }
  }
  if (xs >= GxGDEH0154Z90_WIDTH) return;
  if (ys >= GxGDEH0154Z90_HEIGHT) return;
  if (xd >= GxGDEH0154Z90_WIDTH) return;
  if (yd >= GxGDEH0154Z90_HEIGHT) return;
  // the screen limits are the hard limits
  uint16_t xde = gx_uint16_min(GxGDEH0154Z90_WIDTH, xd + w) - 1;
  uint16_t yde = gx_uint16_min(GxGDEH0154Z90_HEIGHT, yd + h) - 1;
  // soft limits, must send as many bytes as set by _SetRamArea
  uint16_t yse = ys + yde - yd;
  uint16_t xss_d8 = xs / 8;
  uint16_t xse_d8 = xss_d8 + _setPartialRamArea(xd, yd, xde, yde);
  _writeCommand(0x24);
  for (int16_t y1 = ys; y1 <= yse; y1++)
  {
    for (int16_t x1 = xss_d8; x1 < xse_d8; x1++)
    {
      uint16_t idx = y1 * (GxGDEH0154Z90_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0x00; // white is 0x00 in buffer
      _writeData(~data); // white is 0xFF on device
    }
  }
  _writeCommand(0x26);
  for (int16_t y1 = ys; y1 <= yse; y1++)
  {
    for (int16_t x1 = xss_d8; x1 < xse_d8; x1++)
    {
      uint16_t idx = y1 * (GxGDEH0154Z90_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_red_buffer)) ? _red_buffer[idx] : 0x00; // white is 0x00 in buffer
      _writeData(data); // white is 0x00 on device
    }
  }
}

void GxGDEH0154Z90::powerDown()
{
  _using_partial_mode = false; // force _wakeUp()
  _PowerOff();
}

uint16_t GxGDEH0154Z90::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye)
{
  x &= 0xFFF8; // byte boundary
  xe = (xe - 1) | 0x0007; // byte boundary - 1
  _writeCommand(0x44);
  _writeData(x / 8);
  _writeData(xe / 8);
  _writeCommand(0x45);
  _writeData(y % 256);
  _writeData(y / 256);
  _writeData(ye % 256);
  _writeData(ye / 256);
  _writeCommand(0x4E);
  _writeData(x / 8);
  _writeCommand(0x4F);
  _writeData(y % 256);
  _writeData(y / 256);
  return (8 + xe - x) / 8; // number of bytes to transfer per line
}

void GxGDEH0154Z90::_writeCommand(uint8_t command)
{
  IO.writeCommandTransaction(command);
}

void GxGDEH0154Z90::_writeData(uint8_t data)
{
  IO.writeDataTransaction(data);
}

void GxGDEH0154Z90::_waitWhileBusy(const char* comment)
{
  unsigned long start = micros();
  while (1)
  {
    if (digitalRead(_busy) == 0) break;
    delay(1);
    if (micros() - start > 20000000) // >14.9s !
    {
      if (_diag_enabled) Serial.println("Busy Timeout!");
      break;
    }
  }
  if (comment)
  {
#if !defined(DISABLE_DIAGNOSTIC_OUTPUT)
    if (_diag_enabled)
    {
      unsigned long elapsed = micros() - start;
      Serial.print(comment);
      Serial.print(" : ");
      Serial.println(elapsed);
    }
#endif
  }
  (void) start;
}

void GxGDEH0154Z90::_PowerOn()
{
  if (!_power_is_on)
  {
    _writeCommand(0x22);
    _writeData(0xc0);
    _writeCommand(0x20);
    _waitWhileBusy("_PowerOn");
  }
  _power_is_on = true;
}

void GxGDEH0154Z90::_PowerOff()
{
  _writeCommand(0x22);
  _writeData(0xc3);
  _writeCommand(0x20);
  _waitWhileBusy("_PowerOff");
  _power_is_on = false;
}

void GxGDEH0154Z90::_InitDisplay()
{
  if (_hibernating && (_rst >= 0))
  {
    digitalWrite(_rst, 0);
    delay(10);
    digitalWrite(_rst, 1);
    delay(10);
  }
  _writeCommand(0x12);  //SWRESET
  _waitWhileBusy(0);
  _writeCommand(0x01); //Driver output control
  _writeData(0xC7);
  _writeData(0x00);
  _writeData(0x00);
  _writeCommand(0x11); //data entry mode
  _writeData(0x03);
  _writeCommand(0x3C); //BorderWavefrom
  _writeData(0x05);
  _writeCommand(0x18); //Read built-in temperature sensor
  _writeData(0x80);
  _setPartialRamArea(0, 0, GxGDEH0154Z90_WIDTH - 1, GxGDEH0154Z90_HEIGHT - 1);
}

void GxGDEH0154Z90::_Init_Full()
{
  _InitDisplay();
  _PowerOn();
}

void GxGDEH0154Z90::_Init_Part()
{
  _InitDisplay();
  _PowerOn();
}

void GxGDEH0154Z90::_Update_Full()
{
  _writeCommand(0x22); //Display Update Control
  _writeData(0xF7);
  _writeCommand(0x20);  //Activate Display Update Sequence
  _waitWhileBusy("_Update_Full");
}

void GxGDEH0154Z90::_Update_Part()
{
  _writeCommand(0x22); //Display Update Control
  _writeData(0xF7);
  _writeCommand(0x20);  //Activate Display Update Sequence
  _waitWhileBusy("_Update_Part");
}

void GxGDEH0154Z90::drawPaged(void (*drawCallback)(void))
{
  if (_current_page != -1) return;
  _Init_Full();
  _writeCommand(0x24);
  for (_current_page = 0; _current_page < GxGDEH0154Z90_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback();
    for (int16_t y1 = 0; y1 < GxGDEH0154Z90_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEH0154Z90_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEH0154Z90_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0x00; // white is 0x00 in buffer
        _writeData(~data); // white is 0xFF on device
      }
    }
  }
  _writeCommand(0x26);
  for (_current_page = 0; _current_page < GxGDEH0154Z90_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback();
    for (int16_t y1 = 0; y1 < GxGDEH0154Z90_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEH0154Z90_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEH0154Z90_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_red_buffer)) ? _red_buffer[idx] : 0x00; // white is 0x00 in buffer
        _writeData(data); // white is 0x00 on device
      }
    }
  }
  _current_page = -1;
  _Update_Full();
  _PowerOff();
}

void GxGDEH0154Z90::drawPaged(void (*drawCallback)(uint32_t), uint32_t p)
{
  if (_current_page != -1) return;
  _Init_Full();
  _writeCommand(0x24);
  for (_current_page = 0; _current_page < GxGDEH0154Z90_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p);
    for (int16_t y1 = 0; y1 < GxGDEH0154Z90_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEH0154Z90_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEH0154Z90_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0x00; // white is 0x00 in buffer
        _writeData(~data); // white is 0xFF on device
      }
    }
  }
  _writeCommand(0x26);
  for (_current_page = 0; _current_page < GxGDEH0154Z90_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p);
    for (int16_t y1 = 0; y1 < GxGDEH0154Z90_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEH0154Z90_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEH0154Z90_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_red_buffer)) ? _red_buffer[idx] : 0x00; // white is 0x00 in buffer
        _writeData(data); // white is 0x00 on device
      }
    }
  }
  _current_page = -1;
  _Update_Full();
  _PowerOff();
}

void GxGDEH0154Z90::drawPaged(void (*drawCallback)(const void*), const void* p)
{
  if (_current_page != -1) return;
  _Init_Full();
  _writeCommand(0x24);
  for (_current_page = 0; _current_page < GxGDEH0154Z90_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p);
    for (int16_t y1 = 0; y1 < GxGDEH0154Z90_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEH0154Z90_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEH0154Z90_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0x00; // white is 0x00 in buffer
        _writeData(~data); // white is 0xFF on device
      }
    }
  }
  _writeCommand(0x26);
  for (_current_page = 0; _current_page < GxGDEH0154Z90_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p);
    for (int16_t y1 = 0; y1 < GxGDEH0154Z90_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEH0154Z90_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEH0154Z90_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_red_buffer)) ? _red_buffer[idx] : 0x00; // white is 0x00 in buffer
        _writeData(data); // white is 0x00 on device
      }
    }
  }
  _current_page = -1;
  _Update_Full();
  _PowerOff();
}

void GxGDEH0154Z90::drawPaged(void (*drawCallback)(const void*, const void*), const void* p1, const void* p2)
{
  if (_current_page != -1) return;
  _Init_Full();
  _writeCommand(0x24);
  for (_current_page = 0; _current_page < GxGDEH0154Z90_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p1, p2);
    for (int16_t y1 = 0; y1 < GxGDEH0154Z90_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEH0154Z90_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEH0154Z90_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0x00; // white is 0x00 in buffer
        _writeData(~data); // white is 0xFF on device
      }
    }
  }
  _writeCommand(0x26);
  for (_current_page = 0; _current_page < GxGDEH0154Z90_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p1, p2);
    for (int16_t y1 = 0; y1 < GxGDEH0154Z90_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEH0154Z90_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEH0154Z90_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_red_buffer)) ? _red_buffer[idx] : 0x00; // white is 0x00 in buffer
        _writeData(data); // white is 0x00 on device
      }
    }
  }
  _current_page = -1;
  _Update_Full();
  _PowerOff();
}

void GxGDEH0154Z90::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = GxGDEH0154Z90_WIDTH - x - w;
      break;
    case 2:
      x = GxGDEH0154Z90_WIDTH - x - w;
      y = GxGDEH0154Z90_HEIGHT - y - h;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = GxGDEH0154Z90_HEIGHT - y - h;
      break;
  }
}

void GxGDEH0154Z90::drawPagedToWindow(void (*drawCallback)(void), uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  _Init_Part();
  for (_current_page = 0; _current_page < GxGDEH0154Z90_PAGES; _current_page++)
  {
    uint16_t yds = gx_uint16_max(y, _current_page * GxGDEH0154Z90_PAGE_HEIGHT);
    uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEH0154Z90_PAGE_HEIGHT);
    if (yde > yds)
    {
      fillScreen(GxEPD_WHITE);
      drawCallback();
      uint16_t ys = yds % GxGDEH0154Z90_PAGE_HEIGHT;
      _writeToWindow(x, ys, x, yds, w, yde - yds, false);
    }
  }
  _current_page = -1;
  _Update_Part();
  delay(GxGDEH0154Z90_PU_DELAY); // don't stress this display
}

void GxGDEH0154Z90::drawPagedToWindow(void (*drawCallback)(uint32_t), uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t p)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  _Init_Part();
  for (_current_page = 0; _current_page < GxGDEH0154Z90_PAGES; _current_page++)
  {
    uint16_t yds = gx_uint16_max(y, _current_page * GxGDEH0154Z90_PAGE_HEIGHT);
    uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEH0154Z90_PAGE_HEIGHT);
    if (yde > yds)
    {
      fillScreen(GxEPD_WHITE);
      drawCallback(p);
      uint16_t ys = yds % GxGDEH0154Z90_PAGE_HEIGHT;
      _writeToWindow(x, ys, x, yds, w, yde - yds, false);
    }
  }
  _current_page = -1;
  _Update_Part();
  delay(GxGDEH0154Z90_PU_DELAY); // don't stress this display
}

void GxGDEH0154Z90::drawPagedToWindow(void (*drawCallback)(const void*), uint16_t x, uint16_t y, uint16_t w, uint16_t h, const void* p)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  _Init_Part();
  _using_partial_mode = true;
  for (_current_page = 0; _current_page < GxGDEH0154Z90_PAGES; _current_page++)
  {
    uint16_t yds = gx_uint16_max(y, _current_page * GxGDEH0154Z90_PAGE_HEIGHT);
    uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEH0154Z90_PAGE_HEIGHT);
    if (yde > yds)
    {
      fillScreen(GxEPD_WHITE);
      drawCallback(p);
      uint16_t ys = yds % GxGDEH0154Z90_PAGE_HEIGHT;
      _writeToWindow(x, ys, x, yds, w, yde - yds, false);
    }
  }
  _current_page = -1;
  _Update_Part();
  delay(GxGDEH0154Z90_PU_DELAY); // don't stress this display
}

void GxGDEH0154Z90::drawPagedToWindow(void (*drawCallback)(const void*, const void*), uint16_t x, uint16_t y, uint16_t w, uint16_t h, const void* p1, const void* p2)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  _Init_Part();
  _using_partial_mode = true;
  for (_current_page = 0; _current_page < GxGDEH0154Z90_PAGES; _current_page++)
  {
    uint16_t yds = gx_uint16_max(y, _current_page * GxGDEH0154Z90_PAGE_HEIGHT);
    uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEH0154Z90_PAGE_HEIGHT);
    if (yde > yds)
    {
      fillScreen(GxEPD_WHITE);
      drawCallback(p1, p2);
      uint16_t ys = yds % GxGDEH0154Z90_PAGE_HEIGHT;
      _writeToWindow(x, ys, x, yds, w, yde - yds, false);
    }
  }
  _current_page = -1;
  _Update_Part();
  delay(GxGDEH0154Z90_PU_DELAY); // don't stress this display
}

void GxGDEH0154Z90::drawCornerTest(uint8_t em)
{
  if (_current_page != -1) return;
  _Init_Full();
  _writeCommand(0x24);
  for (uint32_t y = 0; y < GxGDEH0154Z90_HEIGHT; y++)
  {
    for (uint32_t x = 0; x < GxGDEH0154Z90_WIDTH / 8; x++)
    {
      uint8_t data = 0xFF; // white is 0xFF on device
      if ((x < 1) && (y < 8)) data = 0x00;
      if ((x > GxGDEH0154Z90_WIDTH / 8 - 3) && (y < 16)) data = 0x00;
      if ((x > GxGDEH0154Z90_WIDTH / 8 - 4) && (y > GxGDEH0154Z90_HEIGHT - 25)) data = 0x00;
      if ((x < 4) && (y > GxGDEH0154Z90_HEIGHT - 33)) data = 0x00;
      _writeData(data);
    }
  }
  _writeCommand(0x26);
  for (uint32_t i = 0; i < GxGDEH0154Z90_BUFFER_SIZE; i++)
  {
    _writeData(0x00); // white is 0x00 on device
  }
  _Update_Full();
  _PowerOff();
}
