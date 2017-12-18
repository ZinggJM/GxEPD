/************************************************************************************
   class GxGDEW042Z15 : Display class example for GDEW042Z15 e-Paper from Dalian Good Display Co., Ltd.: www.good-display.com

   based on Demo Example from Good Display, available here: http://www.good-display.com/download_detail/downloadsId=524.html

   Author : J-M Zingg

   Version : 2.2

   Support: limited, provided as example, no claim to be fit for serious use

   Controller: IL91874 : http://www.good-display.com/download_detail/downloadsId=539.html

   connection to the e-Paper display is through DESTM32-S2 connection board, available from Good Display

   DESTM32-S2 pinout (top, component side view):
         |-------------------------------------------------
         |  VCC  |o o| VCC 5V  not needed
         |  GND  |o o| GND
         |  3.3  |o o| 3.3     3.3V
         |  nc   |o o| nc
         |  nc   |o o| nc
         |  nc   |o o| nc
   MOSI  |  DIN  |o o| CLK     SCK
   SS    |  CS   |o o| DC      e.g. D3
   D4    |  RST  |o o| BUSY    e.g. D2
         |  nc   |o o| BS      GND
         |-------------------------------------------------

   note: for correct red color jumper J3 must be set on 0.47 side (towards FCP connector)

*/
#include "GxGDEW042Z15.h"

#if defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

GxGDEW042Z15::GxGDEW042Z15(GxIO& io, uint8_t rst, uint8_t busy)
  : GxEPD(GxGDEW042Z15_WIDTH, GxGDEW042Z15_HEIGHT),
    IO(io), _current_page(-1), 
    _rst(rst), _busy(busy) 
{
}

template <typename T> static inline void
swap(T& a, T& b)
{
  T t = a;
  a = b;
  b = t;
}

void GxGDEW042Z15::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GxGDEW042Z15_WIDTH - x - 1;
      break;
    case 2:
      x = GxGDEW042Z15_WIDTH - x - 1;
      y = GxGDEW042Z15_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GxGDEW042Z15_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GxGDEW042Z15_WIDTH / 8;
  if (_current_page < 1)
  {
    if (i >= sizeof(_black_buffer)) return;
  }
  else
  {
    y -= _current_page * GxGDEW042Z15_PAGE_HEIGHT;
    if ((y < 0) || (y >= GxGDEW042Z15_PAGE_HEIGHT)) return;
    i = x / 8 + y * GxGDEW042Z15_WIDTH / 8;
  }

  _black_buffer[i] = (_black_buffer[i] | (1 << (7 - x % 8))); // white
  _red_buffer[i] = (_red_buffer[i] | (1 << (7 - x % 8))); // white
  if (color == GxEPD_WHITE) return;
  else if (color == GxEPD_BLACK) _black_buffer[i] = (_black_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
  else if (color == GxEPD_RED) _red_buffer[i] = (_red_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
}


void GxGDEW042Z15::init(void)
{
  IO.init();
  IO.setFrequency(4000000); // 4MHz : 250ns > 150ns min RD cycle
  digitalWrite(_rst, 1);
  pinMode(_rst, OUTPUT);
  pinMode(_busy, INPUT);
  fillScreen(GxEPD_WHITE);
  _current_page = -1;
}

void GxGDEW042Z15::fillScreen(uint16_t color)
{
  uint8_t black = 0xFF;
  uint8_t red = 0xFF;
  if (color == GxEPD_WHITE);
  else if (color == GxEPD_BLACK) black = 0x00;
  else if (color == GxEPD_RED) red = 0x00;
  for (uint16_t x = 0; x < sizeof(_black_buffer); x++)
  {
    _black_buffer[x] = black;
    _red_buffer[x] = red;
  }
}

void GxGDEW042Z15::update(void)
{
  if (_current_page != -1) return;
  _wakeUp();
  _writeCommand(0x10);
  for (uint32_t i = 0; i < GxGDEW042Z15_BUFFER_SIZE; i++)
  {
    _writeData((i < sizeof(_black_buffer)) ? _black_buffer[i] : 0x00);
  }
  _writeCommand(0x13);
  for (uint32_t i = 0; i < GxGDEW042Z15_BUFFER_SIZE; i++)
  {
    _writeData((i < sizeof(_red_buffer)) ? _red_buffer[i] : 0x00);
  }
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("update");
  _sleep();
}

void  GxGDEW042Z15::drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode)
{
  if (mode & bm_default) mode |= bm_normal; // no change
  drawBitmapBM(bitmap, x, y, w, h, color, mode);
}

void GxGDEW042Z15::drawExamplePicture(const uint8_t* black_bitmap, const uint8_t* red_bitmap, uint32_t black_size, uint32_t red_size)
{
  drawPicture(black_bitmap, red_bitmap, black_size, red_size, bm_normal);
}

void GxGDEW042Z15::drawPicture(const uint8_t* black_bitmap, const uint8_t* red_bitmap, uint32_t black_size, uint32_t red_size, int16_t mode)
{
  if (_current_page != -1) return;
  _wakeUp();
  _writeCommand(0x10);
  for (uint32_t i = 0; i < GxGDEW042Z15_BUFFER_SIZE; i++)
  {
    uint8_t data = 0x00; // white is 0x00 on device
    if (i < black_size)
    {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
      data = pgm_read_byte(&black_bitmap[i]);
#else
      data = black_bitmap[i];
#endif
      if (mode & bm_invert) data = ~data;
    }
    _writeData(data);
  }
  _writeCommand(0x13);
  for (uint32_t i = 0; i < GxGDEW042Z15_BUFFER_SIZE; i++)
  {
    uint8_t data = 0x00; // white is 0x00 on device
    if (i < red_size)
    {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
      data = pgm_read_byte(&red_bitmap[i]);
#else
      data = red_bitmap[i];
#endif
      if (mode & bm_invert_red) data = ~data;
    }
    _writeData(data);
  }
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("drawPicture");
  _sleep();
}

void GxGDEW042Z15::drawBitmap(const uint8_t* bitmap, uint32_t size, int16_t mode)
{
  if (_current_page != -1) return;
  if (mode & bm_default) mode |= bm_normal; // no change
  _wakeUp();
  _writeCommand(0x10);
  for (uint32_t i = 0; i < GxGDEW042Z15_BUFFER_SIZE; i++)
  {
    uint8_t data = 0x00; // white is 0x00 on device
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
  for (uint32_t i = 0; i < GxGDEW042Z15_BUFFER_SIZE; i++)
  {
    _writeData(0);
  }
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("drawBitmap");
  _sleep();
}

void GxGDEW042Z15::eraseDisplay(bool using_partial_update)
{
  if (_current_page != -1) return;
  _wakeUp();
  _writeCommand(0x10);
  for (uint32_t i = 0; i < GxGDEW042Z15_BUFFER_SIZE; i++)
  {
    _writeData(0x00);
  }
  _writeCommand(0x13);
  for (uint32_t i = 0; i < GxGDEW042Z15_BUFFER_SIZE; i++)
  {
    _writeData(0x00);
  }
  _writeCommand(0x12);      //display refresh
  _waitWhileBusy("eraseDisplay");
  _sleep();
}

void GxGDEW042Z15::powerDown()
{
  _sleep();
}

void GxGDEW042Z15::_writeCommand(uint8_t command)
{
  if (!digitalRead(_busy))
  {
    String str = String("command 0x") + String(command, HEX);
    _waitWhileBusy(str.c_str());
  }
  IO.writeCommandTransaction(command);
}

void GxGDEW042Z15::_writeData(uint8_t data)
{
  IO.writeDataTransaction(data);
}

void GxGDEW042Z15::_waitWhileBusy(const char* comment)
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
    //unsigned long elapsed = micros() - start;
    //Serial.print(comment);
    //Serial.print(" : ");
    //Serial.println(elapsed);
  }
}

void GxGDEW042Z15::_wakeUp()
{
  digitalWrite(_rst, 0);
  delay(10);
  digitalWrite(_rst, 1);
  delay(10);

  _writeCommand(0x06);
  _writeData(0x17);
  _writeData(0x17);
  _writeData(0x17);
  _writeCommand(0x04);
  _waitWhileBusy("Power On");
  _writeCommand(0x00);
  _writeData(0x0F);
  _writeCommand(0x30); // PLL
  _writeData(0x39); // 3A 100HZ   29 150Hz 39 200HZ 31 171HZ
}

void GxGDEW042Z15::_sleep(void)
{
  _writeCommand(0x50); // border floating
  _writeData(0xF7);
  _writeCommand(0x02); // power off
  _waitWhileBusy("Power Off");
  _writeCommand(0x07); // deep sleep
  _writeData(0xA5);
}

void GxGDEW042Z15::drawPaged(void (*drawCallback)(void))
{
  if (_current_page != -1) return;
  _wakeUp();
  _writeCommand(0x10);
  for (_current_page = 0; _current_page < GxGDEW042Z15_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback();
    for (int16_t y1 = 0; y1 < GxGDEW042Z15_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW042Z15_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW042Z15_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0x00;
        _writeData(data);
      }
    }
  }
  _writeCommand(0x13);
  for (_current_page = 0; _current_page < GxGDEW042Z15_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback();
    for (int16_t y1 = 0; y1 < GxGDEW042Z15_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW042Z15_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW042Z15_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_red_buffer)) ? _red_buffer[idx] : 0x00;
        _writeData(data);
      }
    }
  }
  _current_page = -1;
  _writeCommand(0x12);      //display refresh
  _waitWhileBusy("drawPaged");
  _sleep();
}

void GxGDEW042Z15::drawPaged(void (*drawCallback)(uint32_t), uint32_t p)
{
  if (_current_page != -1) return;
  _wakeUp();
  _writeCommand(0x10);
  for (_current_page = 0; _current_page < GxGDEW042Z15_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p);
    for (int16_t y1 = 0; y1 < GxGDEW042Z15_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW042Z15_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW042Z15_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0x00;
        _writeData(data);
      }
    }
  }
  _writeCommand(0x13);
  for (_current_page = 0; _current_page < GxGDEW042Z15_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p);
    for (int16_t y1 = 0; y1 < GxGDEW042Z15_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW042Z15_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW042Z15_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_red_buffer)) ? _red_buffer[idx] : 0x00;
        _writeData(data);
      }
    }
  }
  _current_page = -1;
  _writeCommand(0x12);      //display refresh
  _waitWhileBusy("drawPaged");
  _sleep();
}

void GxGDEW042Z15::drawPaged(void (*drawCallback)(const void*), const void* p)
{
  if (_current_page != -1) return;
  _wakeUp();
  _writeCommand(0x10);
  for (_current_page = 0; _current_page < GxGDEW042Z15_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p);
    for (int16_t y1 = 0; y1 < GxGDEW042Z15_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW042Z15_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW042Z15_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0x00;
        _writeData(data);
      }
    }
  }
  _writeCommand(0x13);
  for (_current_page = 0; _current_page < GxGDEW042Z15_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p);
    for (int16_t y1 = 0; y1 < GxGDEW042Z15_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW042Z15_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW042Z15_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_red_buffer)) ? _red_buffer[idx] : 0x00;
        _writeData(data);
      }
    }
  }
  _current_page = -1;
  _writeCommand(0x12);      //display refresh
  _waitWhileBusy("drawPaged");
  _sleep();
}

void GxGDEW042Z15::drawPaged(void (*drawCallback)(const void*, const void*), const void* p1, const void* p2)
{
  if (_current_page != -1) return;
  _wakeUp();
  _writeCommand(0x10);
  for (_current_page = 0; _current_page < GxGDEW042Z15_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p1, p2);
    for (int16_t y1 = 0; y1 < GxGDEW042Z15_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW042Z15_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW042Z15_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0x00;
        _writeData(data);
      }
    }
  }
  _writeCommand(0x13);
  for (_current_page = 0; _current_page < GxGDEW042Z15_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p1, p2);
    for (int16_t y1 = 0; y1 < GxGDEW042Z15_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW042Z15_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW042Z15_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_red_buffer)) ? _red_buffer[idx] : 0x00;
        _writeData(data);
      }
    }
  }
  _current_page = -1;
  _writeCommand(0x12);      //display refresh
  _waitWhileBusy("drawPaged");
  _sleep();
}

void GxGDEW042Z15::drawCornerTest(uint8_t em)
{
  if (_current_page != -1) return;
  _wakeUp();
  _writeCommand(0x10);
  for (uint32_t y = 0; y < GxGDEW042Z15_HEIGHT; y++)
  {
    for (uint32_t x = 0; x < GxGDEW042Z15_WIDTH / 8; x++)
    {
      uint8_t data = 0xFF;
      if ((x < 1) && (y < 8)) data = 0x00;
      if ((x > GxGDEW042Z15_WIDTH / 8 - 3) && (y < 16)) data = 0x00;
      if ((x > GxGDEW042Z15_WIDTH / 8 - 4) && (y > GxGDEW042Z15_HEIGHT - 25)) data = 0x00;
      if ((x < 4) && (y > GxGDEW042Z15_HEIGHT - 33)) data = 0x00;
      _writeData(~data);
    }
  }
  _writeCommand(0x13);
  for (uint32_t i = 0; i < GxGDEW042Z15_BUFFER_SIZE; i++)
  {
    _writeData(0);
  }
  _writeCommand(0x12);      //display refresh
  _waitWhileBusy("drawCornerTest");
  _sleep();
}
