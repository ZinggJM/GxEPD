/************************************************************************************
   class GxGDEW027C44 : Display class example for GDEW027C44 e-Paper from Dalian Good Display Co., Ltd.: www.good-display.com

   based on Demo Example from Good Display, available here: http://www.good-display.com/download_detail/downloadsId=519.html

   Author : J-M Zingg

   Version : 2.3

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
#include "GxGDEW027C44.h"

//#define DISABLE_DIAGNOSTIC_OUTPUT

#if defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

const uint8_t GxGDEW027C44::lut_vcomDC[] =
{
  0x00	, 0x00,
  0x00	, 0x1A	, 0x1A	, 0x00	, 0x00	, 0x01,
  0x00	, 0x0A	, 0x0A	, 0x00	, 0x00	, 0x08,
  0x00	, 0x0E	, 0x01	, 0x0E	, 0x01	, 0x10,
  0x00	, 0x0A	, 0x0A	, 0x00	, 0x00	, 0x08,
  0x00	, 0x04	, 0x10	, 0x00	, 0x00	, 0x05,
  0x00	, 0x03	, 0x0E	, 0x00	, 0x00	, 0x0A,
  0x00	, 0x23	, 0x00	, 0x00	, 0x00	, 0x01
};
//R21H
const uint8_t GxGDEW027C44::lut_ww[] = {
  0x90	, 0x1A	, 0x1A	, 0x00	, 0x00	, 0x01,
  0x40	, 0x0A	, 0x0A	, 0x00	, 0x00	, 0x08,
  0x84	, 0x0E	, 0x01	, 0x0E	, 0x01	, 0x10,
  0x80	, 0x0A	, 0x0A	, 0x00	, 0x00	, 0x08,
  0x00	, 0x04	, 0x10	, 0x00	, 0x00	, 0x05,
  0x00	, 0x03	, 0x0E	, 0x00	, 0x00	, 0x0A,
  0x00	, 0x23	, 0x00	, 0x00	, 0x00	, 0x01
};
//R22H	r
const uint8_t GxGDEW027C44::lut_bw[] = {
  0xA0	, 0x1A	, 0x1A	, 0x00	, 0x00	, 0x01,
  0x00	, 0x0A	, 0x0A	, 0x00	, 0x00	, 0x08,
  0x84	, 0x0E	, 0x01	, 0x0E	, 0x01	, 0x10,
  0x90	, 0x0A	, 0x0A	, 0x00	, 0x00	, 0x08,
  0xB0	, 0x04	, 0x10	, 0x00	, 0x00	, 0x05,
  0xB0	, 0x03	, 0x0E	, 0x00	, 0x00	, 0x0A,
  0xC0	, 0x23	, 0x00	, 0x00	, 0x00	, 0x01
};
//R23H	w
const uint8_t GxGDEW027C44::lut_bb[] = {
  0x90	, 0x1A	, 0x1A	, 0x00	, 0x00	, 0x01,
  0x40	, 0x0A	, 0x0A	, 0x00	, 0x00	, 0x08,
  0x84	, 0x0E	, 0x01	, 0x0E	, 0x01	, 0x10,
  0x80	, 0x0A	, 0x0A	, 0x00	, 0x00	, 0x08,
  0x00	, 0x04	, 0x10	, 0x00	, 0x00	, 0x05,
  0x00	, 0x03	, 0x0E	, 0x00	, 0x00	, 0x0A,
  0x00	, 0x23	, 0x00	, 0x00	, 0x00	, 0x01
};
//R24H	b
const uint8_t GxGDEW027C44::lut_wb[] = {
  0x90	, 0x1A	, 0x1A	, 0x00	, 0x00	, 0x01,
  0x20	, 0x0A	, 0x0A	, 0x00	, 0x00	, 0x08,
  0x84	, 0x0E	, 0x01	, 0x0E	, 0x01	, 0x10,
  0x10	, 0x0A	, 0x0A	, 0x00	, 0x00	, 0x08,
  0x00	, 0x04	, 0x10	, 0x00	, 0x00	, 0x05,
  0x00	, 0x03	, 0x0E	, 0x00	, 0x00	, 0x0A,
  0x00	, 0x23	, 0x00	, 0x00	, 0x00	, 0x01
};

GxGDEW027C44::GxGDEW027C44(GxIO& io, int8_t rst, int8_t busy)
  : GxEPD(GxGDEW027C44_WIDTH, GxGDEW027C44_HEIGHT),
    IO(io), _current_page(-1),
    _rst(rst), _busy(busy)
{
}

void GxGDEW027C44::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GxGDEW027C44_WIDTH - x - 1;
      break;
    case 2:
      x = GxGDEW027C44_WIDTH - x - 1;
      y = GxGDEW027C44_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GxGDEW027C44_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GxGDEW027C44_WIDTH / 8;
  if (_current_page < 1)
  {
    if (i >= sizeof(_black_buffer)) return;
  }
  else
  {
    y -= _current_page * GxGDEW027C44_PAGE_HEIGHT;
    if ((y < 0) || (y >= GxGDEW027C44_PAGE_HEIGHT)) return;
    i = x / 8 + y * GxGDEW027C44_WIDTH / 8;
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


void GxGDEW027C44::init(void)
{
  IO.init();
  IO.setFrequency(4000000); // 4MHz : 250ns > 150ns min RD cycle
  if (_rst >= 0)
  {
    digitalWrite(_rst, HIGH);
    pinMode(_rst, OUTPUT);
  }
  pinMode(_busy, INPUT);
  fillScreen(GxEPD_WHITE);
  _current_page = -1;
}

void GxGDEW027C44::fillScreen(uint16_t color)
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

void GxGDEW027C44::update(void)
{
  if (_current_page != -1) return;
  _wakeUp();
  _writeCommand(0x10);
  for (uint32_t i = 0; i < GxGDEW027C44_BUFFER_SIZE; i++)
  {
    _writeData((i < sizeof(_black_buffer)) ? _black_buffer[i] : 0x00);
  }
  _writeCommand(0x13);
  for (uint32_t i = 0; i < GxGDEW027C44_BUFFER_SIZE; i++)
  {
    _writeData((i < sizeof(_red_buffer)) ? _red_buffer[i] : 0x00);
  }
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("update");
  _sleep();
}

void  GxGDEW027C44::drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode)
{
  if (mode & bm_default) mode |= bm_normal; // no change
  drawBitmapBM(bitmap, x, y, w, h, color, mode);
}

void GxGDEW027C44::drawExamplePicture(const uint8_t* black_bitmap, const uint8_t* red_bitmap, uint32_t black_size, uint32_t red_size)
{
  drawPicture(black_bitmap, red_bitmap, black_size, red_size, bm_normal);
}

void GxGDEW027C44::drawPicture(const uint8_t* black_bitmap, const uint8_t* red_bitmap, uint32_t black_size, uint32_t red_size, int16_t mode)
{
  if (_current_page != -1) return;
  _wakeUp();
  _writeCommand(0x10);
  for (uint32_t i = 0; i < GxGDEW027C44_BUFFER_SIZE; i++)
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
  for (uint32_t i = 0; i < GxGDEW027C44_BUFFER_SIZE; i++)
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

void GxGDEW027C44::drawBitmap(const uint8_t* bitmap, uint32_t size, int16_t mode)
{
  if (_current_page != -1) return;
  if (mode & bm_default) mode |= bm_normal; // no change
  _wakeUp();
  _writeCommand(0x10);
  for (uint32_t i = 0; i < GxGDEW027C44_BUFFER_SIZE; i++)
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
  for (uint32_t i = 0; i < GxGDEW027C44_BUFFER_SIZE; i++)
  {
    _writeData(0);
  }
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("drawBitmap");
  _sleep();
}

void GxGDEW027C44::eraseDisplay(bool using_partial_update)
{
  if (_current_page != -1) return;
  _wakeUp();
  _writeCommand(0x10);
  for (uint32_t i = 0; i < GxGDEW027C44_BUFFER_SIZE; i++)
  {
    _writeData(0x00);
  }
  _writeCommand(0x13);
  for (uint32_t i = 0; i < GxGDEW027C44_BUFFER_SIZE; i++)
  {
    _writeData(0x00);
  }
  _writeCommand(0x12);      //display refresh
  _waitWhileBusy("eraseDisplay");
  _sleep();
}

void GxGDEW027C44::powerDown()
{
  _sleep();
}

void GxGDEW027C44::_writeCommand(uint8_t command)
{
  IO.writeCommandTransaction(command);
}

void GxGDEW027C44::_writeData(uint8_t data)
{
  IO.writeDataTransaction(data);
}

void GxGDEW027C44::_waitWhileBusy(const char* comment)
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

void GxGDEW027C44::_wakeUp()
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
  _writeData (0x73);
  _writeData (0x41);

  _writeCommand(0x16);
  _writeData(0x00);

  _writeCommand(0x04);
  _waitWhileBusy("_wakeUp Power On");

  _writeCommand(0x00);
  _writeData(0xaf);

  _writeCommand(0x30);      //PLL�趨 // define by OTP
  _writeData (0x3a);       //3A 100HZ   29 150Hz 39 200HZ 31 171HZ

  _writeCommand(0x61);      //�����趨 // define by OTP
  _writeData (0x00);
  _writeData (0xb0);       //176
  _writeData (0x01);
  _writeData (0x08);    //264

  _writeCommand(0x82);      //vcom�趨 // define by OTP
  _writeData (0x12);

  _writeCommand(0X50);      // define by OTP
  _writeData(0x87);   // define by OTP
  _writeLUT();              //д��lut
}

void GxGDEW027C44::_sleep(void)
{
  _writeCommand(0x02); // power off
  _waitWhileBusy("_sleep Power Off");
  if (_rst >= 0)
  {
    _writeCommand(0x07); // deep sleep
    _writeData (0xa5);
  }
}

void GxGDEW027C44::_writeLUT(void)
{
  unsigned int count;
  {
    _writeCommand(0x20);							//vcom
    for (count = 0; count < 44; count++)
    {
      _writeData(lut_vcomDC[count]);
    }

    _writeCommand(0x21);							//ww --
    for (count = 0; count < 42; count++)
    {
      _writeData(lut_ww[count]);
    }

    _writeCommand(0x22);							//bw r
    for (count = 0; count < 42; count++)
    {
      _writeData(lut_bw[count]);
    }

    _writeCommand(0x23);							//wb w
    for (count = 0; count < 42; count++)
    {
      _writeData(lut_bb[count]);
    }

    _writeCommand(0x24);							//bb b
    for (count = 0; count < 42; count++)
    {
      _writeData(lut_wb[count]);
    }
  }
}

void GxGDEW027C44::drawPaged(void (*drawCallback)(void))
{
  if (_current_page != -1) return;
  _wakeUp();
  _writeCommand(0x10);
  for (_current_page = 0; _current_page < GxGDEW027C44_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback();
    for (int16_t y1 = 0; y1 < GxGDEW027C44_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW027C44_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW027C44_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0x00;
        _writeData(data);
      }
    }
  }
  _writeCommand(0x13);
  for (_current_page = 0; _current_page < GxGDEW027C44_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback();
    for (int16_t y1 = 0; y1 < GxGDEW027C44_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW027C44_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW027C44_WIDTH / 8) + x1;
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

void GxGDEW027C44::drawPaged(void (*drawCallback)(uint32_t), uint32_t p)
{
  if (_current_page != -1) return;
  _wakeUp();
  _writeCommand(0x10);
  for (_current_page = 0; _current_page < GxGDEW027C44_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p);
    for (int16_t y1 = 0; y1 < GxGDEW027C44_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW027C44_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW027C44_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0x00;
        _writeData(data);
      }
    }
  }
  _writeCommand(0x13);
  for (_current_page = 0; _current_page < GxGDEW027C44_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p);
    for (int16_t y1 = 0; y1 < GxGDEW027C44_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW027C44_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW027C44_WIDTH / 8) + x1;
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

void GxGDEW027C44::drawPaged(void (*drawCallback)(const void*), const void* p)
{
  if (_current_page != -1) return;
  _wakeUp();
  _writeCommand(0x10);
  for (_current_page = 0; _current_page < GxGDEW027C44_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p);
    for (int16_t y1 = 0; y1 < GxGDEW027C44_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW027C44_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW027C44_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0x00;
        _writeData(data);
      }
    }
  }
  _writeCommand(0x13);
  for (_current_page = 0; _current_page < GxGDEW027C44_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p);
    for (int16_t y1 = 0; y1 < GxGDEW027C44_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW027C44_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW027C44_WIDTH / 8) + x1;
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

void GxGDEW027C44::drawPaged(void (*drawCallback)(const void*, const void*), const void* p1, const void* p2)
{
  if (_current_page != -1) return;
  _wakeUp();
  _writeCommand(0x10);
  for (_current_page = 0; _current_page < GxGDEW027C44_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p1, p2);
    for (int16_t y1 = 0; y1 < GxGDEW027C44_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW027C44_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW027C44_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0x00;
        _writeData(data);
      }
    }
  }
  _writeCommand(0x13);
  for (_current_page = 0; _current_page < GxGDEW027C44_PAGES; _current_page++)
  {
    fillScreen(GxEPD_WHITE);
    drawCallback(p1, p2);
    for (int16_t y1 = 0; y1 < GxGDEW027C44_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDEW027C44_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDEW027C44_WIDTH / 8) + x1;
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

void GxGDEW027C44::drawCornerTest(uint8_t em)
{
  if (_current_page != -1) return;
  _wakeUp();
  _writeCommand(0x10);
  for (uint32_t y = 0; y < GxGDEW027C44_HEIGHT; y++)
  {
    for (uint32_t x = 0; x < GxGDEW027C44_WIDTH / 8; x++)
    {
      uint8_t data = 0xFF;
      if ((x < 1) && (y < 8)) data = 0x00;
      if ((x > GxGDEW027C44_WIDTH / 8 - 3) && (y < 16)) data = 0x00;
      if ((x > GxGDEW027C44_WIDTH / 8 - 4) && (y > GxGDEW027C44_HEIGHT - 25)) data = 0x00;
      if ((x < 4) && (y > GxGDEW027C44_HEIGHT - 33)) data = 0x00;
      _writeData(~data);
    }
  }
  _writeCommand(0x13);
  for (uint32_t i = 0; i < GxGDEW027C44_BUFFER_SIZE; i++)
  {
    _writeData(0);
  }
  _writeCommand(0x12);      //display refresh
  _waitWhileBusy("drawCornerTest");
  _sleep();
}

