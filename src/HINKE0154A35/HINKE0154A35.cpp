// class HINKE0154A35 : Display class for HINKE0154A35
// Author : J-M Zingg
// Edited : ATCnetz.de
//
// Version : see library.properties
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// Library: https://github.com/ZinggJM/GxEPD

#include "HINKE0154A35.h"

//#define DISABLE_DIAGNOSTIC_OUTPUT

#if defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

// Partial Update Delay, may have an influence on degradation
#define HINKE0154A35_PU_DELAY 500

HINKE0154A35::HINKE0154A35(GxIO& io, int8_t rst, int8_t busy)
  : GxEPD(HINKE0154A35_WIDTH, HINKE0154A35_HEIGHT), IO(io),
    _current_page(-1), _using_partial_mode(false), _diag_enabled(false),
    _rst(rst), _busy(busy)
{
}

void HINKE0154A35::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  swap(x, y);
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = HINKE0154A35_WIDTH - x - 1;
      break;
    case 2:
      x = HINKE0154A35_WIDTH - x - 1;
      y = HINKE0154A35_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = HINKE0154A35_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * HINKE0154A35_WIDTH / 8;
  if (_current_page < 1)
  {
    if (i >= sizeof(_black_buffer)) return;
  }
  else
  {
    y -= _current_page * HINKE0154A35_PAGE_HEIGHT;
    if ((y < 0) || (y >= HINKE0154A35_PAGE_HEIGHT)) return;
    i = x / 8 + y * HINKE0154A35_WIDTH / 8;
  }

  _black_buffer[i] = (_black_buffer[i] & (0xFF ^ (1 << (7 - x % 8)))); // white

  _red_buffer[i] = (_red_buffer[i] | (1 << (7 - x % 8))); // white
  if (color == GxEPD_WHITE) return;
  else if (color == GxEPD_BLACK) _black_buffer[i] = (_black_buffer[i] | (1 << (7 - x % 8)));
  else if (color == GxEPD_RED)_red_buffer[i] = (_red_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
  else
  {
    if ((color & 0xF100) > (0xF100 / 2))_red_buffer[i] = (_red_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
    else if ((((color & 0xF100) >> 11) + ((color & 0x07E0) >> 5) + (color & 0x001F)) < 3 * 255 / 2)
    {
      _black_buffer[i] = (_black_buffer[i] | (1 << (7 - x % 8)));
    }
  }
}


void HINKE0154A35::init(uint32_t serial_diag_bitrate)
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
}

void HINKE0154A35::fillScreen(uint16_t color)
{
  uint8_t black = 0x00;
  uint8_t red = 0xFF;
  if (color == GxEPD_WHITE);
  else if (color == GxEPD_BLACK) black = 0xFF;
  else if (color == GxEPD_RED) red = 0x00;
  else if ((color & 0xF100) > (0xF100 / 2))  red = 0x00;
  else if ((((color & 0xF100) >> 11) + ((color & 0x07E0) >> 5) + (color & 0x001F)) < 3 * 255 / 2) black = 0xFF;
  for (uint16_t x = 0; x < sizeof(_black_buffer); x++)
  {
    _black_buffer[x] = black;
    _red_buffer[x] = red;
  }
}

void HINKE0154A35::update(void)
{
 // reset required for wakeup
  if (_rst >= 0)
  {
    digitalWrite(_rst, 0);
    delay(10);
    digitalWrite(_rst, 1);
    delay(10);
  }
  //1.54 Zoll Display: 
  _writeCommand(0x12);
  _waitWhileBusy("SendResetfirst");
  delay(5);

  _writeCommand(0x74);
  _writeData (0x54);

  _writeCommand(0x7E);
  _writeData (0x3B);

  _writeCommand(0x2B);
  _writeData (0x04);
  _writeData (0x63);

  _writeCommand(0x0C);
  _writeData (0x8F);
  _writeData (0x8F);
  _writeData (0x8F);
  _writeData (0x3F);

  _writeCommand(0x01);
  _writeData (0x97);
  _writeData (0x00);
  _writeData (0x00);

  _writeCommand(0x11);
  _writeData (0x02);

  _writeCommand(0x44);
  _writeData (0x12);
  _writeData (0x00);

  _writeCommand(0x45);
  _writeData (0x00);
  _writeData (0x00);
  _writeData (0x97);
  _writeData (0x00);

  _writeCommand (0x3C);
  _writeData (0x01);
  _writeCommand (0x18);
  _writeData (0x80);
  _writeCommand (0x22);
  _writeData (0xB1);
  _writeCommand (0x20);
  _waitWhileBusy("SendResetmiddle");
  delay(5);

  _writeCommand (0x1B);
  _writeCommand (0x0C);
  _writeCommand (0xFF);

  _writeCommand (0x4E);
  _writeData (0x12);

  _writeCommand (0x4F);
  _writeData (0x00);
  _writeData (0x00);

  _writeCommand (0x26);

  for (uint32_t i = 0; i < HINKE0154A35_BUFFER_SIZE; i++)
  {
    _writeData((i < sizeof(_red_buffer)) ? ~_red_buffer[i] : 0xFF);
  }

  _writeCommand (0x4E);
  _writeData (0x12);

  _writeCommand (0x4F);
  _writeData (0x00);
  _writeData (0x00);

  _writeCommand (0x24);

  for (uint32_t i = 0; i < HINKE0154A35_BUFFER_SIZE; i++)
  {
    _writeData((i < sizeof(_black_buffer)) ? ~_black_buffer[i] : 0xFF);
  }

  _writeCommand (0x22);
  _writeData (0xC7);

  _writeCommand (0x20);
  _waitWhileBusy("SEND DONE");  
}

void HINKE0154A35::_writeCommand(uint8_t command)
{
  IO.writeCommandTransaction(command);
}

void HINKE0154A35::_writeData(uint8_t data)
{
  IO.writeDataTransaction(data);
}

void HINKE0154A35::_waitWhileBusy(const char* comment)
{
  unsigned long start = micros();
  while (1)
  {
    if (digitalRead(_busy) == 0) break;
    delay(1);
    if (micros() - start > 30000000) // >14.9s !
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

void HINKE0154A35::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = HINKE0154A35_WIDTH - x - w - 1;
      break;
    case 2:
      x = HINKE0154A35_WIDTH - x - w - 1;
      y = HINKE0154A35_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = HINKE0154A35_HEIGHT - y - h - 1;
      break;
  }
}
