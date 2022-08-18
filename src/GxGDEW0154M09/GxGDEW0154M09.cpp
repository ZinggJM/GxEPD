// class GxGDEW0154M09 : Display class for GDEW0154M09 e-Paper from Dalian Good Display Co. https://www.good-display.com
//
// based on Demo Example from Good Display, available here: http://www.e-paper-display.com/download_detail/downloadsId=597.html
// Panel: GDEW0154M09 : http://www.e-paper-display.com/products_detail/productId=513.html
// Controller : JD79653A : http://www.e-paper-display.com/download_detail/downloadsId%3d943.html
//
// Author : J-M Zingg
//
// Version : see library.properties
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// Library: https://github.com/ZinggJM/GxEPD

#include "GxGDEW0154M09.h"

//#define DISABLE_DIAGNOSTIC_OUTPUT

#if defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

// Partial Update Delay, may have an influence on degradation
#define GxGDEW0154M09_PU_DELAY 100

GxGDEW0154M09::GxGDEW0154M09(GxIO& io, int8_t rst, int8_t busy)
  : GxEPD(GxGDEW0154M09_WIDTH, GxGDEW0154M09_HEIGHT), IO(io),
    _current_page(-1), _power_is_on(false), _using_partial_mode(false), _hibernating(false), _diag_enabled(false),
    _rst(rst), _busy(busy)
{
}

void GxGDEW0154M09::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GxGDEW0154M09_WIDTH - x - 1;
      break;
    case 2:
      x = GxGDEW0154M09_WIDTH - x - 1;
      y = GxGDEW0154M09_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GxGDEW0154M09_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GxGDEW0154M09_WIDTH / 8;
  if (_current_page < 1)
  {
    if (i >= sizeof(_buffer)) return;
  }
  else
  {
    y -= _current_page * GxGDEW0154M09_PAGE_HEIGHT;
    if ((y < 0) || (y >= GxGDEW0154M09_PAGE_HEIGHT)) return;
    i = x / 8 + y * GxGDEW0154M09_WIDTH / 8;
  }

  if (!color)
    _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
  else
    _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
}


void GxGDEW0154M09::init(uint32_t serial_diag_bitrate)
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

void GxGDEW0154M09::fillScreen(uint16_t color)
{
  uint8_t data = (color == GxEPD_BLACK) ? 0xFF : 0x00;
  for (uint16_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = data;
  }
}

void GxGDEW0154M09::update(void)
{
  if (_current_page != -1) return;
  _Init_Full();
  _writeCommand(0x10);
  for (uint32_t i = 0; i < GxGDEW0154M09_BUFFER_SIZE; i++)
  {
    //_writeData(0xFF); // 0xFF is white
    _writeData((i < sizeof(_buffer)) ? ~_buffer[i] : 0xFF);
  }
  _writeCommand(0x13);
  for (uint32_t i = 0; i < GxGDEW0154M09_BUFFER_SIZE; i++)
  {
    _writeData((i < sizeof(_buffer)) ? ~_buffer[i] : 0xFF);
  }
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("update");
  _PowerOff();
}

void  GxGDEW0154M09::drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode)
{
  if (mode & bm_default) mode |= bm_invert;
  drawBitmapBM(bitmap, x, y, w, h, color, mode);
}

void GxGDEW0154M09::drawBitmap(const uint8_t* bitmap, uint32_t size, int16_t mode)
{
  if (_current_page != -1) return;
  if (mode & bm_partial_update)
  {
    if (!_using_partial_mode) _Init_Part();
    _writeCommand(0x91); // partial in
    _setPartialRamArea(0, 0, GxGDEW0154M09_WIDTH - 1, GxGDEW0154M09_HEIGHT - 1);
    _writeCommand(0x13);
    for (uint32_t i = 0; i < GxGDEW0154M09_BUFFER_SIZE; i++)
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
      _writeData(data);
    }
    _writeCommand(0x92); // partial out
    _writeCommand(0x12); //display refresh
    _waitWhileBusy("drawBitmap");
    _writeCommand(0x91); // partial in
    _setPartialRamArea(0, 0, GxGDEW0154M09_WIDTH - 1, GxGDEW0154M09_HEIGHT - 1);
    _writeCommand(0x10);
    for (uint32_t i = 0; i < GxGDEW0154M09_BUFFER_SIZE; i++)
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
      _writeData(data);
    }
    _writeCommand(0x92); // partial out
    delay(GxGDEW0154M09_PU_DELAY); // don't stress this display
  }
  else
  {
    _Init_Full();
    _writeCommand(0x10);
    for (uint32_t i = 0; i < GxGDEW0154M09_BUFFER_SIZE; i++)
    {
      //_writeData(0xFF); // white is 0xFF on device
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
      _writeData(data);
    }
    _writeCommand(0x13);
    for (uint32_t i = 0; i < GxGDEW0154M09_BUFFER_SIZE; i++)
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
      _writeData(data);
    }
    _writeCommand(0x12); //display refresh
    _waitWhileBusy("drawBitmap");
    //_PowerOff();
  }
}

void GxGDEW0154M09::eraseDisplay(bool using_partial_update)
{
  if (_current_page != -1) return;
  if (using_partial_update)
  {
    if (!_using_partial_mode) _Init_Part();
    _writeCommand(0x91); // partial in
    _setPartialRamArea(0, 0, GxGDEW0154M09_WIDTH - 1, GxGDEW0154M09_HEIGHT - 1);
    _writeCommand(0x13);
    for (uint32_t i = 0; i < GxGDEW0154M09_BUFFER_SIZE; i++)
    {
      _writeData(0xFF); // white is 0xFF on device
    }
    _writeCommand(0x12); //display refresh
    _waitWhileBusy("eraseDisplay");
    _writeCommand(0x10);
    for (uint32_t i = 0; i < GxGDEW0154M09_BUFFER_SIZE; i++)
    {
      _writeData(0xFF); // white is 0xFF on device
    }
    _writeCommand(0x92); // partial out
  }
  else
  {
    _Init_Full();
    _writeCommand(0x10);
    for (uint32_t i = 0; i < GxGDEW0154M09_BUFFER_SIZE; i++)
    {
      _writeData(0xFF); // white is 0xFF on device
    }
    _writeCommand(0x13);
    for (uint32_t i = 0; i < GxGDEW0154M09_BUFFER_SIZE; i++)
    {
      _writeData(0xFF); // white is 0xFF on device
    }
    _writeCommand(0x12); //display refresh
    _waitWhileBusy("eraseDisplay");
    _PowerOff();
  }
}

void GxGDEW0154M09::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  if (_current_page != -1) return;
  if (using_rotation) _rotate(x, y, w, h);
  if (x >= GxGDEW0154M09_WIDTH) return;
  if (y >= GxGDEW0154M09_HEIGHT) return;
  uint16_t xe = gx_uint16_min(GxGDEW0154M09_WIDTH, x + w) - 1;
  uint16_t ye = gx_uint16_min(GxGDEW0154M09_HEIGHT, y + h) - 1;
  // x &= 0xFFF8; // byte boundary, not needed here
  uint16_t xs_bx = x / 8;
  uint16_t xe_bx = (xe + 7) / 8;
  if (!_using_partial_mode) _Init_Part();
  _writeCommand(0x91); // partial in
  _setPartialRamArea(x, y, xe, ye);
  _writeCommand(0x13);
  for (int16_t y1 = y; y1 <= ye; y1++)
  {
    for (int16_t x1 = xs_bx; x1 < xe_bx; x1++)
    {
      uint16_t idx = y1 * (GxGDEW0154M09_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00; // white is 0x00 in buffer
      _writeData(~data); // white is 0xFF on device
    }
  }
  _writeCommand(0x12);      //display refresh
  _waitWhileBusy("updateWindow");
  _writeCommand(0x10);
  for (int16_t y1 = y; y1 <= ye; y1++)
  {
    for (int16_t x1 = xs_bx; x1 < xe_bx; x1++)
    {
      uint16_t idx = y1 * (GxGDEW0154M09_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00; // white is 0x00 in buffer
      _writeData(~data); // white is 0xFF on device
    }
  }
  _writeCommand(0x92); // partial out
  delay(GxGDEW0154M09_PU_DELAY); // don't stress this display
}

void GxGDEW0154M09::updateToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h, bool using_rotation)
{
  if (using_rotation)
  {
    switch (getRotation())
    {
      case 1:
        swap(xs, ys);
        swap(xd, yd);
        swap(w, h);
        xs = GxGDEW0154M09_WIDTH - xs - w;
        xd = GxGDEW0154M09_WIDTH - xd - w;
        break;
      case 2:
        xs = GxGDEW0154M09_WIDTH - xs - w;
        ys = GxGDEW0154M09_HEIGHT - ys - h;
        xd = GxGDEW0154M09_WIDTH - xd - w;
        yd = GxGDEW0154M09_HEIGHT - yd - h;
        break;
      case 3:
        swap(xs, ys);
        swap(xd, yd);
        swap(w, h);
        ys = GxGDEW0154M09_HEIGHT - ys  - h;
        yd = GxGDEW0154M09_HEIGHT - yd  - h;
        break;
    }
  }
  if (xs >= GxGDEW0154M09_WIDTH) return;
  if (ys >= GxGDEW0154M09_HEIGHT) return;
  if (xd >= GxGDEW0154M09_WIDTH) return;
  if (yd >= GxGDEW0154M09_HEIGHT) return;
  // the screen limits are the hard limits
  uint16_t xde = gx_uint16_min(GxGDEW0154M09_WIDTH, xd + w) - 1;
  uint16_t yde = gx_uint16_min(GxGDEW0154M09_HEIGHT, yd + h) - 1;
  if (!_using_partial_mode) _Init_Part();
  _writeCommand(0x91); // partial in
  // soft limits, must send as many bytes as set by _SetRamArea
  uint16_t yse = ys + yde - yd;
  uint16_t xss_d8 = xs / 8;
  uint16_t xse_d8 = xss_d8 + _setPartialRamArea(xd, yd, xde, yde);
  _writeCommand(0x13);
  for (int16_t y1 = ys; y1 <= yse; y1++)
  {
    for (int16_t x1 = xss_d8; x1 < xse_d8; x1++)
    {
      uint16_t idx = y1 * (GxGDEW0154M09_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00; // white is 0x00 in buffer
      _writeData(~data); // white is 0xFF on device
    }
  }
  _writeCommand(0x12);      //display refresh
  _waitWhileBusy("updateToWindow");
  _writeCommand(0x10);
  for (int16_t y1 = ys; y1 <= yse; y1++)
  {
    for (int16_t x1 = xss_d8; x1 < xse_d8; x1++)
    {
      uint16_t idx = y1 * (GxGDEW0154M09_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00; // white is 0x00 in buffer
      _writeData(~data); // white is 0xFF on device
    }
  }
  _writeCommand(0x92); // partial out
  delay(GxGDEW0154M09_PU_DELAY); // don't stress this display
}

void GxGDEW0154M09::powerDown()
{
  _using_partial_mode = false; // force _Init_Part()
  _PowerOff();
}

uint16_t GxGDEW0154M09::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye)
{
  x &= 0xFFF8; // byte boundary
  xe = (xe - 1) | 0x0007; // byte boundary - 1
  _writeCommand(0x90); // partial window
  //_writeData(x / 256);
  _writeData(x % 256);
  //_writeData(xe / 256);
  _writeData(xe % 256);
  _writeData(y / 256);
  _writeData(y % 256);
  _writeData(ye / 256);
  _writeData(ye % 256);
  _writeData(0x01); // don't see any difference
  //_writeData(0x00); // don't see any difference
  return (8 + xe - x) / 8; // number of bytes to transfer per line
}

void GxGDEW0154M09::_writeCommand(uint8_t command)
{
  IO.writeCommandTransaction(command);
}

void GxGDEW0154M09::_writeData(uint8_t data)
{
  IO.writeDataTransaction(data);
}

void GxGDEW0154M09::_writeDataPGM(const uint8_t* data, uint16_t n, int16_t fill_with_zeroes)
{
  for (uint16_t i = 0; i < n; i++)
  {
    _writeData(pgm_read_byte(&*data++));
  }
  while (fill_with_zeroes > 0)
  {
    _writeData(0x00);
    fill_with_zeroes--;
  }
}

void GxGDEW0154M09::_waitWhileBusy(const char* comment)
{
  unsigned long start = micros();
  while (1)
  {
    if (digitalRead(_busy) == 1) break;
    delay(1);
    if (micros() - start > 10000000)
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

void GxGDEW0154M09::_PowerOn()
{
  if (!_power_is_on)
  {
    _writeCommand(0x04);
    _waitWhileBusy("_PowerOn");
  }
  _power_is_on = true;
}

void GxGDEW0154M09::_PowerOff()
{
  if (_power_is_on)
  {
    //if (_using_partial_mode) _Update_Part(); // would hang on _powerOn() without
    _writeCommand(0x02); // power off
    _waitWhileBusy("_PowerOff");
    _power_is_on = false;
    _using_partial_mode = false;
  }
}

void GxGDEW0154M09::_InitDisplay()
{
  if (_hibernating && (_rst >= 0))
  {
    digitalWrite(_rst, 0);
    delay(10);
    digitalWrite(_rst, 1);
    delay(10);
  }
 _writeCommand(0x00); // panel setting
  _writeData (0xff);
  _writeData (0x0e);
  _writeCommand(0x01); // power setting
  _writeData(0x03);
  _writeData(0x06); // 16V
  _writeData(0x2A);//
  _writeData(0x2A);//
  _writeCommand(0x4D); // FITIinternal code
  _writeData (0x55);
  _writeCommand(0xaa);
  _writeData (0x0f);
  _writeCommand(0xE9);
  _writeData (0x02);
  _writeCommand(0xb6);
  _writeData (0x11);
  _writeCommand(0xF3);
  _writeData (0x0a);
  _writeCommand(0x06); // boost soft start
  _writeData (0xc7);
  _writeData (0x0c);
  _writeData (0x0c);
  _writeCommand(0x61); // resolution setting
  _writeData (0xc8); // 200
  _writeData (0x00);
  _writeData (0xc8); // 200
  _writeCommand(0x60); // Tcon setting
  _writeData (0x00);
  _writeCommand(0x82); // VCOM DC setting
  _writeData (0x12);
  _writeCommand(0x30); // PLL control
  _writeData (0x3C);   // default 50Hz
  _writeCommand(0X50); // VCOM and data interval
  _writeData(0x97);//
  _writeCommand(0XE3); // power saving register
  _writeData(0x00); // default
}

const unsigned char GxGDEW0154M09::lut_20_vcomDC[] PROGMEM =
{
  0x01, 0x05, 0x05, 0x05, 0x05, 0x01, 0x01,
  0x01, 0x05, 0x05, 0x05, 0x05, 0x01, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char GxGDEW0154M09::lut_21_ww[] PROGMEM =
{
  0x01, 0x45, 0x45, 0x43, 0x44, 0x01, 0x01,
  0x01, 0x87, 0x83, 0x87, 0x06, 0x01, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char GxGDEW0154M09::lut_22_bw[] PROGMEM =
{
  0x01, 0x05, 0x05, 0x45, 0x42, 0x01, 0x01,
  0x01, 0x87, 0x85, 0x85, 0x85, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char GxGDEW0154M09::lut_23_wb[] PROGMEM =
{
  0x01, 0x08, 0x08, 0x82, 0x42, 0x01, 0x01,
  0x01, 0x45, 0x45, 0x45, 0x45, 0x01, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char GxGDEW0154M09::lut_24_bb[] PROGMEM =
{
  0x01, 0x85, 0x85, 0x85, 0x83, 0x01, 0x01,
  0x01, 0x45, 0x45, 0x04, 0x48, 0x01, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char GxGDEW0154M09::lut_20_vcomDC_partial[] PROGMEM =
{
  0x01, 0x04, 0x04, 0x03, 0x01, 0x01, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char GxGDEW0154M09::lut_21_ww_partial[] PROGMEM =
{
  0x01, 0x04, 0x04, 0x03, 0x01, 0x01, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
const unsigned char GxGDEW0154M09::lut_22_bw_partial[] PROGMEM =
{
  0x01, 0x84, 0x84, 0x83, 0x01, 0x01, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char GxGDEW0154M09::lut_23_wb_partial[] PROGMEM =
{
  0x01, 0x44, 0x44, 0x43, 0x01, 0x01, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char GxGDEW0154M09::lut_24_bb_partial[] PROGMEM =
{
  0x01, 0x04, 0x04, 0x03, 0x01, 0x01, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

void GxGDEW0154M09::_Init_Full()
{
  _InitDisplay();
  _writeCommand(0x20);
  _writeDataPGM(lut_20_vcomDC, sizeof(lut_20_vcomDC));
  _writeCommand(0x21);
  _writeDataPGM(lut_21_ww, sizeof(lut_21_ww));
  _writeCommand(0x22);
  _writeDataPGM(lut_22_bw, sizeof(lut_22_bw));
  _writeCommand(0x23);
  _writeDataPGM(lut_23_wb, sizeof(lut_23_wb));
  _writeCommand(0x24);
  _writeDataPGM(lut_24_bb, sizeof(lut_24_bb));
  _PowerOn();
  _using_partial_mode = false;
}

void GxGDEW0154M09::_Init_Part()
{
  _InitDisplay();
  _writeCommand(0x20);
  _writeDataPGM(lut_20_vcomDC_partial, sizeof(lut_20_vcomDC_partial));
  _writeCommand(0x21);
  _writeDataPGM(lut_21_ww_partial, sizeof(lut_21_ww_partial));
  _writeCommand(0x22);
  _writeDataPGM(lut_22_bw_partial, sizeof(lut_22_bw_partial));
  _writeCommand(0x23);
  _writeDataPGM(lut_23_wb_partial, sizeof(lut_23_wb_partial));
  _writeCommand(0x24);
  _writeDataPGM(lut_24_bb_partial, sizeof(lut_24_bb_partial));
  _PowerOn();
  _using_partial_mode = true;
}

void GxGDEW0154M09::_Update_Full()
{
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("_Update_Full");
}

void GxGDEW0154M09::_Update_Part()
{
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("_Update_Part");
}

void GxGDEW0154M09::drawPaged(void (*drawCallback)(void))
{
  if (_current_page != -1) return;
  _Init_Full();
  _writeCommand(0x10);
  for (uint32_t i = 0; i < GxGDEW0154M09_BUFFER_SIZE; i++)
  {
    _writeData(0xFF); // 0xFF is white
  }
  _writeCommand(0x13);
  for (_current_page = 0; _current_page < GxGDEW0154M09_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback();
    for (int16_t y1 = 0; y1 < GxGDEW0154M09_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW0154M09_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW0154M09_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
        _writeData(~data); // white is 0xFF on device
      }
    }
  }
  _current_page = -1;
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("drawPaged");
  _PowerOff();
}

void GxGDEW0154M09::drawPaged(void (*drawCallback)(uint32_t), uint32_t p)
{
  if (_current_page != -1) return;
  _Init_Full();
  _writeCommand(0x10);
  for (uint32_t i = 0; i < GxGDEW0154M09_BUFFER_SIZE; i++)
  {
    _writeData(0xFF); // 0xFF is white
  }
  _writeCommand(0x13);
  for (_current_page = 0; _current_page < GxGDEW0154M09_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p);
    for (int16_t y1 = 0; y1 < GxGDEW0154M09_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW0154M09_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW0154M09_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
        _writeData(~data); // white is 0xFF on device
      }
    }
  }
  _current_page = -1;
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("drawPaged");
  _PowerOff();
}

void GxGDEW0154M09::drawPaged(void (*drawCallback)(const void*), const void* p)
{
  if (_current_page != -1) return;
  _Init_Full();
  _writeCommand(0x10);
  for (uint32_t i = 0; i < GxGDEW0154M09_BUFFER_SIZE; i++)
  {
    _writeData(0xFF); // 0xFF is white
  }
  _writeCommand(0x13);
  for (_current_page = 0; _current_page < GxGDEW0154M09_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p);
    for (int16_t y1 = 0; y1 < GxGDEW0154M09_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW0154M09_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW0154M09_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
        _writeData(~data); // white is 0xFF on device
      }
    }
  }
  _current_page = -1;
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("drawPaged");
  _PowerOff();
}

void GxGDEW0154M09::drawPaged(void (*drawCallback)(const void*, const void*), const void* p1, const void* p2)
{
  if (_current_page != -1) return;
  _Init_Full();
  _writeCommand(0x10);
  for (uint32_t i = 0; i < GxGDEW0154M09_BUFFER_SIZE; i++)
  {
    _writeData(0xFF); // 0xFF is white
  }
  _writeCommand(0x13);
  for (_current_page = 0; _current_page < GxGDEW0154M09_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p1, p2);
    for (int16_t y1 = 0; y1 < GxGDEW0154M09_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW0154M09_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW0154M09_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
        _writeData(~data); // white is 0xFF on device
      }
    }
  }
  _current_page = -1;
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("drawPaged");
  _PowerOff();
}

void GxGDEW0154M09::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = GxGDEW0154M09_WIDTH - x - w;
      break;
    case 2:
      x = GxGDEW0154M09_WIDTH - x - w;
      y = GxGDEW0154M09_HEIGHT - y - h;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = GxGDEW0154M09_HEIGHT - y - h;
      break;
  }
}

void GxGDEW0154M09::drawPagedToWindow(void (*drawCallback)(void), uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  if (x >= GxGDEW0154M09_WIDTH) return;
  if (y >= GxGDEW0154M09_HEIGHT) return;
  uint16_t xe = gx_uint16_min(GxGDEW0154M09_WIDTH, x + w) - 1;
  uint16_t ye = gx_uint16_min(GxGDEW0154M09_HEIGHT, y + h) - 1;
  uint16_t xs_bx = x / 8;
  uint16_t xe_bx = (xe + 7) / 8;
  if (!_using_partial_mode) eraseDisplay(true);
  _Init_Part();
  _writeCommand(0x91); // partial in
  //for (uint16_t twice = 0; twice < 2; twice++)
  for (uint8_t command = 0x13; true; command = 0x10)
  { // leave both controller buffers equal
    _setPartialRamArea(x, y, xe, ye);
    _writeCommand(command);
    for (_current_page = 0; _current_page < GxGDEW0154M09_PAGES; _current_page++)
    {
      uint16_t yds = gx_uint16_max(y, _current_page * GxGDEW0154M09_PAGE_HEIGHT);
      uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEW0154M09_PAGE_HEIGHT) - 1;
      if (yde > yds)
      {
        fillScreen(GxEPD_WHITE);
        drawCallback();
        uint16_t ys = yds % GxGDEW0154M09_PAGE_HEIGHT;
        for (int16_t y1 = yds; y1 <= yde; ys++, y1++)
        {
          for (int16_t x1 = xs_bx; x1 < xe_bx; x1++)
          {
            uint16_t idx = ys * (GxGDEW0154M09_WIDTH / 8) + x1;
            uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00; // white is 0x00 in buffer
            _writeData(~data); // white is 0xFF on device
          }
        }
      }
    }
    _current_page = -1;
    if (command == 0x10) break;
    _writeCommand(0x12);      //display refresh
    _waitWhileBusy("drawPagedToWindow");
  } // leave both controller buffers equal
  _writeCommand(0x92); // partial out
  delay(GxGDEW0154M09_PU_DELAY); // don't stress this display
}

void GxGDEW0154M09::drawPagedToWindow(void (*drawCallback)(uint32_t), uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t p)
{
    Serial.println("drawPagedToWindow");
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  if (x >= GxGDEW0154M09_WIDTH) return;
  if (y >= GxGDEW0154M09_HEIGHT) return;
  uint16_t xe = gx_uint16_min(GxGDEW0154M09_WIDTH, x + w) - 1;
  uint16_t ye = gx_uint16_min(GxGDEW0154M09_HEIGHT, y + h) - 1;
  uint16_t xs_bx = x / 8;
  uint16_t xe_bx = (xe + 7) / 8;
  if (!_using_partial_mode) eraseDisplay(true);
  _Init_Part();
  _writeCommand(0x91); // partial in
  for (uint8_t command = 0x13; true; command = 0x10)
  { // leave both controller buffers equal
    _setPartialRamArea(x, y, xe, ye);
    _writeCommand(command);
    for (_current_page = 0; _current_page < GxGDEW0154M09_PAGES; _current_page++)
    {
      uint16_t yds = gx_uint16_max(y, _current_page * GxGDEW0154M09_PAGE_HEIGHT);
      uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEW0154M09_PAGE_HEIGHT) - 1;
      if (yde > yds)
      {
        fillScreen(GxEPD_WHITE);
        drawCallback(p);
        uint16_t ys = yds % GxGDEW0154M09_PAGE_HEIGHT;
        for (int16_t y1 = yds; y1 <= yde; ys++, y1++)
        {
          for (int16_t x1 = xs_bx; x1 < xe_bx; x1++)
          {
            uint16_t idx = ys * (GxGDEW0154M09_WIDTH / 8) + x1;
            uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00; // white is 0x00 in buffer
            _writeData(~data); // white is 0xFF on device
          }
        }
      }
    }
    _current_page = -1;
    if (command == 0x10) break;
    _writeCommand(0x12);      //display refresh
    _waitWhileBusy("drawPagedToWindow");
  } // leave both controller buffers equal
  _writeCommand(0x92); // partial out
  delay(GxGDEW0154M09_PU_DELAY); // don't stress this display
}

void GxGDEW0154M09::drawPagedToWindow(void (*drawCallback)(const void*), uint16_t x, uint16_t y, uint16_t w, uint16_t h, const void* p)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  if (x >= GxGDEW0154M09_WIDTH) return;
  if (y >= GxGDEW0154M09_HEIGHT) return;
  uint16_t xe = gx_uint16_min(GxGDEW0154M09_WIDTH, x + w) - 1;
  uint16_t ye = gx_uint16_min(GxGDEW0154M09_HEIGHT, y + h) - 1;
  uint16_t xs_bx = x / 8;
  uint16_t xe_bx = (xe + 7) / 8;
  if (!_using_partial_mode) eraseDisplay(true);
  _Init_Part();
  _writeCommand(0x91); // partial in
  for (uint8_t command = 0x13; true; command = 0x10)
  { // leave both controller buffers equal
    _setPartialRamArea(x, y, xe, ye);
    _writeCommand(command);
    for (_current_page = 0; _current_page < GxGDEW0154M09_PAGES; _current_page++)
    {
      uint16_t yds = gx_uint16_max(y, _current_page * GxGDEW0154M09_PAGE_HEIGHT);
      uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEW0154M09_PAGE_HEIGHT) - 1;
      if (yde > yds)
      {
        fillScreen(GxEPD_WHITE);
        drawCallback(p);
        uint16_t ys = yds % GxGDEW0154M09_PAGE_HEIGHT;
        for (int16_t y1 = yds; y1 <= yde; ys++, y1++)
        {
          for (int16_t x1 = xs_bx; x1 < xe_bx; x1++)
          {
            uint16_t idx = ys * (GxGDEW0154M09_WIDTH / 8) + x1;
            uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00; // white is 0x00 in buffer
            _writeData(~data); // white is 0xFF on device
          }
        }
      }
    }
    _current_page = -1;
    if (command == 0x10) break;
    _writeCommand(0x12);      //display refresh
    _waitWhileBusy("drawPagedToWindow");
  } // leave both controller buffers equal
  _writeCommand(0x92); // partial out
  delay(GxGDEW0154M09_PU_DELAY); // don't stress this display
}

void GxGDEW0154M09::drawPagedToWindow(void (*drawCallback)(const void*, const void*), uint16_t x, uint16_t y, uint16_t w, uint16_t h, const void* p1, const void* p2)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  if (x >= GxGDEW0154M09_WIDTH) return;
  if (y >= GxGDEW0154M09_HEIGHT) return;
  uint16_t xe = gx_uint16_min(GxGDEW0154M09_WIDTH, x + w) - 1;
  uint16_t ye = gx_uint16_min(GxGDEW0154M09_HEIGHT, y + h) - 1;
  uint16_t xs_bx = x / 8;
  uint16_t xe_bx = (xe + 7) / 8;
  if (!_using_partial_mode) eraseDisplay(true);
  _Init_Part();
  _writeCommand(0x91); // partial in
  for (uint8_t command = 0x13; true; command = 0x10)
  { // leave both controller buffers equal
    _setPartialRamArea(x, y, xe, ye);
    _writeCommand(command);
    for (_current_page = 0; _current_page < GxGDEW0154M09_PAGES; _current_page++)
    {
      uint16_t yds = gx_uint16_max(y, _current_page * GxGDEW0154M09_PAGE_HEIGHT);
      uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEW0154M09_PAGE_HEIGHT) - 1;
      if (yde > yds)
      {
        fillScreen(GxEPD_WHITE);
        drawCallback(p1, p2);
        uint16_t ys = yds % GxGDEW0154M09_PAGE_HEIGHT;
        for (int16_t y1 = yds; y1 <= yde; ys++, y1++)
        {
          for (int16_t x1 = xs_bx; x1 < xe_bx; x1++)
          {
            uint16_t idx = ys * (GxGDEW0154M09_WIDTH / 8) + x1;
            uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00; // white is 0x00 in buffer
            _writeData(~data); // white is 0xFF on device
          }
        }
      }
    }
    _current_page = -1;
    if (command == 0x10) break;
    _writeCommand(0x12);      //display refresh
    _waitWhileBusy("drawPagedToWindow");
  } // leave both controller buffers equal
  _writeCommand(0x92); // partial out
  delay(GxGDEW0154M09_PU_DELAY); // don't stress this display
}

void GxGDEW0154M09::drawCornerTest(uint8_t em)
{
  if (_current_page != -1) return;
  _Init_Full();
  _writeCommand(0x10);
  for (uint32_t i = 0; i < GxGDEW0154M09_BUFFER_SIZE; i++)
  {
    _writeData(0xFF); // white is 0xFF on device
  }
  _writeCommand(0x13);
  for (uint32_t y = 0; y < GxGDEW0154M09_HEIGHT; y++)
  {
    for (uint32_t x = 0; x < GxGDEW0154M09_WIDTH / 8; x++)
    {
      uint8_t data = 0xFF; // white is 0xFF on device
      if ((x < 1) && (y < 8)) data = 0x00;
      if ((x > GxGDEW0154M09_WIDTH / 8 - 3) && (y < 16)) data = 0x00;
      if ((x > GxGDEW0154M09_WIDTH / 8 - 4) && (y > GxGDEW0154M09_HEIGHT - 25)) data = 0x00;
      if ((x < 4) && (y > GxGDEW0154M09_HEIGHT - 33)) data = 0x00;
      _writeData(data);
    }
  }
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("drawCornerTest");
  _PowerOff();
}
