/************************************************************************************
   class GxGDE0213B1 : Display class example for GDE0213B1 e-Paper from Dalian Good Display Co., Ltd.: www.good-display.com

   based on Demo Example from Good Display, now available on http://www.good-display.com/download_list/downloadcategoryid=34&isMode=false.html

   Author : J-M Zingg

   modified by :

   Version : 2.1

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

#include "GxGDE0213B1.h"

#if defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif


#define xPixelsPar (GxGDE0213B1_X_PIXELS-1)
#define yPixelsPar (GxGDE0213B1_Y_PIXELS-1)

const uint8_t LUTDefault_full[] =
{
  0x32,  // command
  0x22, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x01, 0x00, 0x00, 0x00, 0x00
};

const uint8_t LUTDefault_part[] =
{
  0x32,  // command
  0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x0F, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t GDOControl[] = {0x01, yPixelsPar % 256, yPixelsPar / 256, 0x00}; //for 2.13inch
uint8_t softstart[] = {0x0c, 0xd7, 0xd6, 0x9d};
uint8_t VCOMVol[] = {0x2c, 0xa8};  // VCOM 7c
uint8_t DummyLine[] = {0x3a, 0x1a}; // 4 dummy line per gate
uint8_t Gatetime[] = {0x3b, 0x08};  // 2us per line
uint8_t RamDataEntryMode[] = {0x11, 0x01};  // Ram data entry mode

GxGDE0213B1::GxGDE0213B1(GxIO& io, uint8_t rst, uint8_t busy) :
  GxEPD(GxGDE0213B1_VISIBLE_WIDTH, GxGDE0213B1_HEIGHT),
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

void GxGDE0213B1::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GxGDE0213B1_VISIBLE_WIDTH - x - 1;
      break;
    case 2:
      x = GxGDE0213B1_VISIBLE_WIDTH - x - 1;
      y = GxGDE0213B1_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GxGDE0213B1_HEIGHT - y - 1;
      break;
  }
  y = GxGDE0213B1_HEIGHT - y - 1;
  uint16_t i = x / 8 + y * GxGDE0213B1_WIDTH / 8;
  if (_current_page < 1)
  {
    if (i >= sizeof(_buffer)) return;
  }
  else
  {
    if (i < GxGDE0213B1_PAGE_SIZE * _current_page) return;
    if (i >= GxGDE0213B1_PAGE_SIZE * (_current_page + 1)) return;
    i -= GxGDE0213B1_PAGE_SIZE * _current_page;
  }

  if (!color)
    _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
  else
    _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
}

void GxGDE0213B1::init(void)
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

void GxGDE0213B1::fillScreen(uint16_t color)
{
  uint8_t data = (color == GxEPD_BLACK) ? 0xFF : 0x00;
  for (uint16_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = data;
  }
}

void GxGDE0213B1::update(void)
{
  if (_current_page != -1) return;
  _using_partial_mode = false;
  _Init_Full(0x01);
  _writeCommand(0x24);
  for (uint16_t y = 0; y < GxGDE0213B1_HEIGHT; y++)
  {
    for (uint16_t x = 0; x < GxGDE0213B1_WIDTH / 8; x++)
    {
      uint16_t idx = y * (GxGDE0213B1_WIDTH / 8) + x;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
      _writeData(~data);
    }
  }
  _Update_Full();
  _PowerOff();
}

void  GxGDE0213B1::drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color)
{
  drawBitmap(bitmap, x, y, w, h, color);
}

void  GxGDE0213B1::drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode)
{
  uint16_t inverse_color = (color == GxEPD_WHITE) ? GxEPD_BLACK : GxEPD_WHITE;
  uint16_t fg_color = (mode & bm_invert) ? inverse_color : color;
  uint16_t bg_color = (mode & bm_invert) ? color : inverse_color;
  // taken from Adafruit_GFX.cpp, modified
  int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
  uint8_t byte = 0;
  for (int16_t j = 0; j < h; j++, y++)
  {
    for (int16_t i = 0; i < w; i++ )
    {
      if (i & 7) byte <<= 1;
      else
      {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
        byte = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
#else
        byte = bitmap[j * byteWidth + i / 8];
#endif
      }
      // keep using overwrite mode
      uint16_t pixelcolor = (byte & 0x80) ? fg_color  : bg_color;
      if (mode & bm_flip_v) drawPixel(width() - (x + i) - 1, y, pixelcolor);
      else drawPixel(x + i, y, pixelcolor);
    }
  }
}

void GxGDE0213B1::drawBitmap(const uint8_t *bitmap, uint32_t size, int16_t mode)
{
  uint8_t ram_entry_mode = 0x01; // always y-decrease, x-increase
  if ((mode & bm_flip_h) && (mode & bm_flip_v)) ram_entry_mode = 0x00;
  else if (mode & bm_flip_h) ram_entry_mode = 0x00; // always y-decrease, x-decrease
  //else if (mode & bm_flip_v) ram_entry_mode = 0x02; // always y-decrease
  if (mode & bm_partial_update) drawBitmapPU(bitmap, size, ram_entry_mode);
  else drawBitmapEM(bitmap, size, ram_entry_mode);
}

void GxGDE0213B1::drawBitmapEM(const uint8_t *bitmap, uint32_t size, uint8_t em)
{
  _using_partial_mode = false; // remember
  _Init_Full(em);
  _writeCommand(0x24);
  for (uint32_t i = 0; i < GxGDE0213B1_BUFFER_SIZE; i++)
  {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
    _writeData((i < size) ? pgm_read_byte(bitmap + i) : 0xFF);
#else
    _writeData((i < size) ? bitmap[i] : 0xFF);
#endif
  }
  _Update_Full();
  _PowerOff();
}

void GxGDE0213B1::drawBitmapPU(const uint8_t *bitmap, uint32_t size, uint8_t em)
{
  _using_partial_mode = true; // remember
  _Init_Part(em);
  _writeCommand(0x24);
  for (uint32_t i = 0; i < GxGDE0213B1_BUFFER_SIZE; i++)
  {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
    _writeData((i < size) ? pgm_read_byte(bitmap + i) : 0xFF);
#else
    _writeData((i < size) ? bitmap[i] : 0xFF);
#endif
  }
  _Update_Part();
  _PowerOff();
}

void GxGDE0213B1::eraseDisplay(bool using_partial_update)
{
  if (using_partial_update)
  {
    _using_partial_mode = true; // remember
    _Init_Part(0x01);
    _writeCommand(0x24);
    for (uint32_t i = 0; i < GxGDE0213B1_BUFFER_SIZE; i++)
    {
      _writeData(0xFF);
    }
    _Update_Part();
    delay(300);
    // update erase buffer
    _writeCommand(0x24);
    for (uint32_t i = 0; i < GxGDE0213B1_BUFFER_SIZE; i++)
    {
      _writeData(0xFF);
    }
    delay(300);
    _PowerOff();
  }
  else
  {
    _using_partial_mode = false; // remember
    _Init_Full(0x01);
    _writeCommand(0x24);
    for (uint32_t i = 0; i < GxGDE0213B1_BUFFER_SIZE; i++)
    {
      _writeData(0xFF);
    }
    _Update_Full();
    _PowerOff();
  }
}

void GxGDE0213B1::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  if (using_rotation)
  {
    switch (getRotation())
    {
      case 1:
        swap(x, y);
        swap(w, h);
        x = GxGDE0213B1_WIDTH - x - w - 1;
        break;
      case 2:
        x = GxGDE0213B1_WIDTH - x - w - 1;
        y = GxGDE0213B1_HEIGHT - y - h - 1;
        break;
      case 3:
        swap(x, y);
        swap(w, h);
        y = GxGDE0213B1_HEIGHT - y - h - 1;
        break;
    }
  }
  //fillScreen(0x0);
  if (x >= GxGDE0213B1_WIDTH) return;
  if (y >= GxGDE0213B1_HEIGHT) return;
  uint16_t xe = min(GxGDE0213B1_WIDTH, x + w) - 1;
  uint16_t ye = min(GxGDE0213B1_HEIGHT, y + h) - 1;
  uint16_t xs_d8 = x / 8;
  uint16_t xe_d8 = xe / 8; //(xe + 7) / 8;
  uint16_t ys_bx = GxGDE0213B1_HEIGHT - ye - 1;
  uint16_t ye_bx = GxGDE0213B1_HEIGHT - y - 1;
  _Init_Part(0x01); //
  _SetRamArea(xs_d8, xe_d8, ye % 256, ye / 256, y % 256, y / 256); // X-source area,Y-gate area
  _SetRamPointer(xs_d8, ye % 256, ye / 256); // set ram
  _waitWhileBusy();
  _writeCommand(0x24);
  for (int16_t y1 = ys_bx; y1 <= ye_bx; y1++)
  {
    for (int16_t x1 = xs_d8; x1 <= xe_d8; x1++)
    {
      uint16_t idx = y1 * (GxGDE0213B1_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
      _writeData(~data);
    }
  }
  _Update_Part();
  delay(300);
  // update erase buffer
  _SetRamArea(xs_d8, xe_d8, ye % 256, ye / 256, y % 256, y / 256); // X-source area,Y-gate area
  _SetRamPointer(xs_d8, ye % 256, ye / 256); // set ram
  _waitWhileBusy();
  _writeCommand(0x24);
  for (int16_t y1 = ys_bx; y1 <= ye_bx; y1++)
  {
    for (int16_t x1 = xs_d8; x1 <= xe_d8; x1++)
    {
      uint16_t idx = y1 * (GxGDE0213B1_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
      _writeData(~data);
    }
  }
  delay(300);
  _PowerOff();
}

void GxGDE0213B1::_writeCommand(uint8_t command)
{
  //while (digitalRead(_busy));
  if (digitalRead(_busy))
  {
    String str = String("command 0x") + String(command, HEX);
    _waitWhileBusy(str.c_str());
  }
  IO.writeCommandTransaction(command);
}

void GxGDE0213B1::_writeData(uint8_t data)
{
  IO.writeDataTransaction(data);
}

void GxGDE0213B1::_writeCommandData(const uint8_t* pCommandData, uint8_t datalen)
{
  //while (digitalRead(_busy)); // wait
  if (digitalRead(_busy))
  {
    String str = String("command 0x") + String(pCommandData[0], HEX);
    _waitWhileBusy(str.c_str());
  }
  IO.startTransaction();
  IO.writeCommand(*pCommandData++);
  for (uint8_t i = 0; i < datalen - 1; i++)	// sub the command
  {
    IO.writeData(*pCommandData++);
  }
  IO.endTransaction();

}

void GxGDE0213B1::_waitWhileBusy(const char* comment)
{
  unsigned long start = micros();
  while (1)
  {
    if (!digitalRead(_busy)) break;
    delay(1);
  }
  if (comment)
  {
    unsigned long elapsed = micros() - start;
    //Serial.print(comment);
    //Serial.print(" : ");
    //Serial.println(elapsed);
  }
}

void GxGDE0213B1::_setRamDataEntryMode(uint8_t em)
{
  em = min(em, 0x03);
  _writeCommand(0x11);
  _writeData(em);
  switch (em)
  {
    case 0x00: // x decrease, y decrease
      _SetRamArea(xPixelsPar / 8, 0x00, yPixelsPar % 256, yPixelsPar / 256, 0x00, 0x00);  // X-source area,Y-gate area
      _SetRamPointer(xPixelsPar / 8, yPixelsPar % 256, yPixelsPar / 256); // set ram
      break;
    case 0x01: // x increase, y decrease : as in demo code
      _SetRamArea(0x00, xPixelsPar / 8, yPixelsPar % 256, yPixelsPar / 256, 0x00, 0x00);  // X-source area,Y-gate area
      _SetRamPointer(0x00, yPixelsPar % 256, yPixelsPar / 256); // set ram
      break;
    case 0x02: // x decrease, y increase
      _SetRamArea(xPixelsPar / 8, 0x00, 0x00, 0x00, yPixelsPar % 256, yPixelsPar / 256);  // X-source area,Y-gate area
      _SetRamPointer(xPixelsPar / 8, 0x00, 0x00); // set ram
      break;
    case 0x03: // x increase, y increase : normal mode
      _SetRamArea(0x00, xPixelsPar / 8, 0x00, 0x00, yPixelsPar % 256, yPixelsPar / 256);  // X-source area,Y-gate area
      _SetRamPointer(0x00, 0x00, 0x00); // set ram
      break;
  }
}

void GxGDE0213B1::_SetRamArea(uint8_t Xstart, uint8_t Xend, uint8_t Ystart, uint8_t Ystart1, uint8_t Yend, uint8_t Yend1)
{
  _writeCommand(0x44);
  _writeData(Xstart);
  _writeData(Xend);
  _writeCommand(0x45);
  _writeData(Ystart);
  _writeData(Ystart1);
  _writeData(Yend);
  _writeData(Yend1);
}

void GxGDE0213B1::_SetRamPointer(uint8_t addrX, uint8_t addrY, uint8_t addrY1)
{
  _writeCommand(0x4e);
  _writeData(addrX);
  _writeCommand(0x4f);
  _writeData(addrY);
  _writeData(addrY1);
}

void GxGDE0213B1::_PowerOn(void)
{
  _writeCommand(0x22);
  _writeData(0xc0);
  _writeCommand(0x20);
}

void GxGDE0213B1::_PowerOff(void)
{
  _writeCommand(0x22);
  _writeData(0xc3);
  _writeCommand(0x20);
}

void GxGDE0213B1::_InitDisplay(uint8_t em)
{
  _writeCommandData(GDOControl, sizeof(GDOControl));  // Pannel configuration, Gate selection
  _writeCommandData(softstart, sizeof(softstart));  // X decrease, Y decrease
  _writeCommandData(VCOMVol, sizeof(VCOMVol));    // VCOM setting
  _writeCommandData(DummyLine, sizeof(DummyLine));  // dummy line per gate
  _writeCommandData(Gatetime, sizeof(Gatetime));    // Gate time setting
  _setRamDataEntryMode(em);
}

void GxGDE0213B1::_Init_Full(uint8_t em)
{
  _InitDisplay(em);
  _writeCommandData(LUTDefault_full, sizeof(LUTDefault_full));
  _PowerOn();
}

void GxGDE0213B1::_Init_Part(uint8_t em)
{
  _InitDisplay(em);
  _writeCommandData(LUTDefault_part, sizeof(LUTDefault_part));
  _PowerOn();
}

void GxGDE0213B1::_Update_Full(void)
{
  _writeCommand(0x22);
  _writeData(0xc4);
  _writeCommand(0x20);
  _writeCommand(0xff);
}

void GxGDE0213B1::_Update_Part(void)
{
  _writeCommand(0x22);
  _writeData(0x04);
  _writeCommand(0x20);
  _writeCommand(0xff);
}

//#define DRAW_PAGED_USING_PARTIAL_UPDATE

#if defined(DRAW_PAGED_USING_PARTIAL_UPDATE)

void GxGDE0213B1::_drawCurrentPage()
{
  uint16_t xs_d8 = 0;
  uint16_t xe_d8 = (GxGDE0213B1_WIDTH - 1) / 8;
  uint16_t y = GxGDE0213B1_PAGE_HEIGHT * _current_page;
  uint16_t ye = y + GxGDE0213B1_PAGE_HEIGHT - 1;
  uint16_t ys_p = (GxGDE0213B1_HEIGHT - y - 1);
  uint16_t ye_p = (GxGDE0213B1_HEIGHT - ye - 1);
  _SetRamArea(xs_d8, xe_d8, ys_p % 256, ys_p / 256, ye_p % 256, ye_p / 256); // X-source area,Y-gate area
  _SetRamPointer(xs_d8, ys_p % 256, ys_p / 256); // set ram
  _waitWhileBusy();
  _writeCommand(0x24);
  for (int16_t y1 = 0; y1 < GxGDE0213B1_PAGE_HEIGHT; y1++)
  {
    for (int16_t x1 = 0; x1 < GxGDE0213B1_WIDTH / 8; x1++)
    {
      uint16_t idx = y1 * (GxGDE0213B1_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
      _writeData(~data);
    }
  }
  _Update_Part();
  delay(300);
  // update erase buffer
  _SetRamArea(xs_d8, xe_d8, ys_p % 256, ys_p / 256, ye_p % 256, ye_p / 256); // X-source area,Y-gate area
  _SetRamPointer(xs_d8, ys_p % 256, ys_p / 256); // set ram
  _waitWhileBusy();
  _writeCommand(0x24);
  for (int16_t y1 = 0; y1 < GxGDE0213B1_PAGE_HEIGHT; y1++)
  {
    for (int16_t x1 = 0; x1 < GxGDE0213B1_WIDTH / 8; x1++)
    {
      uint16_t idx = y1 * (GxGDE0213B1_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
      _writeData(~data);
    }
  }
  delay(300);
}

void GxGDE0213B1::drawPaged(void (*drawCallback)(void))
{
  if (!_using_partial_mode)
  {
    eraseDisplay(false);
    eraseDisplay(true);
  }
  _using_partial_mode = true;
  _Init_Part(0x01);
  for (_current_page = 0; _current_page < GxGDE0213B1_PAGES; _current_page++)
  {
    fillScreen(0xFF);
    drawCallback();
    //fillScreen(0x00);
    _drawCurrentPage();
    //delay(2000);
  }
  _current_page = -1;
  _PowerOff();
}

#else

void GxGDE0213B1::drawPaged(void (*drawCallback)(void))
{
  _Init_Full(0x01);
  _writeCommand(0x24);
  for (_current_page = 0; _current_page < GxGDE0213B1_PAGES; _current_page++)
  {
    fillScreen(0xFF);
    drawCallback();
    for (int16_t y1 = 0; y1 < GxGDE0213B1_PAGE_HEIGHT; y1++)
    {
      for (int16_t x1 = 0; x1 < GxGDE0213B1_WIDTH / 8; x1++)
      {
        uint16_t idx = y1 * (GxGDE0213B1_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
        _writeData(~data);
      }
    }
  }
  _current_page = -1;
  _Update_Full();
  _PowerOff();
}

#endif

void GxGDE0213B1::drawCornerTest(uint8_t em)
{
  _Init_Full(em);
  _writeCommand(0x24);
  for (uint32_t y = 0; y < GxGDE0213B1_HEIGHT; y++)
  {
    for (uint32_t x = 0; x < GxGDE0213B1_WIDTH / 8; x++)
    {
      uint8_t data = 0xFF;
      if ((x < 1) && (y < 8)) data = 0x00;
      if ((x > GxGDE0213B1_WIDTH / 8 - 3) && (y < 16)) data = 0x00;
      if ((x > GxGDE0213B1_WIDTH / 8 - 4) && (y > GxGDE0213B1_HEIGHT - 25)) data = 0x00;
      if ((x < 4) && (y > GxGDE0213B1_HEIGHT - 33)) data = 0x00;
      _writeData(data);
    }
  }
  _Update_Full();
  _PowerOff();
}

