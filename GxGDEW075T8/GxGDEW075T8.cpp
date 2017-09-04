/************************************************************************************
   class GxGDEW075T8 : Display class example for GDEW075T8 e-Paper from Dalian Good Display Co., Ltd.: www.good-display.com

   based on Demo Example from Good Display, now available on http://www.good-display.com/download_list/downloadcategoryid=34&isMode=false.html

   Author : J-M Zingg

   modified by :

   Version : 2.0

   Support: limited, provided as example, no claim to be fit for serious use

   connection to the e-Paper display is through DESTM32-S2 connection board, available from Good Display

   DESTM32-S2 pinout (top, component side view):
       |-------------------------------------------------
       |  VCC  |o o| VCC 5V, not needed
       |  GND  |o o| GND
       |  3.3  |o o| 3.3V
       |  nc   |o o| nc
       |  nc   |o o| nc
       |  nc   |o o| nc
       |  MOSI |o o| CLK=SCK
       | SS=DC |o o| D/C=RS    // Slave Select = Device Connect |o o| Data/Command = Register Select
       |  RST  |o o| BUSY
       |  nc   |o o| BS, connect to GND
       |-------------------------------------------------
*/

// note 28.8.2017 : the controller UC8159 seems to know command 90H Partial Window, but neither 91H Partial In nor 92H Partial Out
// the GDEW075T8 specification does not contain these commands.
// the specification for UC8159 from UltraChip does not contain a command list, and their internet presence is "very up-to-date";
// see http://www.ultrachip.com/en/news.php?id=47

#include "GxGDEW075T8.h"

#if defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

GxGDEW075T8::GxGDEW075T8(GxIO& io, uint8_t rst, uint8_t busy)
  : GxEPD(GxGDEW075T8_WIDTH, GxGDEW075T8_HEIGHT),
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

void GxGDEW075T8::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GxGDEW075T8_WIDTH - x - 1;
      break;
    case 2:
      x = GxGDEW075T8_WIDTH - x - 1;
      y = GxGDEW075T8_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GxGDEW075T8_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GxGDEW075T8_WIDTH / 8;
  if (_current_page < 1)
  {
    if (i >= sizeof(_buffer)) return;
  }
  else
  {
    if (i < GxGDEW075T8_PAGE_SIZE * _current_page) return;
    if (i >= GxGDEW075T8_PAGE_SIZE * (_current_page + 1)) return;
    i -= GxGDEW075T8_PAGE_SIZE * _current_page;
  }

  if (!color)
    _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
  else
    _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
}

void GxGDEW075T8::init(void)
{
  IO.init();
  IO.setFrequency(4000000); // 4MHz : 250ns > 150ns min RD cycle
  digitalWrite(_rst, HIGH);
  pinMode(_rst, OUTPUT);
  pinMode(_busy, INPUT);
  fillScreen(GxEPD_WHITE);
  _current_page = -1;
  _using_partial_mode = false;
}

void GxGDEW075T8::fillScreen(uint16_t color)
{
  uint8_t data = (color == GxEPD_BLACK) ? 0xFF : 0x00;
  for (uint16_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = data;
  }
}

void GxGDEW075T8::update(void)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _wakeUp();
  IO.writeCommandTransaction(0x10);
  for (uint32_t i = 0; i < GxGDEW075T8_BUFFER_SIZE; i++)
  {
#if defined(ESP8266)
    // (10000 * 8bit * (8bits/bit + gap)/ 4MHz = ~ 200ms
    // (31000 * 8bit * (8bits/bit + gap)/ 4MHz = ~ 600ms is safe
    // if ((i % 10000) == 0) yield(); // avoid watchdog reset
#endif
    _send8pixel(i < sizeof(_buffer) ? _buffer[i] : 0x00);
  }
  IO.writeCommandTransaction(0x12);      //display refresh
  _waitWhileBusy("update");
  _sleep();
}

void  GxGDEW075T8::drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color)
{
  drawBitmap(bitmap, x, y, w, h, color);
}

void  GxGDEW075T8::drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode)
{
  if (mode == bm_default) mode = bm_normal;
  drawBitmapBM(bitmap, x, y, w, h, color, mode);
}

void GxGDEW075T8::drawBitmap(const uint8_t *bitmap, uint32_t size)
{
  drawBitmap(bitmap, size, false);
}

void GxGDEW075T8::drawBitmap(const uint8_t *bitmap, uint32_t size, bool using_partial_update)
{
  if (using_partial_update)
  {
    _using_partial_mode = true; // remember
    _wakeUp();
    // set full screen
    IO.writeCommandTransaction(0x91); // partial in
    _setPartialRamArea(0, 0, GxGDEW075T8_WIDTH - 1, GxGDEW075T8_HEIGHT - 1);
    for (uint32_t i = 0; i < GxGDEW075T8_BUFFER_SIZE; i++)
    {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
      _send8pixel(i < size ? pgm_read_byte(bitmap + i) : 0);
#else
      _send8pixel(i < size ? bitmap[i] : 0);
#endif
    }
    IO.writeCommandTransaction(0x12);      //display refresh
    _waitWhileBusy("drawBitmap");
    IO.writeCommandTransaction(0x92); // partial out
  }
  else
  {
    _using_partial_mode = false; // remember
    _wakeUp();
    IO.writeCommandTransaction(0x10);
    for (uint32_t i = 0; i < GxGDEW075T8_BUFFER_SIZE; i++)
    {
#if defined(ESP8266)
      // (10000 * 8bit * (8bits/bit + gap)/ 4MHz = ~ 200ms
      // (31000 * 8bit * (8bits/bit + gap)/ 4MHz = ~ 600ms is safe
      // if ((i % 10000) == 0) yield(); // avoid watchdog reset
#endif
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
      _send8pixel(i < size ? pgm_read_byte(bitmap + i) : 0);
#else
      _send8pixel(i < size ? bitmap[i] : 0);
#endif
    }
    IO.writeCommandTransaction(0x12);      //display refresh
    _waitWhileBusy("drawBitmap");
    _sleep();
  }
}

void GxGDEW075T8::eraseDisplay(bool using_partial_update)
{
  if (using_partial_update)
  {
    _using_partial_mode = true; // remember
    _wakeUp();
    // set full screen
    IO.writeCommandTransaction(0x91); // partial in
    _setPartialRamArea(0, 0, GxGDEW075T8_WIDTH - 1, GxGDEW075T8_HEIGHT - 1);
    IO.writeCommandTransaction(0x10);
    for (uint32_t i = 0; i < GxGDEW075T8_BUFFER_SIZE; i++)
    {
      _send8pixel(0x00);
    }
    IO.writeCommandTransaction(0x12);      //display refresh
    _waitWhileBusy("eraseDisplay");
    IO.writeCommandTransaction(0x92); // partial out
  }
  else
  {
    _using_partial_mode = false; // remember
    _wakeUp();
    IO.writeCommandTransaction(0x10);
    for (uint32_t i = 0; i < GxGDEW075T8_BUFFER_SIZE; i++)
    {
      _send8pixel(0x00);
    }
    IO.writeCommandTransaction(0x12);      //display refresh
    _waitWhileBusy("eraseDisplay");
    _sleep();
  }
}

void GxGDEW075T8::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  if (using_rotation)
  {
    switch (getRotation())
    {
      case 1:
        swap(x, y);
        swap(w, h);
        x = GxGDEW075T8_WIDTH - x - w - 1;
        break;
      case 2:
        x = GxGDEW075T8_WIDTH - x - w - 1;
        y = GxGDEW075T8_HEIGHT - y - h - 1;
        break;
      case 3:
        swap(x, y);
        swap(w, h);
        y = GxGDEW075T8_HEIGHT - y  - h - 1;
        break;
    }
  }
  //fillScreen(0x0);
  if (x >= GxGDEW075T8_WIDTH) return;
  if (y >= GxGDEW075T8_HEIGHT) return;
  // x &= 0xFFF8; // byte boundary, not here, use encompassing rectangle
  uint16_t xe = min(GxGDEW075T8_WIDTH, x + w) - 1;
  uint16_t ye = min(GxGDEW075T8_HEIGHT, y + h) - 1;
  // x &= 0xFFF8; // byte boundary, not needed here
  uint16_t xs_bx = x / 8;
  uint16_t xe_bx = (xe + 7) / 8;
  //if (!_using_partial_mode) _wakeUp();
  if (!_using_partial_mode) eraseDisplay(true); // clean surrounding
  _using_partial_mode = true;
  IO.writeCommandTransaction(0x91); // partial in
  _setPartialRamArea(x, y, xe, ye);
  IO.writeCommandTransaction(0x10);
  for (int16_t y1 = y; y1 <= ye; y1++)
  {
    for (int16_t x1 = xs_bx; x1 < xe_bx; x1++)
    {
      uint16_t idx = y1 * (GxGDEW075T8_WIDTH / 8) + x1;
      _send8pixel((idx < sizeof(_buffer)) ? _buffer[idx] : 0x00);
    }
  }
  IO.writeCommandTransaction(0x12);      //display refresh
  _waitWhileBusy("updateWindow");
  IO.writeCommandTransaction(0x92); // partial out
}

void GxGDEW075T8::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye)
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
  //IO.writeDataTransaction(0x01); // don't see any difference
  IO.writeDataTransaction(0x00); // don't see any difference
}

void GxGDEW075T8::_waitWhileBusy(const char* comment)
{
  unsigned long start = micros();
  while (1)
  { //=0 BUSY
    if (digitalRead(_busy) == 1) break;
    delay(1);
  }
  if (comment)
  {
    //    unsigned long elapsed = micros() - start;
    //    Serial.print(comment);
    //    Serial.print(" : ");
    //    Serial.println(elapsed);
  }
}

void GxGDEW075T8::_send8pixel(uint8_t data)
{
  for (uint8_t j = 0; j < 8; j++)
  {
    uint8_t t = data & 0x80 ? 0x00 : 0x03;
    t <<= 4;
    data <<= 1;
    j++;
    t |= data & 0x80 ? 0x00 : 0x03;
    data <<= 1;
    IO.writeDataTransaction(t);
  }
}

void GxGDEW075T8::_wakeUp()
{
  digitalWrite(_rst, 0);
  delay(10);
  digitalWrite(_rst, 1);
  delay(10);

  /**********************************release flash sleep**********************************/
  IO.writeCommandTransaction(0X65);     //FLASH CONTROL
  IO.writeDataTransaction(0x01);

  IO.writeCommandTransaction(0xAB);

  IO.writeCommandTransaction(0X65);     //FLASH CONTROL
  IO.writeDataTransaction(0x00);
  /**********************************release flash sleep**********************************/
  IO.writeCommandTransaction(0x01);
  IO.writeDataTransaction (0x37);       //POWER SETTING
  IO.writeDataTransaction (0x00);

  IO.writeCommandTransaction(0X00);     //PANNEL SETTING
  IO.writeDataTransaction(0xCF);
  IO.writeDataTransaction(0x08);

  IO.writeCommandTransaction(0x06);     //boost
  IO.writeDataTransaction (0xc7);
  IO.writeDataTransaction (0xcc);
  IO.writeDataTransaction (0x28);

  IO.writeCommandTransaction(0x30);     //PLL setting
  IO.writeDataTransaction (0x3c);

  IO.writeCommandTransaction(0X41);     //TEMPERATURE SETTING
  IO.writeDataTransaction(0x00);

  IO.writeCommandTransaction(0X50);     //VCOM AND DATA INTERVAL SETTING
  IO.writeDataTransaction(0x77);

  IO.writeCommandTransaction(0X60);     //TCON SETTING
  IO.writeDataTransaction(0x22);

  IO.writeCommandTransaction(0x61);     //tres 640*384
  IO.writeDataTransaction (0x02);       //source 640
  IO.writeDataTransaction (0x80);
  IO.writeDataTransaction (0x01);       //gate 384
  IO.writeDataTransaction (0x80);

  IO.writeCommandTransaction(0X82);     //VDCS SETTING
  IO.writeDataTransaction(0x1E);        //decide by LUT file

  IO.writeCommandTransaction(0xe5);     //FLASH MODE
  IO.writeDataTransaction(0x03);

  IO.writeCommandTransaction(0x04);     //POWER ON
  _waitWhileBusy();
}

void GxGDEW075T8::_sleep(void)
{
  /**********************************flash sleep**********************************/
  IO.writeCommandTransaction(0X65);     //FLASH CONTROL
  IO.writeDataTransaction(0x01);

  IO.writeCommandTransaction(0xB9);

  IO.writeCommandTransaction(0X65);     //FLASH CONTROL
  IO.writeDataTransaction(0x00);
  /**********************************flash sleep**********************************/

  IO.writeCommandTransaction(0x02);     // POWER OFF
  _waitWhileBusy();

  IO.writeCommandTransaction(0x07);     // DEEP SLEEP
  IO.writeDataTransaction(0xa5);
}

void GxGDEW075T8::drawPaged(void (*drawCallback)(void))
{
  unsigned long start = micros();
  _wakeUp();
  IO.writeCommandTransaction(0x10);
  for (_current_page = 0; _current_page < GxGDEW075T8_PAGES; _current_page++)
  {
    fillScreen(0xFF);
    drawCallback();
    for (int16_t y1 = 0; y1 < GxGDEW075T8_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW075T8_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW075T8_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
        _send8pixel(data);
      }
    }
#if defined(ESP8266)
    yield();
#endif
  }
  _current_page = -1;
  IO.writeCommandTransaction(0x12);      //display refresh
  _waitWhileBusy("drawPaged refresh");
  _sleep();
  //  unsigned long elapsed = micros() - start;
  //  Serial.print("drawPaged total   : ");
  //  Serial.println(elapsed);
}

void GxGDEW075T8::drawCornerTest(uint8_t em)
{
  _wakeUp();
  IO.writeCommandTransaction(0x10);
  for (uint32_t y = 0; y < GxGDEW075T8_HEIGHT; y++)
  {
    for (uint32_t x = 0; x < GxGDEW075T8_WIDTH / 8; x++)
    {
      uint8_t data = 0xFF;
      if ((x < 1) && (y < 8)) data = 0x00;
      if ((x > GxGDEW075T8_WIDTH / 8 - 3) && (y < 16)) data = 0x00;
      if ((x > GxGDEW075T8_WIDTH / 8 - 4) && (y > GxGDEW075T8_HEIGHT - 25)) data = 0x00;
      if ((x < 4) && (y > GxGDEW075T8_HEIGHT - 33)) data = 0x00;
      _send8pixel(~data);
    }
  }
  IO.writeCommandTransaction(0x12);      //display refresh
  _waitWhileBusy("drawCornerTest");
  _sleep();
}

