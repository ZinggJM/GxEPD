/************************************************************************************
   class GxGDEW042T2 : Display class example for GxGDEW042T2 e-Paper from Dalian Good Display Co., Ltd.: www.good-display.com

   based on Demo Example from Good Display, now available on http://www.good-display.com/download_list/downloadcategoryid=34&isMode=false.html

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

#include "GxGDEW042T2.h"

const uint8_t lut_vcom0[] =
{
  0x00  , 0x17  , 0x00  , 0x00  , 0x00  , 0x02,
  0x00  , 0x17  , 0x17  , 0x00  , 0x00  , 0x02,
  0x00  , 0x0A  , 0x01  , 0x00  , 0x00  , 0x01,
  0x00  , 0x0E  , 0x0E  , 0x00  , 0x00  , 0x02,
  0x00  , 0x00  , 0x00  , 0x00  , 0x00  , 0x00,
  0x00  , 0x00  , 0x00  , 0x00  , 0x00  , 0x00,
  0x00  , 0x00  , 0x00  , 0x00  , 0x00  , 0x00  , 0x00  , 0x00,

};
const uint8_t lut_ww[] = {
  0x40  , 0x17  , 0x00  , 0x00  , 0x00  , 0x02,
  0x90  , 0x17  , 0x17  , 0x00  , 0x00  , 0x02,
  0x40  , 0x0A  , 0x01  , 0x00  , 0x00  , 0x01,
  0xA0  , 0x0E  , 0x0E  , 0x00  , 0x00  , 0x02,
  0x00  , 0x00  , 0x00  , 0x00  , 0x00  , 0x00,
  0x00  , 0x00  , 0x00  , 0x00  , 0x00  , 0x00,
  0x00  , 0x00  , 0x00  , 0x00  , 0x00  , 0x00,

};
const uint8_t lut_bw[] = {
  0x40  , 0x17  , 0x00  , 0x00  , 0x00  , 0x02,
  0x90  , 0x17  , 0x17  , 0x00  , 0x00  , 0x02,
  0x40  , 0x0A  , 0x01  , 0x00  , 0x00  , 0x01,
  0xA0  , 0x0E  , 0x0E  , 0x00  , 0x00  , 0x02,
  0x00  , 0x00  , 0x00  , 0x00  , 0x00  , 0x00,
  0x00  , 0x00  , 0x00  , 0x00  , 0x00  , 0x00,
  0x00  , 0x00  , 0x00  , 0x00  , 0x00  , 0x00,

};

const uint8_t lut_bb[] = {
  0x80  , 0x17  , 0x00  , 0x00  , 0x00  , 0x02,
  0x90  , 0x17  , 0x17  , 0x00  , 0x00  , 0x02,
  0x80  , 0x0A  , 0x01  , 0x00  , 0x00  , 0x01,
  0x50  , 0x0E  , 0x0E  , 0x00  , 0x00  , 0x02,
  0x00  , 0x00  , 0x00  , 0x00  , 0x00  , 0x00,
  0x00  , 0x00  , 0x00  , 0x00  , 0x00  , 0x00,
  0x00  , 0x00  , 0x00  , 0x00  , 0x00  , 0x00,

};

const uint8_t lut_wb[] = {
  0x80  , 0x17  , 0x00  , 0x00  , 0x00  , 0x02,
  0x90  , 0x17  , 0x17  , 0x00  , 0x00  , 0x02,
  0x80  , 0x0A  , 0x01  , 0x00  , 0x00  , 0x01,
  0x50  , 0x0E  , 0x0E  , 0x00  , 0x00  , 0x02,
  0x00  , 0x00  , 0x00  , 0x00  , 0x00  , 0x00,
  0x00  , 0x00  , 0x00  , 0x00  , 0x00  , 0x00,
  0x00  , 0x00  , 0x00  , 0x00  , 0x00  , 0x00,

};

GxGDEW042T2::GxGDEW042T2(GxIO& io, uint8_t rst, uint8_t busy)
  : GxEPD(GxGDEW042T2_WIDTH, GxGDEW042T2_HEIGHT),
    IO(io), _rst(rst), _busy(busy)
{
  // do not init hw here, doesn't work
}

template <typename T> static inline void
swap(T& a, T& b)
{
  T t = a;
  a = b;
  b = t;
}

void GxGDEW042T2::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GxGDEW042T2_WIDTH - x - 1;
      break;
    case 2:
      x = GxGDEW042T2_WIDTH - x - 1;
      y = GxGDEW042T2_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GxGDEW042T2_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GxGDEW042T2_WIDTH / 8;

  if (!color)
    _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
  else
    _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
}

void GxGDEW042T2::init(void)
{
  IO.init();
  IO.setFrequency(4000000); // 4MHz : 250ns > 150ns min RD cycle
  digitalWrite(_rst, 1);
  pinMode(_rst, OUTPUT);
  pinMode(_busy, INPUT);
  fillScreen(GxEPD_WHITE);
}

void GxGDEW042T2::fillScreen(uint16_t color)
{
  uint8_t data = (color == GxEPD_BLACK) ? 0xFF : 0x00;
  for (uint16_t x = 0; x < GxGDEW042T2_BUFFER_SIZE; x++)
  {
    _buffer[x] = data;
  }
}

void GxGDEW042T2::update(void)
{
  uint32_t i;
  uint8_t data;
  _wakeUp();
  IO.writeCommandTransaction(0x13);
  for (i = 0; i < GxGDEW042T2_BUFFER_SIZE; i++)
  {
    data = _buffer[i];
    IO.writeDataTransaction(~data);
  }
  IO.writeCommandTransaction(0x12);      //display refresh
  _waitWhileBusy("update display refresh");
  _sleep();
}

void  GxGDEW042T2::drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color)
{
  drawBitmap(bitmap, x, y, w, h, color);
}

void  GxGDEW042T2::drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, bool mirror)
{
  if (mirror)
  {
    for (uint16_t x1 = x; x1 < x + w; x1++)
    {
      for (uint16_t y1 = y; y1 < y + h; y1++)
      {
        uint32_t i = (w - (x1 - x) - 1) / 8 + uint32_t(y1 - y) * uint32_t(w) / 8;
        uint16_t pixelcolor = (bitmap[i] & (0x01 << (x1 - x) % 8)) ? GxEPD_WHITE  : color;
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
        uint16_t pixelcolor = (bitmap[i] & (0x80 >> (x1 - x) % 8)) ? GxEPD_WHITE  : color;
        drawPixel(x1, y1, pixelcolor);
      }
    }
  }
}

void GxGDEW042T2::drawBitmap(const uint8_t *bitmap, uint32_t size)
{
  uint32_t i;
  uint8_t data;
  _wakeUp();
  IO.writeCommandTransaction(0x13);
  for (i = 0; i < GxGDEW042T2_BUFFER_SIZE; i++)
  {
    data = (i < size) ? bitmap[i] : 0xFF;
    IO.writeDataTransaction(data);
  }
  IO.writeCommandTransaction(0x12);      //display refresh
  _waitWhileBusy("update display refresh");
  _sleep();
}

void GxGDEW042T2::_waitWhileBusy(const char* comment)
{
  unsigned long start = micros();
  while (1)
  { //=0 BUSY
    if (digitalRead(BSY) == 1) break;
    delay(1);
  }
  if (comment)
  {
    unsigned long elapsed = micros() - start;
    //    Serial.print(comment);
    //    Serial.print(" : ");
    //    Serial.println(elapsed);
  }
}

void GxGDEW042T2::_wakeUp(void)
{
  digitalWrite(_rst, 0);
  delay(100);
  digitalWrite(_rst, 1);
  delay(100);

  IO.writeCommandTransaction(0x01);
  IO.writeDataTransaction (0x03);
  IO.writeDataTransaction (0x00);
  IO.writeDataTransaction (0x2b);
  IO.writeDataTransaction (0x2b);
  IO.writeDataTransaction (0xff);

  IO.writeCommandTransaction(0x06);
  IO.writeDataTransaction (0x17);
  IO.writeDataTransaction (0x17);
  IO.writeDataTransaction (0x17);

  IO.writeCommandTransaction(0x04);
  _waitWhileBusy("_wakeUp Power On");

  IO.writeCommandTransaction(0x00);
  IO.writeDataTransaction(0xbf);
  IO.writeDataTransaction(0x0b);

  IO.writeCommandTransaction(0x30);
  IO.writeDataTransaction (0x3c);

  IO.writeCommandTransaction(0x61);
  IO.writeDataTransaction (0x01);
  IO.writeDataTransaction (0x90);
  IO.writeDataTransaction (0x01);
  IO.writeDataTransaction (0x2c);

  IO.writeCommandTransaction(0x82);
  IO.writeDataTransaction (0x12);

  IO.writeCommandTransaction(0X50);
  IO.writeDataTransaction(0x97);
  _writeLUT();
}

void GxGDEW042T2::_sleep(void)
{
  IO.writeCommandTransaction(0X50);
  IO.writeDataTransaction(0x17);

  IO.writeCommandTransaction(0X82);
  IO.writeCommandTransaction(0X00);

  IO.writeCommandTransaction(0x01);
  IO.writeDataTransaction (0x00);
  IO.writeDataTransaction (0x00);
  IO.writeDataTransaction (0x00);
  IO.writeDataTransaction (0x00);
  IO.writeDataTransaction (0x00);

  IO.writeCommandTransaction(0X02);
  _waitWhileBusy("Power Off");
  IO.writeCommandTransaction(0X07);
  IO.writeDataTransaction(0xA5);
}

void GxGDEW042T2::_writeLUT(void)
{
  uint16_t count;
  IO.writeCommandTransaction(0x20);
  for (count = 0; count < 44; count++)
  {
    IO.writeDataTransaction(lut_vcom0[count]);
  }

  IO.writeCommandTransaction(0x21);
  for (count = 0; count < 42; count++)
  {
    IO.writeDataTransaction(lut_ww[count]);
  }

  IO.writeCommandTransaction(0x22);
  for (count = 0; count < 42; count++)
  {
    IO.writeDataTransaction(lut_bw[count]);
  }

  IO.writeCommandTransaction(0x23);
  for (count = 0; count < 42; count++)
  {
    IO.writeDataTransaction(lut_wb[count]);
  }

  IO.writeCommandTransaction(0x24);
  for (count = 0; count < 42; count++)
  {
    IO.writeDataTransaction(lut_bb[count]);
  }
}

void GxGDEW042T2::greyTest() // what do the 2 channels provide ?
{
  _wakeUp();
  IO.writeCommandTransaction(0x10);
  for (uint16_t y = 0; y < GxGDEW042T2_HEIGHT; y++)
  {
    for (uint16_t x8 = 0; x8 < GxGDEW042T2_WIDTH / 8; x8++)
    {
      uint8_t data = (x8 < GxGDEW042T2_WIDTH / 8 / 2) ? 0xFF : 0x00;
      IO.writeDataTransaction(data);
    }
  }
  IO.writeCommandTransaction(0x13);
  for (uint16_t y = 0; y < GxGDEW042T2_HEIGHT; y++)
  {
    for (uint16_t x8 = 0; x8 < GxGDEW042T2_WIDTH / 8; x8++)
    {
      uint8_t data = (y <  GxGDEW042T2_HEIGHT / 2) ? 0xFF : 0x00;
      IO.writeDataTransaction(data);
    }
  }
  IO.writeCommandTransaction(0x12);      //display refresh
  _waitWhileBusy("greyTest display refresh");
  _sleep();
}

