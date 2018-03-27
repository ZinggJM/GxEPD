// class GxGDEW042T2_BK : Display class for GDEW042T2 e-Paper from Dalian Good Display Co., Ltd.: www.good-display.com
//
// based on Demo Example from Good Display, available here: http://www.good-display.com/download_detail/downloadsId=515.html
// Controller: IL0398 : http://www.good-display.com/download_detail/downloadsId=537.html
//
// Author : J-M Zingg
//
// Version : see library.properties
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// Library: https://github.com/ZinggJM/GxEPD

// IMPORTANT NOTE: This Fast Partial Update variant works with an experimental partial update waveform table
//                 Side effects and life expectancy with this LUT are unknown, as it is NOT from the manufacturer!

#include "GxGDEW042T2_BK.h"

//#define DISABLE_DIAGNOSTIC_OUTPUT

#define GxGDEW042T2_BK_BUSY_TIMEOUT 10000000

#if defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

GxGDEW042T2_BK::GxGDEW042T2_BK(GxIO& io, int8_t rst, int8_t busy)
  : GxEPD(GxGDEW042T2_BK_WIDTH, GxGDEW042T2_BK_HEIGHT), IO(io),
    _current_page(-1), _initial(true), _using_partial_mode(false), _diag_enabled(false),
    _rst(rst), _busy(busy)
{
}

void GxGDEW042T2_BK::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GxGDEW042T2_BK_WIDTH - x - 1;
      break;
    case 2:
      x = GxGDEW042T2_BK_WIDTH - x - 1;
      y = GxGDEW042T2_BK_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GxGDEW042T2_BK_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GxGDEW042T2_BK_WIDTH / 8;
  if (_current_page < 0)
  {
    if (i >= sizeof(_buffer)) return;
  }
  else
  {
    y -= _current_page * GxGDEW042T2_BK_PAGE_HEIGHT;
    if ((y < 0) || (y >= GxGDEW042T2_BK_PAGE_HEIGHT)) return;
    i = x / 8 + y * GxGDEW042T2_BK_WIDTH / 8;
  }

  if (!color)
    _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
  else
    _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
}

void GxGDEW042T2_BK::init(uint32_t serial_diag_bitrate)
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
  _initial = true;
  _current_page = -1;
  _using_partial_mode = false;
}

void GxGDEW042T2_BK::fillScreen(uint16_t color)
{
  uint8_t data = (color == GxEPD_BLACK) ? 0xFF : 0x00;
  for (uint16_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = data;
  }
}

void GxGDEW042T2_BK::update(void)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  IO.writeCommandTransaction(0x13);
  for (uint32_t i = 0; i < GxGDEW042T2_BK_BUFFER_SIZE; i++)
  {
    uint8_t data = i < sizeof(_buffer) ? _buffer[i] : 0x00;
    IO.writeDataTransaction(~data);
  }
  IO.writeCommandTransaction(0x12);      //display refresh
  _waitWhileBusy("update");
  if (_initial)
  {
    _initial = false;
    IO.writeCommandTransaction(0x13);
    for (uint32_t i = 0; i < GxGDEW042T2_BK_BUFFER_SIZE; i++)
    {
      uint8_t data = i < sizeof(_buffer) ? _buffer[i] : 0x00;
      IO.writeDataTransaction(~data);
    }
    IO.writeCommandTransaction(0x12);      //display refresh
    _waitWhileBusy("update");
  }
  _sleep();
}

void  GxGDEW042T2_BK::drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode)
{
  if (mode & bm_default) mode |= bm_invert;
  drawBitmapBM(bitmap, x, y, w, h, color, mode);
}

void GxGDEW042T2_BK::drawBitmap(const uint8_t *bitmap, uint32_t size, int16_t mode)
{
  if (_current_page != -1) return;
  if (mode & bm_default) mode |= bm_normal;
  if (mode & bm_partial_update)
  {
    _using_partial_mode = true; // remember
    _wakeUp();
    _Init_PartialUpdate();
    // set full screen
    IO.writeCommandTransaction(0x91); // partial in
    _setPartialRamArea(0, 0, GxGDEW042T2_BK_WIDTH - 1, GxGDEW042T2_BK_HEIGHT - 1);
    IO.writeCommandTransaction(0x13);
    for (uint32_t i = 0; i < GxGDEW042T2_BK_BUFFER_SIZE; i++)
    {
      uint8_t data = 0xFF; // white is 0xFF on device
      if (i < size)
      {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
        data = pgm_read_byte(&bitmap[i]);
#else
        data = bitmap[i];
#endif
        if (mode & bm_invert) data = ~data;
      }
      IO.writeDataTransaction(data);
    }
    IO.writeCommandTransaction(0x92); // partial out
    IO.writeCommandTransaction(0x12);      //display refresh
    _waitWhileBusy("drawBitmap");
    // update erase buffer
    IO.writeCommandTransaction(0x91); // partial in
    _setPartialRamArea(0, 0, GxGDEW042T2_BK_WIDTH - 1, GxGDEW042T2_BK_HEIGHT - 1);
    IO.writeCommandTransaction(0x13);
    for (uint32_t i = 0; i < GxGDEW042T2_BK_BUFFER_SIZE; i++)
    {
      uint8_t data = 0xFF; // white is 0xFF on device
      if (i < size)
      {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
        data = pgm_read_byte(&bitmap[i]);
#else
        data = bitmap[i];
#endif
        if (mode & bm_invert) data = ~data;
      }
      IO.writeDataTransaction(data);
    }
    IO.writeCommandTransaction(0x92); // partial out
    _waitWhileBusy("drawBitmap");
  }
  else
  {
    _using_partial_mode = false; // remember
    _wakeUp();
    IO.writeCommandTransaction(0x13);
    for (uint32_t i = 0; i < GxGDEW042T2_BK_BUFFER_SIZE; i++)
    {
      uint8_t data = 0xFF; // white is 0xFF on device
      if (i < size)
      {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
        data = pgm_read_byte(&bitmap[i]);
#else
        data = bitmap[i];
#endif
        if (mode & bm_invert) data = ~data;
      }
      IO.writeDataTransaction(data);
    }
    IO.writeCommandTransaction(0x12);      //display refresh
    _waitWhileBusy("drawBitmap");
    if (_initial)
    {
      _initial = false;
      IO.writeCommandTransaction(0x13);
      for (uint32_t i = 0; i < GxGDEW042T2_BK_BUFFER_SIZE; i++)
      {
        uint8_t data = 0xFF; // white is 0xFF on device
        if (i < size)
        {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
          data = pgm_read_byte(&bitmap[i]);
#else
          data = bitmap[i];
#endif
          if (mode & bm_invert) data = ~data;
        }
        IO.writeDataTransaction(data);
      }
      IO.writeCommandTransaction(0x12);      //display refresh
      _waitWhileBusy("drawBitmap");
    }
    _sleep();
  }
}

void GxGDEW042T2_BK::eraseDisplay(bool using_partial_update)
{
  if (_current_page != -1) return;
  if (using_partial_update)
  {
    _using_partial_mode = true; // remember
    _wakeUp();
    _Init_PartialUpdate();
    // set full screen
    IO.writeCommandTransaction(0x91); // partial in
    _setPartialRamArea(0, 0, GxGDEW042T2_BK_WIDTH - 1, GxGDEW042T2_BK_HEIGHT - 1);
    IO.writeCommandTransaction(0x13);
    for (uint32_t i = 0; i < GxGDEW042T2_BK_BUFFER_SIZE; i++)
    {
      IO.writeDataTransaction(0xFF);
    }
    IO.writeCommandTransaction(0x10);
    for (uint32_t i = 0; i < GxGDEW042T2_BK_BUFFER_SIZE; i++)
    {
      IO.writeDataTransaction(0xFF);
    }
    IO.writeCommandTransaction(0x92); // partial out
    IO.writeCommandTransaction(0x12);      //display refresh
    _waitWhileBusy("eraseDisplay");
  }
  else
  {
    _using_partial_mode = false; // remember
    _wakeUp();
    IO.writeCommandTransaction(0x13);
    for (uint32_t i = 0; i < GxGDEW042T2_BK_BUFFER_SIZE; i++)
    {
      IO.writeDataTransaction(0xFF);
    }
    IO.writeCommandTransaction(0x12);      //display refresh
    _waitWhileBusy("eraseDisplay");
    _sleep();
  }
}

void GxGDEW042T2_BK::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  if (_current_page != -1) return;
  if (using_rotation)
  {
    switch (getRotation())
    {
      case 1:
        swap(x, y);
        swap(w, h);
        x = GxGDEW042T2_BK_WIDTH - x - w - 1;
        break;
      case 2:
        x = GxGDEW042T2_BK_WIDTH - x - w - 1;
        y = GxGDEW042T2_BK_HEIGHT - y - h - 1;
        break;
      case 3:
        swap(x, y);
        swap(w, h);
        y = GxGDEW042T2_BK_HEIGHT - y  - h - 1;
        break;
    }
  }
  //fillScreen(0x0);
  if (x >= GxGDEW042T2_BK_WIDTH) return;
  if (y >= GxGDEW042T2_BK_HEIGHT) return;
  // x &= 0xFFF8; // byte boundary, not here, use encompassing rectangle
  uint16_t xe = gx_uint16_min(GxGDEW042T2_BK_WIDTH, x + w) - 1;
  uint16_t ye = gx_uint16_min(GxGDEW042T2_BK_HEIGHT, y + h) - 1;
  // x &= 0xFFF8; // byte boundary, not needed here
  uint16_t xs_bx = x / 8;
  uint16_t xe_bx = (xe + 7) / 8;
  if (!_using_partial_mode) _wakeUp();
  _using_partial_mode = true;
  _Init_PartialUpdate();
  IO.writeCommandTransaction(0x91); // partial in
  _setPartialRamArea(x, y, xe, ye);
  IO.writeCommandTransaction(0x13);
  for (int16_t y1 = y; y1 <= ye; y1++)
  {
    for (int16_t x1 = xs_bx; x1 < xe_bx; x1++)
    {
      uint16_t idx = y1 * (GxGDEW042T2_BK_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
      IO.writeDataTransaction(~data);
    }
  }
  IO.writeCommandTransaction(0x92); // partial out
  IO.writeCommandTransaction(0x12); //display refresh
  _waitWhileBusy("updateWindow");
  IO.writeCommandTransaction(0x91); // partial in
  _setPartialRamArea(x, y, xe, ye);
  IO.writeCommandTransaction(0x13);
  for (int16_t y1 = y; y1 <= ye; y1++)
  {
    for (int16_t x1 = xs_bx; x1 < xe_bx; x1++)
    {
      uint16_t idx = y1 * (GxGDEW042T2_BK_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
      IO.writeDataTransaction(~data);
    }
  }
  IO.writeCommandTransaction(0x92); // partial out
}

void GxGDEW042T2_BK::_writeToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h)
{
  //Serial.printf("_writeToWindow(%d, %d, %d, %d, %d, %d)\n", xs, ys, xd, yd, w, h);
  // the screen limits are the hard limits
  if (xs >= GxGDEW042T2_BK_WIDTH) return;
  if (ys >= GxGDEW042T2_BK_HEIGHT) return;
  if (xd >= GxGDEW042T2_BK_WIDTH) return;
  if (yd >= GxGDEW042T2_BK_HEIGHT) return;
  uint16_t xde = gx_uint16_min(GxGDEW042T2_BK_WIDTH, xd + w) - 1;
  uint16_t yde = gx_uint16_min(GxGDEW042T2_BK_HEIGHT, yd + h) - 1;
  // soft limits, must send as many bytes as set by _SetRamArea
  uint16_t yse = ys + yde - yd;
  uint16_t xss_d8 = xs / 8;
  IO.writeCommandTransaction(0x91); // partial in
  uint16_t xse_d8 = xss_d8 + _setPartialRamArea(xd, yd, xde, yde);
  IO.writeCommandTransaction(0x13);
  for (int16_t y1 = ys; y1 <= yse; y1++)
  {
    for (int16_t x1 = xss_d8; x1 < xse_d8; x1++)
    {
      uint16_t idx = y1 * (GxGDEW042T2_BK_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
      IO.writeDataTransaction(~data);
    }
  }
  delay(2);
  IO.writeCommandTransaction(0x92); // partial out
}

void GxGDEW042T2_BK::updateToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h, bool using_rotation)
{
  if (using_rotation)
  {
    switch (getRotation())
    {
      case 1:
        swap(xs, ys);
        swap(xd, yd);
        swap(w, h);
        xs = GxGDEW042T2_BK_WIDTH - xs - w - 1;
        xd = GxGDEW042T2_BK_WIDTH - xd - w - 1;
        break;
      case 2:
        xs = GxGDEW042T2_BK_WIDTH - xs - w - 1;
        ys = GxGDEW042T2_BK_HEIGHT - ys - h - 1;
        xd = GxGDEW042T2_BK_WIDTH - xd - w - 1;
        yd = GxGDEW042T2_BK_HEIGHT - yd - h - 1;
        break;
      case 3:
        swap(xs, ys);
        swap(xd, yd);
        swap(w, h);
        ys = GxGDEW042T2_BK_HEIGHT - ys  - h - 1;
        yd = GxGDEW042T2_BK_HEIGHT - yd  - h - 1;
        break;
    }
  }
  _wakeUp();
  _using_partial_mode = true;
  _Init_PartialUpdate();
  _writeToWindow(xs, ys, xd, yd, w, h);
  IO.writeCommandTransaction(0x12);      //display refresh
  _waitWhileBusy("updateToWindow");
  delay(500); // don't stress this display
}

void GxGDEW042T2_BK::powerDown()
{
  _sleep();
}

uint16_t GxGDEW042T2_BK::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye)
{
  x &= 0xFFF8; // byte boundary
  xe = (xe - 1) | 0x0007; // byte boundary - 1
  IO.writeCommandTransaction(0x90); // partial window
  IO.writeDataTransaction(x / 256);
  IO.writeDataTransaction(x % 256);
  IO.writeDataTransaction(xe / 256);
  IO.writeDataTransaction(xe % 256);
  IO.writeDataTransaction(y / 256);
  IO.writeDataTransaction(y % 256);
  IO.writeDataTransaction(ye / 256);
  IO.writeDataTransaction(ye % 256);
  IO.writeDataTransaction(0x01); // don't see any difference
  //IO.writeDataTransaction(0x00); // don't see any difference
  return (7 + xe - x) / 8; // number of bytes to transfer
}

void GxGDEW042T2_BK::_waitWhileBusy(const char* comment)
{
  unsigned long start = micros();
  while (1)
  { //=0 BUSY
    if (digitalRead(_busy) == 1) break;
    delay(1);
    if (micros() - start > GxGDEW042T2_BK_BUSY_TIMEOUT)
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

void GxGDEW042T2_BK::_wakeUp(void)
{
  if (_rst >= 0)
  {
    digitalWrite(_rst, 0);
    delay(10);
    digitalWrite(_rst, 1);
    delay(10);
  }
  IO.writeCommandTransaction(0x06); // boost
  IO.writeDataTransaction(0x17);
  IO.writeDataTransaction(0x17);
  IO.writeDataTransaction(0x17);
  IO.writeCommandTransaction(0x04);
  _waitWhileBusy("Power On");
  //IO.writeCommandTransaction(0x00);
  //IO.writeDataTransaction(0x1f); // LUT from OTP Pixel with B/W.
  _Init_FullUpdate();
}

void GxGDEW042T2_BK::_sleep(void)
{
  IO.writeCommandTransaction(0x50); // border floating
  IO.writeDataTransaction(0x17);
  IO.writeCommandTransaction(0x02); // power off
  _waitWhileBusy("Power Off");
  if (_rst >= 0)
  {
    IO.writeCommandTransaction(0x07); // deep sleep
    IO.writeDataTransaction(0xA5);
  }
}

void GxGDEW042T2_BK::drawPaged(void (*drawCallback)(void))
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  IO.writeCommandTransaction(0x13);
  for (_current_page = 0; _current_page < GxGDEW042T2_BK_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback();
    for (int16_t y1 = 0; y1 < GxGDEW042T2_BK_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW042T2_BK_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW042T2_BK_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
        IO.writeDataTransaction(~data);
      }
    }
  }
  _current_page = -1;
  IO.writeCommandTransaction(0x12);      //display refresh
  _waitWhileBusy("drawPaged");
  _sleep();
}

void GxGDEW042T2_BK::drawPaged(void (*drawCallback)(uint32_t), uint32_t p)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  IO.writeCommandTransaction(0x13);
  for (_current_page = 0; _current_page < GxGDEW042T2_BK_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p);
    for (int16_t y1 = 0; y1 < GxGDEW042T2_BK_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW042T2_BK_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW042T2_BK_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
        IO.writeDataTransaction(~data);
      }
    }
  }
  _current_page = -1;
  IO.writeCommandTransaction(0x12);      //display refresh
  _waitWhileBusy("drawPaged");
  _sleep();
}

void GxGDEW042T2_BK::drawPaged(void (*drawCallback)(const void*), const void* p)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  IO.writeCommandTransaction(0x13);
  for (_current_page = 0; _current_page < GxGDEW042T2_BK_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p);
    for (int16_t y1 = 0; y1 < GxGDEW042T2_BK_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW042T2_BK_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW042T2_BK_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
        IO.writeDataTransaction(~data);
      }
    }
  }
  _current_page = -1;
  IO.writeCommandTransaction(0x12);      //display refresh
  _waitWhileBusy("drawPaged");
  _sleep();
}

void GxGDEW042T2_BK::drawPaged(void (*drawCallback)(const void*, const void*), const void* p1, const void* p2)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  IO.writeCommandTransaction(0x13);
  for (_current_page = 0; _current_page < GxGDEW042T2_BK_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p1, p2);
    for (int16_t y1 = 0; y1 < GxGDEW042T2_BK_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW042T2_BK_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW042T2_BK_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
        IO.writeDataTransaction(~data);
      }
    }
  }
  _current_page = -1;
  IO.writeCommandTransaction(0x12);      //display refresh
  _waitWhileBusy("drawPaged");
  _sleep();
}

void GxGDEW042T2_BK::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = GxGDEW042T2_BK_WIDTH - x - w - 1;
      break;
    case 2:
      x = GxGDEW042T2_BK_WIDTH - x - w - 1;
      y = GxGDEW042T2_BK_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = GxGDEW042T2_BK_HEIGHT - y - h - 1;
      break;
  }
}

void GxGDEW042T2_BK::drawPagedToWindow(void (*drawCallback)(void), uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  if (!_using_partial_mode)
  {
    eraseDisplay(false);
    eraseDisplay(true);
  }
  _using_partial_mode = true;
  for (_current_page = 0; _current_page < GxGDEW042T2_BK_PAGES; _current_page++)
  {
    uint16_t yds = gx_uint16_max(y, _current_page * GxGDEW042T2_BK_PAGE_HEIGHT);
    uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEW042T2_BK_PAGE_HEIGHT);
    if (yde > yds)
    {
      fillScreen(GxEPD_WHITE);
      drawCallback();
      uint16_t ys = yds % GxGDEW042T2_BK_PAGE_HEIGHT;
      _writeToWindow(x, ys, x, yds, w, yde - yds);
    }
  }
  IO.writeCommandTransaction(0x12); //display refresh
  delay(2);
  _waitWhileBusy("updateToWindow");
  // update erase buffer
  for (_current_page = 0; _current_page < GxGDEW042T2_BK_PAGES; _current_page++)
  {
    uint16_t yds = gx_uint16_max(y, _current_page * GxGDEW042T2_BK_PAGE_HEIGHT);
    uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEW042T2_BK_PAGE_HEIGHT);
    if (yde > yds)
    {
      fillScreen(GxEPD_WHITE);
      drawCallback();
      uint16_t ys = yds % GxGDEW042T2_BK_PAGE_HEIGHT;
      _writeToWindow(x, ys, x, yds, w, yde - yds);
    }
  }
  delay(2);
  _waitWhileBusy("updateToWindow");
  _current_page = -1;
}

void GxGDEW042T2_BK::drawPagedToWindow(void (*drawCallback)(uint32_t), uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t p)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  if (!_using_partial_mode)
  {
    eraseDisplay(false);
    eraseDisplay(true);
  }
  _using_partial_mode = true;
  for (_current_page = 0; _current_page < GxGDEW042T2_BK_PAGES; _current_page++)
  {
    uint16_t yds = gx_uint16_max(y, _current_page * GxGDEW042T2_BK_PAGE_HEIGHT);
    uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEW042T2_BK_PAGE_HEIGHT);
    if (yde > yds)
    {
      fillScreen(GxEPD_WHITE);
      drawCallback(p);
      //fillScreen(p);
      uint16_t ys = yds % GxGDEW042T2_BK_PAGE_HEIGHT;
      _writeToWindow(x, ys, x, yds, w, yde - yds);
    }
  }
  IO.writeCommandTransaction(0x12); //display refresh
  delay(2);
  _waitWhileBusy("updateToWindow");
  // update erase buffer
  for (_current_page = 0; _current_page < GxGDEW042T2_BK_PAGES; _current_page++)
  {
    uint16_t yds = gx_uint16_max(y, _current_page * GxGDEW042T2_BK_PAGE_HEIGHT);
    uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEW042T2_BK_PAGE_HEIGHT);
    if (yde > yds)
    {
      fillScreen(GxEPD_WHITE);
      drawCallback(p);
      //fillScreen(p);
      uint16_t ys = yds % GxGDEW042T2_BK_PAGE_HEIGHT;
      _writeToWindow(x, ys, x, yds, w, yde - yds);
    }
  }
  delay(2);
  _waitWhileBusy("updateToWindow");
  _current_page = -1;
}

void GxGDEW042T2_BK::drawPagedToWindow(void (*drawCallback)(const void*), uint16_t x, uint16_t y, uint16_t w, uint16_t h, const void* p)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  if (!_using_partial_mode)
  {
    eraseDisplay(false);
    eraseDisplay(true);
  }
  _using_partial_mode = true;
  for (_current_page = 0; _current_page < GxGDEW042T2_BK_PAGES; _current_page++)
  {
    uint16_t yds = gx_uint16_max(y, _current_page * GxGDEW042T2_BK_PAGE_HEIGHT);
    uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEW042T2_BK_PAGE_HEIGHT);
    if (yde > yds)
    {
      fillScreen(GxEPD_WHITE);
      drawCallback(p);
      uint16_t ys = yds % GxGDEW042T2_BK_PAGE_HEIGHT;
      _writeToWindow(x, ys, x, yds, w, yde - yds);
    }
  }
  IO.writeCommandTransaction(0x12); //display refresh
  delay(2);
  _waitWhileBusy("updateToWindow");
  // update erase buffer
  for (_current_page = 0; _current_page < GxGDEW042T2_BK_PAGES; _current_page++)
  {
    uint16_t yds = gx_uint16_max(y, _current_page * GxGDEW042T2_BK_PAGE_HEIGHT);
    uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEW042T2_BK_PAGE_HEIGHT);
    if (yde > yds)
    {
      fillScreen(GxEPD_WHITE);
      drawCallback(p);
      uint16_t ys = yds % GxGDEW042T2_BK_PAGE_HEIGHT;
      _writeToWindow(x, ys, x, yds, w, yde - yds);
    }
  }
  delay(2);
  _waitWhileBusy("updateToWindow");
  _current_page = -1;
}

void GxGDEW042T2_BK::drawPagedToWindow(void (*drawCallback)(const void*, const void*), uint16_t x, uint16_t y, uint16_t w, uint16_t h, const void* p1, const void* p2)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  if (!_using_partial_mode)
  {
    eraseDisplay(false);
    eraseDisplay(true);
  }
  _using_partial_mode = true;
  for (_current_page = 0; _current_page < GxGDEW042T2_BK_PAGES; _current_page++)
  {
    uint16_t yds = gx_uint16_max(y, _current_page * GxGDEW042T2_BK_PAGE_HEIGHT);
    uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEW042T2_BK_PAGE_HEIGHT);
    if (yde > yds)
    {
      fillScreen(GxEPD_WHITE);
      drawCallback(p1, p2);
      uint16_t ys = yds % GxGDEW042T2_BK_PAGE_HEIGHT;
      _writeToWindow(x, ys, x, yds, w, yde - yds);
    }
  }
  IO.writeCommandTransaction(0x12); //display refresh
  delay(2);
  _waitWhileBusy("updateToWindow");
  // update erase buffer
  for (_current_page = 0; _current_page < GxGDEW042T2_BK_PAGES; _current_page++)
  {
    uint16_t yds = gx_uint16_max(y, _current_page * GxGDEW042T2_BK_PAGE_HEIGHT);
    uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEW042T2_BK_PAGE_HEIGHT);
    if (yde > yds)
    {
      fillScreen(GxEPD_WHITE);
      drawCallback(p1, p2);
      uint16_t ys = yds % GxGDEW042T2_BK_PAGE_HEIGHT;
      _writeToWindow(x, ys, x, yds, w, yde - yds);
    }
  }
  delay(2);
  _waitWhileBusy("updateToWindow");
  _current_page = -1;
}

void GxGDEW042T2_BK::drawCornerTest(uint8_t em)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  IO.writeCommandTransaction(0x13);
  for (uint32_t y = 0; y < GxGDEW042T2_BK_HEIGHT; y++)
  {
    for (uint32_t x = 0; x < GxGDEW042T2_BK_WIDTH / 8; x++)
    {
      uint8_t data = 0xFF;
      if ((x < 1) && (y < 8)) data = 0x00;
      if ((x > GxGDEW042T2_BK_WIDTH / 8 - 3) && (y < 16)) data = 0x00;
      if ((x > GxGDEW042T2_BK_WIDTH / 8 - 4) && (y > GxGDEW042T2_BK_HEIGHT - 25)) data = 0x00;
      if ((x < 4) && (y > GxGDEW042T2_BK_HEIGHT - 33)) data = 0x00;
      IO.writeDataTransaction(data);
    }
  }
  IO.writeCommandTransaction(0x12);      //display refresh
  _waitWhileBusy("drawCornerTest");
  _sleep();
}

//#define USE_OTP_FULL_ONLY
//#define USE_REG_FULL_ONLY
//#define USE_OTP_FULL_FPU_PART
//#define USE_REG_FULL_FPU_PART
#define USE_REG_FULL_BK_PART

#if defined(USE_OTP_FULL_ONLY)

void GxGDEW042T2_BK::_Init_FullUpdate()
{
  IO.writeCommandTransaction(0x00);
  IO.writeDataTransaction(0x1f); // LUT from OTP Pixel with B/W.
}

void GxGDEW042T2_BK::_Init_PartialUpdate()
{
  IO.writeCommandTransaction(0x00);
  IO.writeDataTransaction(0x1f); // LUT from OTP Pixel with B/W.
}

#endif

#if defined(USE_REG_FULL_ONLY)

void GxGDEW042T2_BK::_Init_FullUpdate()
{
  IO.writeCommandTransaction(0x00);
  IO.writeDataTransaction(0x3F); //300x400 B/W mode, LUT set by register
  unsigned int count;
  IO.writeCommandTransaction(0x20); //vcom
  for (count = 0; count < 44; count++)
  {
    IO.writeDataTransaction(lut_vcom0[count]);
  }
  IO.writeCommandTransaction(0x21); //ww --
  for (count = 0; count < 42; count++)
  {
    IO.writeDataTransaction(lut_ww[count]);
  }
  IO.writeCommandTransaction(0x22); //bw r
  for (count = 0; count < 42; count++)
  {
    IO.writeDataTransaction(lut_bw[count]);
  }
  IO.writeCommandTransaction(0x23); //wb w
  for (count = 0; count < 42; count++)
  {
    IO.writeDataTransaction(lut_wb[count]);
  }
  IO.writeCommandTransaction(0x24); //bb b
  for (count = 0; count < 42; count++)
  {
    IO.writeDataTransaction(lut_bb[count]);
  }
}

void GxGDEW042T2_BK::_Init_PartialUpdate()
{
  _Init_FullUpdate();
}

#endif

#if defined(USE_OTP_FULL_FPU_PART)

void GxGDEW042T2_BK::_Init_FullUpdate()
{
  IO.writeCommandTransaction(0x00);
  IO.writeDataTransaction(0x1f); // LUT from OTP Pixel with B/W.
}

void GxGDEW042T2_BK::_Init_PartialUpdate()
{
  IO.writeCommandTransaction(0x00);
  IO.writeDataTransaction(0x3F); //300x400 B/W mode, LUT set by register
  unsigned int count;
  IO.writeCommandTransaction(0x20); //vcom
  for (count = 0; count < 44; count++)
  {
    IO.writeDataTransaction(lut_vcom0_partial[count]);
  }
  IO.writeCommandTransaction(0x21); //ww --
  for (count = 0; count < 42; count++)
  {
    IO.writeDataTransaction(lut_ww_partial[count]);
  }
  IO.writeCommandTransaction(0x22); //bw r
  for (count = 0; count < 42; count++)
  {
    IO.writeDataTransaction(lut_bw_partial[count]);
  }
  IO.writeCommandTransaction(0x23); //wb w
  for (count = 0; count < 42; count++)
  {
    IO.writeDataTransaction(lut_wb_partial[count]);
  }
  IO.writeCommandTransaction(0x24); //bb b
  for (count = 0; count < 42; count++)
  {
    IO.writeDataTransaction(lut_bb_partial[count]);
  }
}

#endif

#if defined(USE_REG_FULL_FPU_PART)

void GxGDEW042T2_BK::_Init_FullUpdate()
{
  IO.writeCommandTransaction(0x00);
  IO.writeDataTransaction(0x3F); //300x400 B/W mode, LUT set by register
  unsigned int count;
  IO.writeCommandTransaction(0x20); //vcom
  for (count = 0; count < 44; count++)
  {
    IO.writeDataTransaction(lut_vcom0[count]);
  }
  IO.writeCommandTransaction(0x21); //ww --
  for (count = 0; count < 42; count++)
  {
    IO.writeDataTransaction(lut_ww[count]);
  }
  IO.writeCommandTransaction(0x22); //bw r
  for (count = 0; count < 42; count++)
  {
    IO.writeDataTransaction(lut_bw[count]);
  }
  IO.writeCommandTransaction(0x23); //wb w
  for (count = 0; count < 42; count++)
  {
    IO.writeDataTransaction(lut_wb[count]);
  }
  IO.writeCommandTransaction(0x24); //bb b
  for (count = 0; count < 42; count++)
  {
    IO.writeDataTransaction(lut_bb[count]);
  }
}

void GxGDEW042T2_BK::_Init_PartialUpdate()
{
  IO.writeCommandTransaction(0x00);
  IO.writeDataTransaction(0x3F); //300x400 B/W mode, LUT set by register
  unsigned int count;
  IO.writeCommandTransaction(0x20); //vcom
  for (count = 0; count < 44; count++)
  {
    IO.writeDataTransaction(lut_vcom0_partial[count]);
  }
  IO.writeCommandTransaction(0x21); //ww --
  for (count = 0; count < 42; count++)
  {
    IO.writeDataTransaction(lut_ww_partial[count]);
  }
  IO.writeCommandTransaction(0x22); //bw r
  for (count = 0; count < 42; count++)
  {
    IO.writeDataTransaction(lut_bw_partial[count]);
  }
  IO.writeCommandTransaction(0x23); //wb w
  for (count = 0; count < 42; count++)
  {
    IO.writeDataTransaction(lut_wb_partial[count]);
  }
  IO.writeCommandTransaction(0x24); //bb b
  for (count = 0; count < 42; count++)
  {
    IO.writeDataTransaction(lut_bb_partial[count]);
  }
}

#endif

#if defined(USE_REG_FULL_BK_PART)

void GxGDEW042T2_BK::_Init_FullUpdate()
{
  IO.writeCommandTransaction(0x00);
  IO.writeDataTransaction(0x3F); //300x400 B/W mode, LUT set by register
  unsigned int count;
  IO.writeCommandTransaction(0x20); //vcom
  for (count = 0; count < 44; count++)
  {
    IO.writeDataTransaction(lut_vcom0[count]);
  }
  IO.writeCommandTransaction(0x21); //ww --
  for (count = 0; count < 42; count++)
  {
    IO.writeDataTransaction(lut_ww[count]);
  }
  IO.writeCommandTransaction(0x22); //bw r
  for (count = 0; count < 42; count++)
  {
    IO.writeDataTransaction(lut_bw[count]);
  }
  IO.writeCommandTransaction(0x23); //wb w
  for (count = 0; count < 42; count++)
  {
    IO.writeDataTransaction(lut_wb[count]);
  }
  IO.writeCommandTransaction(0x24); //bb b
  for (count = 0; count < 42; count++)
  {
    IO.writeDataTransaction(lut_bb[count]);
  }
}

void GxGDEW042T2_BK::_Init_PartialUpdate()
{
  IO.writeCommandTransaction(0x00);
  IO.writeDataTransaction(0x3F); //300x400 B/W mode, LUT set by register
  unsigned int count;
  IO.writeCommandTransaction(0x20); //vcom
  for (count = 0; count < 44; count++)
  {
    IO.writeDataTransaction(lut_vcom0_quick[count]);
  }
  IO.writeCommandTransaction(0x21); //ww --
  for (count = 0; count < 42; count++)
  {
    IO.writeDataTransaction(lut_ww_quick[count]);
  }
  IO.writeCommandTransaction(0x22); //bw r
  for (count = 0; count < 42; count++)
  {
    IO.writeDataTransaction(lut_bw_quick[count]);
  }
  IO.writeCommandTransaction(0x23); //wb w
  for (count = 0; count < 42; count++)
  {
    IO.writeDataTransaction(lut_wb_quick[count]);
  }
  IO.writeCommandTransaction(0x24); //bb b
  for (count = 0; count < 42; count++)
  {
    IO.writeDataTransaction(lut_bb_quick[count]);
  }
}

#endif

#define TP0A  2 // sustain phase for bb and ww, change phase for bw and wb
#define TP0B 45 // change phase for bw and wb

const unsigned char GxGDEW042T2_BK::lut_vcom0_partial[] =
{
  0x00,
  TP0A, TP0B, 0x01, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char GxGDEW042T2_BK::lut_ww_partial[] =
{
  0x80, // 10 00 00 00
  TP0A, TP0B, 0x01, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char GxGDEW042T2_BK::lut_bw_partial[] =
{
  0xA0, // 10 10 00 00
  TP0A, TP0B, 0x01, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char GxGDEW042T2_BK::lut_wb_partial[] =
{
  0x50, // 01 01 00 00
  TP0A, TP0B, 0x01, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char GxGDEW042T2_BK::lut_bb_partial[] =
{
  0x40, // 01 00 00 00
  TP0A, TP0B, 0x01, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char GxGDEW042T2_BK::lut_vcom0[] =
{
  0x40, 0x17, 0x00, 0x00, 0x00, 0x02,
  0x00, 0x17, 0x17, 0x00, 0x00, 0x02,
  0x00, 0x0A, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x0E, 0x0E, 0x00, 0x00, 0x02,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char GxGDEW042T2_BK::lut_vcom0_quick[] =
{
  0x00, 0x0E, 0x00, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};



const unsigned char GxGDEW042T2_BK::lut_ww[] =
{
  0x40, 0x17, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x17, 0x17, 0x00, 0x00, 0x02,
  0x40, 0x0A, 0x01, 0x00, 0x00, 0x01,
  0xA0, 0x0E, 0x0E, 0x00, 0x00, 0x02,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char GxGDEW042T2_BK::lut_ww_quick[] =
{
  0xA0, 0x0E, 0x00, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};


const unsigned char GxGDEW042T2_BK::lut_bw[] =
{
  0x40, 0x17, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x17, 0x17, 0x00, 0x00, 0x02,
  0x40, 0x0A, 0x01, 0x00, 0x00, 0x01,
  0xA0, 0x0E, 0x0E, 0x00, 0x00, 0x02,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};


const unsigned char GxGDEW042T2_BK::lut_bw_quick[] =
{
  0xA0, 0x0E, 0x00, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char GxGDEW042T2_BK::lut_bb[] =
{
  0x80, 0x17, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x17, 0x17, 0x00, 0x00, 0x02,
  0x80, 0x0A, 0x01, 0x00, 0x00, 0x01,
  0x50, 0x0E, 0x0E, 0x00, 0x00, 0x02,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char GxGDEW042T2_BK::lut_bb_quick[] =
{
  0x50, 0x0E, 0x00, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};


const unsigned char GxGDEW042T2_BK::lut_wb[] =
{
  0x80, 0x17, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x17, 0x17, 0x00, 0x00, 0x02,
  0x80, 0x0A, 0x01, 0x00, 0x00, 0x01,
  0x50, 0x0E, 0x0E, 0x00, 0x00, 0x02,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char GxGDEW042T2_BK::lut_wb_quick[] =
{
  0x50, 0x0E, 0x00, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

