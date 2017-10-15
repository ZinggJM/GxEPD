// Class GxGDE06 : display class for GDE06 e-paper display from Dalian Good Display Inc.
//
// Created by Jean-Marc Zingg based on demo code from Good Display for green DESTM32-L board.
//
// The GxGDE06 class is not board specific, but relies on a specific IO channel class for parallel connection e-paper displays.
//
// The GxIO_GreenSTM32F103V is board specific and serves as IO channel for the display class.
//
// These classes can also serve as an example for other boards to use with parallel e-paper displays from Good Display,
// however this is not easy, because of the e-paper specific supply voltages.
//
// To be used with "Generic STM32F103V series" of package "STM32 Boards (STMduino.com)" for Arduino.
// https://github.com/rogerclarkmelbourne/Arduino_STM32
//
// The e-paper display and demo board is available from:
// http://www.buy-lcd.com/index.php?route=product/product&path=2897_8370&product_id=15273
//
// note: "This is a old version 6'' e-paper display, which will not be produced any more if needed quantity is less than 200Kpcs per order."
// The new red DESTM32-L board provides better performance and more resources (1MB FSMC SRAM) for the same price.
//
// The display provides 4 levels of gray. This is supported with the drawPicture() method.
// Drawing text and graphics using Adafruit_GFX methods is black and white only, because of the limited processor RAM available.
// The buffer size for rendering with Adafruit_GFX is reduced ~10% to fit in remaining RAM, producing a white stripe at the bottom!
//
// Added to my library for reference, completeness and backup, but not recommended for use.

#include "GxGDE06.h"

GxGDE06::GxGDE06(GxIO_GreenSTM32F103V& io)
  : GxEPD(GxGDE06_WIDTH, GxGDE06_HEIGHT), IO(io)
{
}

template <typename T> static inline void
swap(T& a, T& b)
{
  T t = a;
  a = b;
  b = t;
}

void GxGDE06::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GxGDE06_WIDTH - x - 1;
      break;
    case 2:
      x = GxGDE06_WIDTH - x - 1;
      y = GxGDE06_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GxGDE06_HEIGHT - y - 1;
      break;
  }
  uint32_t i = x / 8 + y * GxGDE06_WIDTH / 8;
  if (i >= GxGDE06_BUFFER_SIZE) return; // for reduced buffer size
  if (color == GxEPD_BLACK) epd_buffer[i] = (epd_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
  else if (color == GxEPD_WHITE) epd_buffer[i] = (epd_buffer[i] | (1 << (7 - x % 8)));
  else
  {
    uint16_t brightness = ((color & 0xF100) >> 11) + ((color & 0x07E0) >> 5) + (color & 0x001F);
    if (brightness < 3 * 128) (epd_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
    else epd_buffer[i] = (epd_buffer[i] | (1 << (7 - x % 8)));
  }
}

void GxGDE06::init(void)
{
  IO.init();
  init_wave_table();
}

void GxGDE06::fillScreen(uint16_t color)
{
  uint8_t data;
  if (color == GxEPD_BLACK) data = 0x00;
  else if (color == GxEPD_WHITE) data = 0xFF;
  else
  {
    uint16_t brightness = ((color & 0xF100) >> 11) + ((color & 0x07E0) >> 5) + (color & 0x001F);
    if (brightness < 3 * 128)  data = 0x00;
    else data = 0xFF;
  }
  for (uint32_t x = 0; x < GxGDE06_BUFFER_SIZE; x++)
  {
    (epd_buffer)[x] = data;
  }
}

void GxGDE06::update()
{
  drawBitmap(epd_buffer, sizeof(epd_buffer_type));
}

void GxGDE06::drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color)
{
  drawBitmap(bitmap, x, y, w, h, color);
}

void  GxGDE06::drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode)
{
  if (mode == bm_default) mode = bm_normal;
  drawBitmapBM(bitmap, x, y, w, h, color, mode);
}

void GxGDE06::drawPicture(const uint8_t *picture, uint32_t size)
{
  IO.powerOn();
  delay(25);
  clear_display();
  for (uint16_t frame = 0; frame < GxGDE06_FRAME_END_SIZE; frame++)
  {
    IO.start_scan();
    for (uint16_t line = 0; line < GxGDE06_HEIGHT; line++)
    {
      uint32_t x = line * (GxGDE06_WIDTH / 4);
      uint16_t i = 0;
      for (; (i < GxGDE06_ROW_BUFFER_SIZE) && (x < size); i++, x++)
      {
        row_buffer[i] = wave_end_table[picture[x]][frame];
      }
      for (; (i < GxGDE06_ROW_BUFFER_SIZE); i++)
      {
        row_buffer[i] = wave_end_table[0xFF][frame];
      }
      IO.send_row(row_buffer, GxGDE06_ROW_BUFFER_SIZE);
    }
    IO.send_row(row_buffer, GxGDE06_ROW_BUFFER_SIZE);
  }
  delay(25);
  IO.powerOff();
}

void GxGDE06::drawBitmap(const uint8_t *bitmap, uint32_t size, int16_t m)
{
  IO.powerOn();
  delay(25);
  clear_display();
  for (uint16_t frame = 0; frame < GxGDE06_FRAME_END_SIZE; frame++)
  {
    IO.start_scan();
    for (uint16_t line = 0; line < GxGDE06_HEIGHT; line++)
    {
      uint32_t x = line * (GxGDE06_WIDTH / 8);
      uint16_t i = 0;
      for (; (i < GxGDE06_ROW_BUFFER_SIZE) && (x < size); i++)
      {
        if (0 == i % 2)
        {
          uint8_t grey8b = bw2grey[(bitmap[x] & 0xF0) >> 4];
          row_buffer[i] = wave_end_table[grey8b][frame];
        }
        else
        {
          uint8_t grey8b = bw2grey[bitmap[x] & 0x0F];
          row_buffer[i] = wave_end_table[grey8b][frame];
          x++;
        }
      }
      for (; (i < GxGDE06_ROW_BUFFER_SIZE); i++)
      {
        row_buffer[i] = wave_end_table[0xFF][frame];
      }
      IO.send_row(row_buffer, GxGDE06_ROW_BUFFER_SIZE);
    }
    IO.send_row(row_buffer, GxGDE06_ROW_BUFFER_SIZE);
  }
  delay(25);
  IO.powerOff();
}

void GxGDE06::erasePicture(const uint8_t *picture, uint32_t size)
{
  IO.powerOn();
  delay(25);
  for (uint16_t frame = 0; frame < GxGDE06_FRAME_BEGIN_SIZE; frame++)
  {
    IO.start_scan();
    for (uint16_t line = 0; line < GxGDE06_HEIGHT; line++)
    {
      uint32_t x = line * (GxGDE06_WIDTH / 4);
      uint16_t i = 0;
      for (; (i < GxGDE06_ROW_BUFFER_SIZE) && (x < size); i++, x++)
      {
        row_buffer[i] = wave_begin_table[picture[x]][frame];
      }
      for (; (i < GxGDE06_ROW_BUFFER_SIZE); i++)
      {
        row_buffer[i] = wave_begin_table[0x00][frame];
      }
      IO.send_row(row_buffer, GxGDE06_ROW_BUFFER_SIZE);
    }
    IO.send_row(row_buffer, GxGDE06_ROW_BUFFER_SIZE);
  }
  delay(25);
  IO.powerOff();
}

void GxGDE06::eraseBitmap(const uint8_t *bitmap, uint32_t size)
{
  IO.powerOn();
  delay(25);
  for (uint16_t frame = 0; frame < GxGDE06_FRAME_END_SIZE; frame++)
  {
    IO.start_scan();
    for (uint16_t line = 0; line < GxGDE06_HEIGHT; line++)
    {
      uint32_t x = line * (GxGDE06_WIDTH / 8);
      uint16_t i = 0;
      for (; (i < GxGDE06_ROW_BUFFER_SIZE) && (x < size); i++)
      {
        if (0 == i % 2)
        {
          uint8_t grey8b = bw2grey[(bitmap[x] & 0xF0) >> 4];
          row_buffer[i] = wave_begin_table[grey8b][frame];
        }
        else
        {
          uint8_t grey8b = bw2grey[bitmap[x] & 0x0F];
          row_buffer[i] = wave_begin_table[grey8b][frame];
          x++;
        }
      }
      for (; (i < GxGDE06_ROW_BUFFER_SIZE); i++)
      {
        row_buffer[i] = wave_end_table[0x00][frame];
      }
      IO.send_row(row_buffer, GxGDE06_ROW_BUFFER_SIZE);
    }
    IO.send_row(row_buffer, GxGDE06_ROW_BUFFER_SIZE);
  }
  delay(25);
  IO.powerOff();
}

void GxGDE06::clearDisplay()
{
  IO.powerOn();
  delay(25);
  clear_display();
  fillScreen(GxEPD_WHITE);
  delay(25);
  IO.powerOff();
}

void GxGDE06::DisplayTestPicture(const uint8_t *picture)
{
  IO.powerOn();

  for (uint16_t frame = 0; frame < 2; frame++)
  {
    IO.start_scan();
    for (uint16_t line = 0; line < GxGDE06_HEIGHT; line++)
    {
      for (uint16_t i = 0; i < GxGDE06_ROW_BUFFER_SIZE; i++)
      {
        row_buffer[i] = 0x00;
      }
      IO.send_row(row_buffer, GxGDE06_ROW_BUFFER_SIZE);
    }
    IO.send_row(row_buffer, GxGDE06_ROW_BUFFER_SIZE);
  }

  for (uint16_t frame = 0; frame < GxGDE06_FRAME_END_SIZE - 2; frame++)
  {
    IO.start_scan();
    for (uint16_t line = 0; line < GxGDE06_HEIGHT; line++)
    {
      uint32_t x = line * (GxGDE06_WIDTH / 4);
      for (uint16_t i = 0; i < GxGDE06_ROW_BUFFER_SIZE; i++, x++)
      {
        row_buffer[i] = wave_end_table[picture[x]][frame];
      }
      IO.send_row(row_buffer, GxGDE06_ROW_BUFFER_SIZE);
    }
    IO.send_row(row_buffer, GxGDE06_ROW_BUFFER_SIZE);
  }
  IO.powerOff();
}

void GxGDE06::init_wave_table(void)
{
  int frame, num;
  unsigned char tmp, value;


  //wave_begin_table
  for (frame = 0; frame < GxGDE06_FRAME_BEGIN_SIZE; frame++)
  {
    for (num = 0; num < WAVE_TABLE_SIZE; num++)
    {
      tmp = 0;
      tmp = wave_begin[(num >> 6) & 0x3][frame];

      tmp = tmp << 2;
      tmp &= 0xfffc;
      tmp |= wave_begin[(num >> 4) & 0x3][frame];

      tmp = tmp << 2;
      tmp &= 0xfffc;
      tmp |= wave_begin[(num >> 2) & 0x3][frame];

      tmp = tmp << 2;
      tmp &= 0xfffc;
      tmp |= wave_begin[(num) & 0x3][frame];

      value = 0;
      value = (tmp << 6) & 0xc0;
      value += (tmp << 2) & 0x30;
      value += (tmp >> 2) & 0x0c;
      value += (tmp >> 6) & 0x03;
      wave_begin_table[num][frame] = value;
    }
  }

  //wave_end_table
  for (frame = 0; frame < GxGDE06_FRAME_END_SIZE; frame++)
  {
    for (num = 0; num < WAVE_TABLE_SIZE; num++)
    {
      tmp = 0;
      tmp = wave_end[(num >> 6) & 0x3][frame];

      tmp = tmp << 2;
      tmp &= 0xfffc;
      tmp |= wave_end[(num >> 4) & 0x3][frame];

      tmp = tmp << 2;
      tmp &= 0xfffc;
      tmp |= wave_end[(num >> 2) & 0x3][frame];

      tmp = tmp << 2;
      tmp &= 0xfffc;
      tmp |= wave_end[(num) & 0x3][frame];

      value = 0;
      value = (tmp << 6) & 0xc0;
      value += (tmp << 2) & 0x30;
      value += (tmp >> 2) & 0x0c;
      value += (tmp >> 6) & 0x03;
      wave_end_table[num][frame] = value;
    }
  }
}

void GxGDE06::clear_display()
{
  for (uint16_t frame = 0; frame < GxGDE06_FRAME_BEGIN_SIZE; frame++)
  {
    IO.start_scan();
    for (uint16_t line = 0; line < GxGDE06_HEIGHT; line++)
    {
      for (uint16_t i = 0; (i < GxGDE06_ROW_BUFFER_SIZE); i++)
      {
        row_buffer[i] = wave_begin_table[0x00][frame];
      }
      IO.send_row(row_buffer, GxGDE06_ROW_BUFFER_SIZE);
    }
    IO.send_row(row_buffer, GxGDE06_ROW_BUFFER_SIZE);
  }
}

