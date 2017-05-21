/************************************************************************************
   class GxGDEW075T8 : Display class example for GDEW075T8 e-Paper from GoodDisplay.com

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

#include "GxGDEW075T8.h"

GxGDEW075T8::GxGDEW075T8(GxIO& io, uint8_t rst, uint8_t busy)
  : GxEPD(GxGDEW075T8_WIDTH, GxGDEW075T8_HEIGHT),
    IO(io), _rst(rst), _busy(busy)
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

  if (!color)
    _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
  else
    _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
}

void GxGDEW075T8::init(void)
{
  IO.init();
  IO.setFrequency(4000000); // 4MHz : 250ns > 150ns min RD cycle
  digitalWrite(_rst, 1);
  pinMode(_rst, OUTPUT);
  pinMode(_busy, INPUT);
  fillScreen(GxEPD_WHITE);
}

void GxGDEW075T8::fillScreen(uint16_t color)
{
  uint8_t data = (color == GxEPD_BLACK) ? 0xFF : 0x00;
  for (uint16_t x = 0; x < GxGDEW075T8_BUFFER_SIZE; x++)
  {
    _buffer[x] = data;
  }
}

void GxGDEW075T8::update(void)
{
  _wakeUp(true);
  IO.writeCommandTransaction(0x10);
  for (uint32_t i = 0; i < GxGDEW075T8_BUFFER_SIZE; i++)
  {
#if defined(ESP8266)
    // (10000 * 8bit * (8bits/bit + gap)/ 4MHz = ~ 200ms
    // (31000 * 8bit * (8bits/bit + gap)/ 4MHz = ~ 600ms is safe
    // if ((i % 10000) == 0) yield(); // avoid watchdog reset
#endif
    uint8_t t1 = _buffer[i];
    for (uint8_t j = 0; j < 8; j++)
    {
      uint8_t t2 = t1 & 0x80 ? 0x00 : 0x03;
      t2 <<= 4;
      t1 <<= 1;
      j++;
      t2 |= t1 & 0x80 ? 0x00 : 0x03;
      t1 <<= 1;
      IO.writeDataTransaction(t2);
    }
  }
  //IO.writeCommandTransaction(0x04);        //POWER ON
  //_waitWhileBusy();
  IO.writeCommandTransaction(0x12);      //display refresh
  _waitWhileBusy();
  _sleep();
}

void GxGDEW075T8::drawBitmap(const uint8_t *bitmap, uint16_t size)
{
  Serial.print("drawBitmap "); Serial.println(size);
  _wakeUp(true);
  IO.writeCommandTransaction(0x10);
  for (uint32_t i = 0; i < GxGDEW075T8_BUFFER_SIZE; i++)
  {
#if defined(ESP8266)
    // (10000 * 8bit * (8bits/bit + gap)/ 4MHz = ~ 200ms
    // (31000 * 8bit * (8bits/bit + gap)/ 4MHz = ~ 600ms is safe
    // if ((i % 10000) == 0) yield(); // avoid watchdog reset
#endif
    uint8_t t1 = i < size ? bitmap[i] : 0;
    for (uint8_t j = 0; j < 8; j++)
    {
      uint8_t t2 = t1 & 0x80 ? 0x00 : 0x03;
      t2 <<= 4;
      t1 <<= 1;
      j++;
      t2 |= t1 & 0x80 ? 0x00 : 0x03;
      t1 <<= 1;
      IO.writeDataTransaction(t2);
    }
  }
  IO.writeCommandTransaction(0x12);      //display refresh
  _waitWhileBusy();
  _sleep();
}

void  GxGDEW075T8::drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color)
{
  for (uint16_t x1 = x; x1 < x + w; x1++)
  {
    for (uint16_t y1 = y; y1 < y + h; y1++)
    {
      uint16_t i = x1 / 8 + y1 * w / 8;
      uint16_t pixelcolor = (bitmap[i] & (0x80 >> x1 % 8)) ? color : GxEPD_WHITE;
      drawPixel(x1, y1, pixelcolor);
    }
  }
}

void GxGDEW075T8::_waitWhileBusy(const char* comment)
{
  unsigned long start = micros();
  while (1)
  { //=0 BUSY
    if (digitalRead(BSY) == 1) break;
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

void GxGDEW075T8::_wakeUp(bool partial)
{
  digitalWrite(_rst, 0);
  delay(100);
  digitalWrite(_rst, 1);
  delay(200);

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

  IO.writeCommandTransaction(0x04);     //POWER ON
  _waitWhileBusy();

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
  while (!digitalRead(BSY));

  IO.writeCommandTransaction(0x07);     // DEEP SLEEP
  IO.writeDataTransaction(0xa5);
}


