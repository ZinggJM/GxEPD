/************************************************************************************
   class GxGDEW027W3 : Display class example for GDEW027W3 e-Paper from Dalian Good Display Co., Ltd.: www.good-display.com

   based on Demo Example from Good Display, available here: http://www.good-display.com/download_detail/downloadsId=520.html

   Author : J-M Zingg

   Version : 2.3

   Support: limited, provided as example, no claim to be fit for serious use

   Controller: IL91874 : http://www.good-display.com/download_detail/downloadsId=539.html

   connection to the e-Paper display is through DESTM32-S2 connection board, available from Good Display

   DESTM32-S2 pinout (top, component side view):
         |-------------------------------------------------
         |  VCC  |o o| VCC 5V  not needed
         |  GND  |o o| GND     GND
         |  3.3  |o o| 3.3     3.3V
         |  nc   |o o| nc
         |  nc   |o o| nc
         |  nc   |o o| nc
   MOSI  |  DIN  |o o| CLK     SCK
   SS    |  CS   |o o| DC      e.g. D3
   D4    |  RST  |o o| BUSY    e.g. D2
         |  nc   |o o| BS      GND
         |-------------------------------------------------
*/
#include "GxGDEW027W3.h"

//#define DISABLE_DIAGNOSTIC_OUTPUT

#if defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

const uint8_t GxGDEW027W3::lut_20_vcomDC[] =
{
0x00  ,0x00 ,
0x00  ,0x0F ,0x0F ,0x00 ,0x00 ,0x05,
0x00  ,0x32 ,0x32 ,0x00 ,0x00 ,0x02,
0x00  ,0x0F ,0x0F ,0x00 ,0x00 ,0x05,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
};
//R21H
const uint8_t GxGDEW027W3::lut_21_ww[] =
{
0x50  ,0x0F ,0x0F ,0x00 ,0x00 ,0x05,
0x60  ,0x32 ,0x32 ,0x00 ,0x00 ,0x02,
0xA0  ,0x0F ,0x0F ,0x00 ,0x00 ,0x05,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
};
//R22H  r
const uint8_t GxGDEW027W3::lut_22_bw[] =
{
0x50  ,0x0F ,0x0F ,0x00 ,0x00 ,0x05,
0x60  ,0x32 ,0x32 ,0x00 ,0x00 ,0x02,
0xA0  ,0x0F ,0x0F ,0x00 ,0x00 ,0x05,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
};
//R23H  w
const uint8_t GxGDEW027W3::lut_23_wb[] =
{
0xA0  ,0x0F ,0x0F ,0x00 ,0x00 ,0x05,
0x60  ,0x32 ,0x32 ,0x00 ,0x00 ,0x02,
0x50  ,0x0F ,0x0F ,0x00 ,0x00 ,0x05,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
};
//R24H  b
const uint8_t GxGDEW027W3::lut_24_bb[] =
{
0xA0  ,0x0F ,0x0F ,0x00 ,0x00 ,0x05,
0x60  ,0x32 ,0x32 ,0x00 ,0x00 ,0x02,
0x50  ,0x0F ,0x0F ,0x00 ,0x00 ,0x05,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
};

GxGDEW027W3::GxGDEW027W3(GxIO& io, int8_t rst, int8_t busy)
  : GxEPD(GxGDEW027W3_WIDTH, GxGDEW027W3_HEIGHT), IO(io),
    _current_page(-1), _using_partial_mode(false),
    _rst(rst), _busy(busy)
{
}

void GxGDEW027W3::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GxGDEW027W3_WIDTH - x - 1;
      break;
    case 2:
      x = GxGDEW027W3_WIDTH - x - 1;
      y = GxGDEW027W3_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GxGDEW027W3_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GxGDEW027W3_WIDTH / 8;
  if (_current_page < 1)
  {
    if (i >= sizeof(_buffer)) return;
  }
  else
  {
    y -= _current_page * GxGDEW027W3_PAGE_HEIGHT;
    if ((y < 0) || (y >= GxGDEW027W3_PAGE_HEIGHT)) return;
    i = x / 8 + y * GxGDEW027W3_WIDTH / 8;
  }

  if (!color)
    _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
  else
    _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
}


void GxGDEW027W3::init(void)
{
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

void GxGDEW027W3::fillScreen(uint16_t color)
{
  uint8_t data = (color == GxEPD_BLACK) ? 0xFF : 0x00;
  for (uint16_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = data;
  }
}

void GxGDEW027W3::update(void)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  _writeCommand(0x10);
  for (uint32_t i = 0; i < GxGDEW027W3_BUFFER_SIZE; i++)
  {
    _writeData(0xFF); // 0xFF is white
  }
  _writeCommand(0x13);
  for (uint32_t i = 0; i < GxGDEW027W3_BUFFER_SIZE; i++)
  {
    _writeData((i < sizeof(_buffer)) ? ~_buffer[i] : 0xFF);
  }
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("update");
  _sleep();
}

void  GxGDEW027W3::drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode)
{
  if (mode & bm_default) mode |= bm_invert;
  drawBitmapBM(bitmap, x, y, w, h, color, mode);
}

void GxGDEW027W3::drawBitmap(const uint8_t* bitmap, uint32_t size, int16_t mode)
{
  if (_current_page != -1) return;
  if (mode & bm_default) mode |= bm_normal; // no change
  if (mode & bm_partial_update)
  {
    _using_partial_mode = true;
    _wakeUp();
    _setPartialRamArea(0x15, 0, 0, GxGDEW027W3_WIDTH, GxGDEW027W3_HEIGHT);
    for (uint32_t i = 0; i < GxGDEW027W3_BUFFER_SIZE; i++)
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
    _refreshWindow(0, 0, GxGDEW027W3_WIDTH, GxGDEW027W3_HEIGHT);
    _waitWhileBusy("drawBitmap");
  }
  else
  {
    _using_partial_mode = false;
    _wakeUp();
    _writeCommand(0x10);
    for (uint32_t i = 0; i < GxGDEW027W3_BUFFER_SIZE; i++)
    {
      _writeData(0);
    }
    _writeCommand(0x13);
    for (uint32_t i = 0; i < GxGDEW027W3_BUFFER_SIZE; i++)
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
    _sleep();
  }
}

void GxGDEW027W3::eraseDisplay(bool using_partial_update)
{
  if (_current_page != -1) return;
  if (using_partial_update)
  {
    if (!_using_partial_mode) _wakeUp();
    _using_partial_mode = true; // remember
    _setPartialRamArea(0x14, 0, 0, GxGDEW027W3_WIDTH, GxGDEW027W3_HEIGHT);
    for (uint32_t i = 0; i < GxGDEW027W3_BUFFER_SIZE; i++)
    {
      _writeData(0xFF); // 0xFF is white
    }
    _setPartialRamArea(0x15, 0, 0, GxGDEW027W3_WIDTH, GxGDEW027W3_HEIGHT);
    for (uint32_t i = 0; i < GxGDEW027W3_BUFFER_SIZE; i++)
    {
      _writeData(0xFF); // 0xFF is white
    }
    _refreshWindow(0, 0, GxGDEW027W3_WIDTH, GxGDEW027W3_HEIGHT);
    _waitWhileBusy("drawBitmap");
  }
  else
  {
    _using_partial_mode = false;
    _wakeUp();
    _writeCommand(0x10);
    for (uint32_t i = 0; i < GxGDEW027W3_BUFFER_SIZE; i++)
    {
      _writeData(0xFF); // 0xFF is white
    }
    _writeCommand(0x13);
    for (uint32_t i = 0; i < GxGDEW027W3_BUFFER_SIZE; i++)
    {
      _writeData(0xFF); // 0xFF is white
    }
    _writeCommand(0x12);      //display refresh
    _waitWhileBusy("eraseDisplay");
    _sleep();
  }
}

void GxGDEW027W3::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  if (_current_page != -1) return;
  if (using_rotation)
  {
    switch (getRotation())
    {
      case 1:
        swap(x, y);
        swap(w, h);
        x = GxGDEW027W3_WIDTH - x - w - 1;
        break;
      case 2:
        x = GxGDEW027W3_WIDTH - x - w - 1;
        y = GxGDEW027W3_HEIGHT - y - h - 1;
        break;
      case 3:
        swap(x, y);
        swap(w, h);
        y = GxGDEW027W3_HEIGHT - y  - h - 1;
        break;
    }
  }
  //fillScreen(0x0);
  if (!_using_partial_mode) _wakeUp();
  _using_partial_mode = true;
  _writeToWindow(x, y, x, y, w, h);
  _refreshWindow(x, y, w, h);
  _waitWhileBusy("updateWindow");
}

void GxGDEW027W3::updateToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h, bool using_rotation)
{
  if (using_rotation)
  {
    switch (getRotation())
    {
      case 1:
        swap(xs, ys);
        swap(xd, yd);
        swap(w, h);
        xs = GxGDEW027W3_WIDTH - xs - w - 1;
        xd = GxGDEW027W3_WIDTH - xd - w - 1;
        break;
      case 2:
        xs = GxGDEW027W3_WIDTH - xs - w - 1;
        ys = GxGDEW027W3_HEIGHT - ys - h - 1;
        xd = GxGDEW027W3_WIDTH - xd - w - 1;
        yd = GxGDEW027W3_HEIGHT - yd - h - 1;
        break;
      case 3:
        swap(xs, ys);
        swap(xd, yd);
        swap(w, h);
        ys = GxGDEW027W3_HEIGHT - ys  - h - 1;
        yd = GxGDEW027W3_HEIGHT - yd  - h - 1;
        break;
    }
  }
  if (!_using_partial_mode) _wakeUp();
  _using_partial_mode = true;
  _writeToWindow(xs, ys, xd, yd, w, h);
  _refreshWindow(xd, yd, w, h);
  _waitWhileBusy("updateToWindow");
  delay(500); // don't stress this display
}

void GxGDEW027W3::_writeToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h)
{
  //Serial.printf("_writeToWindow(%d, %d, %d, %d, %d, %d)\n", xs, ys, xd, yd, w, h);
  // the screen limits are the hard limits
  if (xs >= GxGDEW027W3_WIDTH) return;
  if (ys >= GxGDEW027W3_HEIGHT) return;
  if (xd >= GxGDEW027W3_WIDTH) return;
  if (yd >= GxGDEW027W3_HEIGHT) return;
  w = gx_uint16_min(w + 7, GxGDEW027W3_WIDTH - xd) + (xd % 8);
  h = gx_uint16_min(h, GxGDEW027W3_HEIGHT - yd);
  uint16_t xe = (xs / 8) + (w / 8);
  _setPartialRamArea(0x15, xd, yd, w, h);
  for (uint16_t y1 = ys; y1 < ys + h; y1++)
  {
    for (uint16_t x1 = xs / 8; x1 < xe; x1++)
    {
      uint16_t idx = y1 * (GxGDEW027W3_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
      IO.writeDataTransaction(~data);
    }
  }
  delay(2);
}

void GxGDEW027W3::powerDown()
{
  _using_partial_mode = false; // force _wakeUp()
  _sleep();
}

void GxGDEW027W3::_setPartialRamArea(uint8_t command, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  IO.writeCommandTransaction(command);
  IO.writeDataTransaction(x >> 8);
  IO.writeDataTransaction(x & 0xf8);
  IO.writeDataTransaction(y >> 8);
  IO.writeDataTransaction(y & 0xff);
  IO.writeDataTransaction(w >> 8);
  IO.writeDataTransaction(w & 0xf8);
  IO.writeDataTransaction(h >> 8);
  IO.writeDataTransaction(h & 0xff);
}

void GxGDEW027W3::_refreshWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  w += (x % 8) + 7;
  h = gx_uint16_min(h, 256); // strange controller error
  IO.writeCommandTransaction(0x16);
  IO.writeDataTransaction(x >> 8);
  IO.writeDataTransaction(x & 0xf8);
  IO.writeDataTransaction(y >> 8);
  IO.writeDataTransaction(y & 0xff);
  IO.writeDataTransaction(w >> 8);
  IO.writeDataTransaction(w & 0xf8);
  IO.writeDataTransaction(h >> 8);
  IO.writeDataTransaction(h & 0xff);
}

void GxGDEW027W3::_writeCommand(uint8_t command)
{
  IO.writeCommandTransaction(command);
}

void GxGDEW027W3::_writeData(uint8_t data)
{
  IO.writeDataTransaction(data);
}

void GxGDEW027W3::_waitWhileBusy(const char* comment)
{
  unsigned long start = micros();
  while (1)
  {
    if (digitalRead(_busy) == 1) break;
    delay(1);
    if (micros() - start > 20000000) // > 15.5s !
    {
      Serial.println("Busy Timeout!");
      break;
    }
  }
  if (comment)
  {
#if !defined(DISABLE_DIAGNOSTIC_OUTPUT)
    unsigned long elapsed = micros() - start;
    Serial.print(comment);
    Serial.print(" : ");
    Serial.println(elapsed);
#endif
  }
  (void) start;
}

void GxGDEW027W3::_wakeUp()
{
  // reset required for wakeup
  if (_rst >= 0)
  {
    digitalWrite(_rst, 0);
    delay(10);
    digitalWrite(_rst, 1);
    delay(10);
  }

  _writeCommand(0x01);
  _writeData (0x03);
  _writeData (0x00);
  _writeData (0x2b);
  _writeData (0x2b);
  _writeData (0x09);

  _writeCommand(0x06);
  _writeData (0x07);
  _writeData (0x07);
  _writeData (0x17);

  _writeCommand(0xF8);
  _writeData (0x60);
  _writeData (0xA5);

  _writeCommand(0xF8);
  _writeData (0x89);
  _writeData (0xA5);

  _writeCommand(0xF8);
  _writeData (0x90);
  _writeData (0x00);

  _writeCommand(0xF8);
  _writeData (0x93);
  _writeData (0x2A);

  _writeCommand(0xF8);
  _writeData (0xa0);
  _writeData (0xa5);

  _writeCommand(0xF8);
  _writeData (0xa1);
  _writeData (0x00);

  _writeCommand(0xF8);
  _writeData (0x73);
  _writeData (0x41);

  _writeCommand(0x16);
  _writeData(0x00);

  _writeCommand(0x04);
  _waitWhileBusy("_wakeUp Power On");

  _writeCommand(0x00);
  //_writeData(0xaf); //KW-BF   KWR-AF    BWROTP 0f
  //_writeData(0xbf); // b/w, by register LUT
  _writeData(0x9f); // b/w, by OTP LUT

  _writeCommand(0x30);
  _writeData (0x3a); //3A 100HZ

  _writeCommand(0x61);
  _writeData (0x00);
  _writeData (0xb0); //176
  _writeData (0x01);
  _writeData (0x08); //264

  _writeCommand(0x82);
  _writeData (0x12);
  _writeLUT();
}

void GxGDEW027W3::_sleep(void)
{
  _writeCommand(0x02); // power off
  _waitWhileBusy("_sleep Power Off");
  if (_rst >= 0)
  {
    _writeCommand(0x07); // deep sleep
    _writeData (0xa5);
  }
}

void GxGDEW027W3::_writeLUT(void)
{
  unsigned int count;
  {
    _writeCommand(0x20);							//vcom
    for (count = 0; count < 44; count++)
    {
      _writeData(lut_20_vcomDC[count]);
    }

    _writeCommand(0x21);							//ww --
    for (count = 0; count < 42; count++)
    {
      _writeData(lut_21_ww[count]);
    }

    _writeCommand(0x22);							//bw r
    for (count = 0; count < 42; count++)
    {
      _writeData(lut_22_bw[count]);
    }

    _writeCommand(0x23);							//wb w
    for (count = 0; count < 42; count++)
    {
      _writeData(lut_23_wb[count]);
    }

    _writeCommand(0x24);							//bb b
    for (count = 0; count < 42; count++)
    {
      _writeData(lut_24_bb[count]);
    }
  }
}

void GxGDEW027W3::drawPaged(void (*drawCallback)(void))
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  _writeCommand(0x13);
  for (_current_page = 0; _current_page < GxGDEW027W3_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback();
    for (int16_t y1 = 0; y1 < GxGDEW027W3_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW027W3_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW027W3_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
        _writeData(~data);
      }
    }
  }
  _current_page = -1;
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("drawPaged");
  _sleep();
}

void GxGDEW027W3::drawPaged(void (*drawCallback)(uint32_t), uint32_t p)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  _writeCommand(0x13);
  for (_current_page = 0; _current_page < GxGDEW027W3_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p);
    for (int16_t y1 = 0; y1 < GxGDEW027W3_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW027W3_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW027W3_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
        _writeData(~data);
      }
    }
  }
  _current_page = -1;
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("drawPaged");
  _sleep();
}

void GxGDEW027W3::drawPaged(void (*drawCallback)(const void*), const void* p)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  _writeCommand(0x13);
  for (_current_page = 0; _current_page < GxGDEW027W3_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p);
    for (int16_t y1 = 0; y1 < GxGDEW027W3_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW027W3_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW027W3_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
        _writeData(~data);
      }
    }
  }
  _current_page = -1;
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("drawPaged");
  _sleep();
}

void GxGDEW027W3::drawPaged(void (*drawCallback)(const void*, const void*), const void* p1, const void* p2)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  _writeCommand(0x13);
  for (_current_page = 0; _current_page < GxGDEW027W3_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p1, p2);
    for (int16_t y1 = 0; y1 < GxGDEW027W3_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW027W3_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW027W3_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
        _writeData(~data);
      }
    }
  }
  _current_page = -1;
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("drawPaged");
  _sleep();
}

void GxGDEW027W3::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = GxGDEW027W3_WIDTH - x - w - 1;
      break;
    case 2:
      x = GxGDEW027W3_WIDTH - x - w - 1;
      y = GxGDEW027W3_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = GxGDEW027W3_HEIGHT - y - h - 1;
      break;
  }
}

void GxGDEW027W3::drawPagedToWindow(void (*drawCallback)(void), uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  if (!_using_partial_mode)
  {
    eraseDisplay(false);
    eraseDisplay(true);
  }
  _using_partial_mode = true;
  for (_current_page = 0; _current_page < GxGDEW027W3_PAGES; _current_page++)
  {
    uint16_t yds = gx_uint16_max(y, _current_page * GxGDEW027W3_PAGE_HEIGHT);
    uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEW027W3_PAGE_HEIGHT);
    if (yde > yds)
    {
      fillScreen(GxEPD_WHITE);
      drawCallback();
      uint16_t ys = yds % GxGDEW027W3_PAGE_HEIGHT;
      _writeToWindow(x, ys, x, yds, w, yde - yds);
    }
  }
  _refreshWindow(x, y, w, h);
  _waitWhileBusy("updateToWindow");
  _current_page = -1;
}

void GxGDEW027W3::drawPagedToWindow(void (*drawCallback)(uint32_t), uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t p)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  if (!_using_partial_mode)
  {
    eraseDisplay(false);
    eraseDisplay(true);
  }
  _using_partial_mode = true;
  for (_current_page = 0; _current_page < GxGDEW027W3_PAGES; _current_page++)
  {
    uint16_t yds = gx_uint16_max(y, _current_page * GxGDEW027W3_PAGE_HEIGHT);
    uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEW027W3_PAGE_HEIGHT);
    if (yde > yds)
    {
      fillScreen(GxEPD_WHITE);
      drawCallback(p);
      //fillScreen(p);
      uint16_t ys = yds % GxGDEW027W3_PAGE_HEIGHT;
      _writeToWindow(x, ys, x, yds, w, yde - yds);
    }
  }
  _refreshWindow(x, y, w, h);
  _waitWhileBusy("updateToWindow");
  _current_page = -1;
}

void GxGDEW027W3::drawPagedToWindow(void (*drawCallback)(const void*), uint16_t x, uint16_t y, uint16_t w, uint16_t h, const void* p)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  if (!_using_partial_mode)
  {
    eraseDisplay(false);
    eraseDisplay(true);
  }
  _using_partial_mode = true;
  for (_current_page = 0; _current_page < GxGDEW027W3_PAGES; _current_page++)
  {
    uint16_t yds = gx_uint16_max(y, _current_page * GxGDEW027W3_PAGE_HEIGHT);
    uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEW027W3_PAGE_HEIGHT);
    if (yde > yds)
    {
      fillScreen(GxEPD_WHITE);
      drawCallback(p);
      uint16_t ys = yds % GxGDEW027W3_PAGE_HEIGHT;
      _writeToWindow(x, ys, x, yds, w, yde - yds);
    }
  }
  _refreshWindow(x, y, w, h);
  _waitWhileBusy("updateToWindow");
  _current_page = -1;
}

void GxGDEW027W3::drawPagedToWindow(void (*drawCallback)(const void*, const void*), uint16_t x, uint16_t y, uint16_t w, uint16_t h, const void* p1, const void* p2)
{
  if (_current_page != -1) return;
  _rotate(x, y, w, h);
  if (!_using_partial_mode)
  {
    eraseDisplay(false);
    eraseDisplay(true);
  }
  _using_partial_mode = true;
  for (_current_page = 0; _current_page < GxGDEW027W3_PAGES; _current_page++)
  {
    uint16_t yds = gx_uint16_max(y, _current_page * GxGDEW027W3_PAGE_HEIGHT);
    uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxGDEW027W3_PAGE_HEIGHT);
    if (yde > yds)
    {
      fillScreen(GxEPD_WHITE);
      drawCallback(p1, p2);
      uint16_t ys = yds % GxGDEW027W3_PAGE_HEIGHT;
      _writeToWindow(x, ys, x, yds, w, yde - yds);
    }
  }
  _refreshWindow(x, y, w, h);
  _waitWhileBusy("updateToWindow");
  _current_page = -1;
}

void GxGDEW027W3::drawCornerTest(uint8_t em)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  _writeCommand(0x13);
  for (uint32_t y = 0; y < GxGDEW027W3_HEIGHT; y++)
  {
    for (uint32_t x = 0; x < GxGDEW027W3_WIDTH / 8; x++)
    {
      uint8_t data = 0xFF;
      if ((x < 1) && (y < 8)) data = 0x00;
      if ((x > GxGDEW027W3_WIDTH / 8 - 3) && (y < 16)) data = 0x00;
      if ((x > GxGDEW027W3_WIDTH / 8 - 4) && (y > GxGDEW027W3_HEIGHT - 25)) data = 0x00;
      if ((x < 4) && (y > GxGDEW027W3_HEIGHT - 33)) data = 0x00;
      _writeData(data);
    }
  }
  _writeCommand(0x12);      //display refresh
  _waitWhileBusy("drawCornerTest");
  _sleep();
}

