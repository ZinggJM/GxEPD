/************************************************************************************
    class GxGDEW075Z09 : Display class example for GDEW075Z09 e-Paper from Dalian Good Display Co., Ltd.: www.good-display.com

    based on Demo Example from Good Display, available here: http://www.good-display.com/download_detail/downloadsId=526.html

    Author : J-M Zingg

    Contributor : Noobidoo

    Version : 2.0

    Support: minimal, provided as example only, as is, no claim to be fit for serious use

    Controller: IL0371 : http://www.good-display.com/download_detail/downloadsId=536.html

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
*/

#include "GxGDEW075Z09.h"

GxGDEW075Z09::GxGDEW075Z09(GxIO& io, uint8_t rst, uint8_t busy)
  : GxEPD(GxGDEW075Z09_WIDTH, GxGDEW075Z09_HEIGHT),
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

void GxGDEW075Z09::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GxGDEW075Z09_WIDTH - x - 1;
      break;
    case 2:
      x = GxGDEW075Z09_WIDTH - x - 1;
      y = GxGDEW075Z09_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GxGDEW075Z09_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 4 + y * GxGDEW075Z09_WIDTH / 4;
  if (i >= GxGDEW075Z09_BUFFER_SIZE) {
    return;
  }

  _buffer[i] = (_buffer[i] & (0xFF ^ (3 << 2 * (3 - x % 4))));
  switch (color) {
    case GxEPD_BLACK:
      break;
    case GxEPD_RED:
      _buffer[i] = (_buffer[i] | (1 << 2 * (3 - x % 4)));
      break;
    default:
      _buffer[i] = (_buffer[i] | (3 << 2 * (3 - x % 4)));
      break;
  }
}

void GxGDEW075Z09::init(void)
{
  IO.init();
  IO.setFrequency(4000000); // 4MHz : 250ns > 150ns min RD cycle
  digitalWrite(_rst, 1);
  pinMode(_rst, OUTPUT);
  pinMode(_busy, INPUT);
  fillScreen(GxEPD_WHITE);
}

void GxGDEW075Z09::fillScreen(uint16_t color)
{
  uint8_t data = 0xFF;
  switch (color) {
    case GxEPD_BLACK:
      data = 0x00;
      break;
    case GxEPD_RED:
      data = 0x55;
      break;
  }

  for (uint16_t x = 0; x < GxGDEW075Z09_BUFFER_SIZE; x++)
  {
    _buffer[x] = data;
  }
}

void GxGDEW075Z09::update(void)
{
  _wakeUp(true);
  IO.writeCommandTransaction(0x10);
  for (uint32_t i = 0; i < GxGDEW075Z09_OUTBUFFER_SIZE; i++)
  {

#if defined(ESP8266)
    // (10000 * 8bit * (8bits/bit + gap)/ 4MHz = ~ 200ms
    // (31000 * 8bit * (8bits/bit + gap)/ 4MHz = ~ 600ms is safe
    if ((i % 10000) == 0) yield(); // avoid watchdog reset
#endif
    uint8_t t1;
    if (i >= GxGDEW075Z09_BUFFER_SIZE) {
      t1 = 0x55;
    } else {
      t1 = _buffer[i];
    }
    for (uint8_t j = 0; j < 4; j++)
    {
      uint8_t t2 = t1 & 0xc0;
      if (t2 == 0xc0)
        t2 = 0x03; //011
      else if (t2 == 0x00)
        t2 = 0x00; //000
      else if (t2 == 0x40)
        t2 = 0x04; //100
      else
        t2 = 0x03; //011
      t2 <<= 4;
      t1 <<= 2;
      j++;
      uint8_t t3 = t1 & 0xc0;
      if (t3 == 0xc0)
        t2 |= 0x03; //011
      else if (t3 == 0x00)
        t2 |= 0x00; //000
      else if (t3 == 0x40)
        t2 |= 0x04; //100
      else
        t2 |= 0x03; //011
      t1 <<= 2;
      IO.writeDataTransaction(t2);
    }
  }
  IO.writeCommandTransaction(0x12);      //display refresh
  _waitWhileBusy("refresh");
  _sleep();
}

void  GxGDEW075Z09::drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color)
{
  drawBitmap(bitmap, x, y, w, h, color);
}

void  GxGDEW075Z09::drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode)
{
  if (mode == bm_default) mode = bm_normal;
  drawBitmapBM(bitmap, x, y, w, h, color, mode);
}

void GxGDEW075Z09::drawBitmap(const uint8_t *bitmap, uint32_t size, int16_t mode)
{
  //Serial.print("drawBitmap "); Serial.println(size);
  _wakeUp(true);
  IO.writeCommandTransaction(0x10);
  for (uint32_t i = 0; i < GxGDEW075Z09_OUTBUFFER_SIZE; i++)
  {
#if defined(ESP8266)
    // (10000 * 8bit * (8bits/bit + gap)/ 4MHz = ~ 200ms
    // (31000 * 8bit * (8bits/bit + gap)/ 4MHz = ~ 600ms is safe
    if ((i % 10000) == 0) yield(); // avoid watchdog reset
#endif
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
    uint8_t t1 = i < size ? pgm_read_byte(bitmap + i) : 0xFF;
#else
    uint8_t t1 = i < size ? bitmap[i] : 0xFF;
#endif
    for (uint8_t j = 0; j < 4; j++)
    {
      uint8_t t2 = t1 & 0xc0;
      if (t2 == 0xc0)
        t2 = 0x03; //011
      else if (t2 == 0x00)
        t2 = 0x00; //000
      else if (t2 == 0x40)
        t2 = 0x04; //100
      else
        t2 = 0x03; //011
      t2 <<= 4;
      t1 <<= 2;
      j++;
      uint8_t t3 = t1 & 0xc0;
      if (t3 == 0xc0)
        t2 |= 0x03; //011
      else if (t3 == 0x00)
        t2 |= 0x00; //000
      else if (t3 == 0x40)
        t2 |= 0x04; //100
      else
        t2 |= 0x03; //011
      t1 <<= 2;
      IO.writeDataTransaction(t2);
    }
  }
  IO.writeCommandTransaction(0x12);      //display refresh
  _waitWhileBusy("refresh");
  _sleep();
}

void GxGDEW075Z09::_waitWhileBusy(const char* comment)
{
  unsigned long start = micros();
  while (1)
  { //=0 BUSY
    if (digitalRead(_busy) == 1) break;
    delay(1);
  }
  //if (comment)
  {
//    unsigned long elapsed = micros() - start;
//    Serial.print(comment);
//    Serial.print(" : ");
//    Serial.println(elapsed);
  }
}

void GxGDEW075Z09::_wakeUp(bool partial)
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
  _waitWhileBusy("POWER");

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

void GxGDEW075Z09::_sleep(void)
{
  /**********************************flash sleep**********************************/
  IO.writeCommandTransaction(0X65);     //FLASH CONTROL
  IO.writeDataTransaction(0x01);

  IO.writeCommandTransaction(0xB9);

  IO.writeCommandTransaction(0X65);     //FLASH CONTROL
  IO.writeDataTransaction(0x00);
  /**********************************flash sleep**********************************/

  IO.writeCommandTransaction(0x02);     // POWER OFF
  _waitWhileBusy("Power off");

  IO.writeCommandTransaction(0x07);     // DEEP SLEEP
  IO.writeDataTransaction(0xa5);
}
