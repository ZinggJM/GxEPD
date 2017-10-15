// Class GxGDE043A2 : display class for GDE043A2 e-paper display from Dalian Good Display Inc.
//
// Created by Jean-Marc Zingg based on demo code from Good Display for red DESTM32-L board.
//
// The GxGDE043A2 class is not board specific, but relies on FMSC SRAM for buffer memory, as available on DESTM32-L,
// and on a specific IO channel class for parallel connection e-paper displays.
// 
// The GxIO_DESTM32L is board specific and serves as IO channel for the display class.
//
// These classes can serve as an example for other boards for this e-paper display,
// however this is not easy, because of the e-paper specific supply voltages and big RAM buffer needed.
//
// To be used with "BLACK 407ZE (V3.0)" of "BLACK F407VE/ZE/ZG boards" of package "STM32GENERIC for STM32 boards" for Arduino.
// https://github.com/danieleff/STM32GENERIC
//
// The e-paper display and demo board is available from:
// http://www.buy-lcd.com/index.php?route=product/product&path=2897_10571&product_id=22833
// or https://www.aliexpress.com/store/product/Epaper-demo-kit-for-6-800X600-epaper-display-GDE060BA/600281_32812255729.html
//
// My GDE043A2 shows degradation of white parts after some seconds, could be timing issue, needs further investigation.

#include "GxGDE043A2.h"

#include "BitmapExamples.h"

GxGDE043A2::GxGDE043A2(GxIO_DESTM32L& io)
  : GxEPD(GxGDE043A2_WIDTH, GxGDE043A2_HEIGHT),
    p_active_buffer(&FMSC_SRAM->epd_sram_buffer1),
    p_erase_buffer(&FMSC_SRAM->epd_sram_buffer2), IO(io)
{
}

template <typename T> static inline void
swap(T& a, T& b)
{
  T t = a;
  a = b;
  b = t;
}

void GxGDE043A2::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GxGDE043A2_WIDTH - x - 1;
      break;
    case 2:
      x = GxGDE043A2_WIDTH - x - 1;
      y = GxGDE043A2_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GxGDE043A2_HEIGHT - y - 1;
      break;
  }
  uint32_t i = x / 4 + y * GxGDE043A2_WIDTH / 4;
  (*p_active_buffer)[i] = ((*p_active_buffer)[i] & (0xFF ^ (3 << 2 * (3 - x % 4))));
  if (color == GxEPD_BLACK) return;
  else if (color == GxEPD_WHITE) (*p_active_buffer)[i] = ((*p_active_buffer)[i] | (3 << 2 * (3 - x % 4)));
  else if (color == GxEPD_DARKGREY) (*p_active_buffer)[i] = ((*p_active_buffer)[i] | (1 << 2 * (3 - x % 4)));
  else if (color == GxEPD_LIGHTGREY) (*p_active_buffer)[i] = ((*p_active_buffer)[i] | (2 << 2 * (3 - x % 4)));
  else
  {
    uint16_t brightness = ((color & 0xF100) >> 11) + ((color & 0x07E0) >> 5) + (color & 0x001F);
    if (brightness < 3 * 128 / 2) return; // < 1/2 of 3 * GxEPD_DARKGREY, below middle between black and dark grey
    else if (brightness < 3 * 256 / 2) (*p_active_buffer)[i] = ((*p_active_buffer)[i] | (1 << 2 * (3 - x % 4))); // below middle
    else if (brightness < 3 * 192 / 2) (*p_active_buffer)[i] = ((*p_active_buffer)[i] | (2 << 2 * (3 - x % 4))); // below middle between light grey and white
    else (*p_active_buffer)[i] = ((*p_active_buffer)[i] | (3 << 2 * (3 - x % 4))); // above middle between light grey and white
  }
}

void GxGDE043A2::init(void)
{
  IO.init(PB12);
  init_wave_table();
}

void GxGDE043A2::fillScreen(uint16_t color)
{
  uint8_t data;
  if (color == GxEPD_BLACK) data = 0x00;
  else if (color == GxEPD_WHITE) data = 0xFF;
  else if (color == GxEPD_DARKGREY) data = 0x55;
  else if (color == GxEPD_LIGHTGREY) data = 0xAA;
  else
  {
    uint16_t brightness = ((color & 0xF100) >> 11) + ((color & 0x07E0) >> 5) + (color & 0x001F);
    if (brightness < 3 * 128 / 2)  data = 0x00; // < 1/2 of 3 * GxEPD_DARKGREY, below middle between black and dark grey
    else if (brightness < 3 * 256 / 2)  data = 0x55; // below middle
    else if (brightness < 3 * 192 / 2)  data = 0xAA; // below middle between light grey and white
    else data = 0xFF; // above middle between light grey and white
  }
  for (uint32_t x = 0; x < GxGDE043A2_BUFFER_SIZE; x++)
  {
    (*p_active_buffer)[x] = data;
  }
}

void GxGDE043A2::update()
{
  erasePicture(*p_erase_buffer, sizeof(epd_buffer_type));
  drawPicture(*p_active_buffer, sizeof(epd_buffer_type));
  swap(p_erase_buffer, p_active_buffer);
}

void GxGDE043A2::drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color)
{
  drawBitmap(bitmap, x, y, w, h, color);
}

void  GxGDE043A2::drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode)
{
  if (mode == bm_default) mode = bm_normal;
  drawBitmapBM(bitmap, x, y, w, h, color, mode);
}

void GxGDE043A2::drawPicture(const uint8_t *bitmap, uint32_t size)
{
  IO.powerOn();
  IO.delay35ns(0xfff);
  for (uint16_t frame = 0; frame < GxGDE043A2_FRAME_END_SIZE - 2; frame++)
  {
    IO.start_scan();
    for (uint16_t line = 0; line < GxGDE043A2_HEIGHT; line++)
    {
      uint32_t x = line * GxGDE043A2_ROW_BUFFER_SIZE;
      uint16_t i = 0;
      for (; (i < GxGDE043A2_ROW_BUFFER_SIZE) && (x < size); i++, x++)
      {
        row_buffer[i] = wave_end_table[bitmap[x]][frame];
      }
      for (; (i < GxGDE043A2_ROW_BUFFER_SIZE); i++)
      {
        row_buffer[i] = wave_end_table[0xFF][frame];
      }
      IO.send_row(row_buffer, GxGDE043A2_ROW_BUFFER_SIZE, GxGDE043A2_CL_DELAY);
    }
    IO.send_row(row_buffer, GxGDE043A2_ROW_BUFFER_SIZE, GxGDE043A2_CL_DELAY);
  }
  IO.delay35ns(0xfff);
  IO.powerOff();
}

void GxGDE043A2::drawBitmap(const uint8_t *bitmap, uint32_t size, int16_t m)
{
  IO.powerOn();
  delay(25);
  clear_display();
  for (uint16_t frame = 0; frame < GxGDE043A2_FRAME_END_SIZE; frame++)
  {
    IO.start_scan();
    for (uint16_t line = 0; line < GxGDE043A2_HEIGHT; line++)
    {
      uint32_t x = line * (GxGDE043A2_WIDTH / 8);
      uint16_t i = 0;
      for (; (i < GxGDE043A2_ROW_BUFFER_SIZE) && (x < size); i++)
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
      for (; (i < GxGDE043A2_ROW_BUFFER_SIZE); i++)
      {
        row_buffer[i] = wave_end_table[0xFF][frame];
      }
      IO.send_row(row_buffer, GxGDE043A2_ROW_BUFFER_SIZE, GxGDE043A2_CL_DELAY);
    }
    IO.send_row(row_buffer, GxGDE043A2_ROW_BUFFER_SIZE, GxGDE043A2_CL_DELAY);
  }
  delay(25);
  IO.powerOff();
}

void GxGDE043A2::erasePicture(const uint8_t *bitmap, uint32_t size)
{
  IO.powerOn();
  IO.delay35ns(0xfff);
  for (uint16_t frame = 0; frame < GxGDE043A2_FRAME_BEGIN_SIZE; frame++)
  {
    IO.start_scan();
    IO.delay35ns(0xff);
    for (uint16_t line = 0; line < GxGDE043A2_HEIGHT; line++)
    {
      uint32_t x = line * GxGDE043A2_ROW_BUFFER_SIZE;
      uint16_t i = 0;
      for (; (i < GxGDE043A2_ROW_BUFFER_SIZE) && (x < size); i++, x++)
      {
        row_buffer[i] = wave_begin_table[bitmap[x]][frame];
      }
      for (; (i < GxGDE043A2_ROW_BUFFER_SIZE); i++)
      {
        row_buffer[i] = wave_begin_table[0xFF][frame];
      }
      IO.send_row(row_buffer, GxGDE043A2_ROW_BUFFER_SIZE, GxGDE043A2_CL_DELAY);
    }
    IO.delay35ns(0xff);
    IO.send_row(row_buffer, GxGDE043A2_ROW_BUFFER_SIZE, GxGDE043A2_CL_DELAY);
  }
  IO.delay35ns(0xfff);
  IO.powerOff();
}

void GxGDE043A2::eraseBitmap(const uint8_t *bitmap, uint32_t size)
{
  IO.powerOn();
  delay(25);
  for (uint16_t frame = 0; frame < GxGDE043A2_FRAME_END_SIZE; frame++)
  {
    IO.start_scan();
    for (uint16_t line = 0; line < GxGDE043A2_HEIGHT; line++)
    {
      uint32_t x = line * (GxGDE043A2_WIDTH / 8);
      uint16_t i = 0;
      for (; (i < GxGDE043A2_ROW_BUFFER_SIZE) && (x < size); i++)
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
      for (; (i < GxGDE043A2_ROW_BUFFER_SIZE); i++)
      {
        row_buffer[i] = wave_end_table[0x00][frame];
      }
      IO.send_row(row_buffer, GxGDE043A2_ROW_BUFFER_SIZE, GxGDE043A2_CL_DELAY);
    }
    IO.send_row(row_buffer, GxGDE043A2_ROW_BUFFER_SIZE, GxGDE043A2_CL_DELAY);
  }
  delay(25);
  IO.powerOff();
}

void GxGDE043A2::eraseDisplay()
{
  IO.powerOn();
  delay(25);
  clear_display();
  delay(25);
  IO.powerOff();
  fillScreen(GxEPD_WHITE);
}

void GxGDE043A2::init_wave_table(void)
{
  int frame, num;
  unsigned char tmp, value;

  //wave_begin_table
  for (frame = 0; frame < GxGDE043A2_FRAME_BEGIN_SIZE; frame++)
  {
    for (num = 0; num < WAVE_TABLE_SIZE; num++)
    {
      tmp = 0;
      tmp = wave_begin_43[(num >> 6) & 0x3][frame];

      tmp = tmp << 2;
      tmp &= 0xfffc;
      tmp |= wave_begin_43[(num >> 4) & 0x3][frame];

      tmp = tmp << 2;
      tmp &= 0xfffc;
      tmp |= wave_begin_43[(num >> 2) & 0x3][frame];

      tmp = tmp << 2;
      tmp &= 0xfffc;
      tmp |= wave_begin_43[(num) & 0x3][frame];

      value = 0;
      value = (tmp << 6) & 0xc0;
      value += (tmp << 2) & 0x30;
      value += (tmp >> 2) & 0x0c;
      value += (tmp >> 6) & 0x03;
      wave_begin_table[num][frame] = value;

    }
  }

  //wave_end_table
  for (frame = 0; frame < GxGDE043A2_FRAME_END_SIZE; frame++)
  {
    for (num = 0; num < WAVE_TABLE_SIZE; num++)
    {
      tmp = 0;
      tmp = wave_end_43[(num >> 6) & 0x3][frame];

      tmp = tmp << 2;
      tmp &= 0xfffc;
      tmp |= wave_end_43[(num >> 4) & 0x3][frame];

      tmp = tmp << 2;
      tmp &= 0xfffc;
      tmp |= wave_end_43[(num >> 2) & 0x3][frame];

      tmp = tmp << 2;
      tmp &= 0xfffc;
      tmp |= wave_end_43[(num) & 0x3][frame];

      value = 0;
      value = (tmp << 6) & 0xc0;
      value += (tmp << 2) & 0x30;
      value += (tmp >> 2) & 0x0c;
      value += (tmp >> 6) & 0x03;
      wave_end_table[num][frame] = value;
    }
  }
}

void GxGDE043A2::clear_display()
{
  for (uint16_t frame = 0; frame < GxGDE043A2_FRAME_BEGIN_SIZE; frame++)
  {
    IO.start_scan();
    for (uint16_t line = 0; line < GxGDE043A2_HEIGHT; line++)
    {
      for (uint16_t i = 0; (i < GxGDE043A2_ROW_BUFFER_SIZE); i++)
      {
        row_buffer[i] = wave_begin_table[0x00][frame];
      }
      IO.send_row(row_buffer, GxGDE043A2_ROW_BUFFER_SIZE, GxGDE043A2_CL_DELAY);
    }
    IO.send_row(row_buffer, GxGDE043A2_ROW_BUFFER_SIZE, GxGDE043A2_CL_DELAY);
  }
}

void GxGDE043A2::DisplayTestPicture(uint8_t nr)
{
  unsigned char *ptr;

  IO.powerOn();
  IO.delay35ns(0xfff);

  ptr = nr == 0 ? (unsigned char *)(BitmapExample2) : (unsigned char *)(BitmapExample1);
  for (uint16_t frame = 0; frame < GxGDE043A2_FRAME_BEGIN_SIZE; frame++)
  {
    IO.start_scan();
    IO.delay35ns(0xff);
    for (uint16_t line = 0; line < GxGDE043A2_HEIGHT; line++)
    {
      uint32_t x = line * GxGDE043A2_ROW_BUFFER_SIZE;
      for (uint16_t i = 0; i < GxGDE043A2_ROW_BUFFER_SIZE; i++, x++)
      {
        row_buffer[i] = wave_begin_table[ptr[x]][frame];
      }
      IO.send_row(row_buffer, GxGDE043A2_ROW_BUFFER_SIZE, GxGDE043A2_CL_DELAY);
    }
    IO.delay35ns(0xff);
    IO.send_row(row_buffer, GxGDE043A2_ROW_BUFFER_SIZE, GxGDE043A2_CL_DELAY);
  }

  IO.delay35ns(0xfff);

  ptr = nr != 0 ? (unsigned char *)(BitmapExample1) : (unsigned char *)(BitmapExample2);
  for (uint16_t frame = 0; frame < GxGDE043A2_FRAME_END_SIZE - 2; frame++)
  {
    IO.start_scan();
    for (uint16_t line = 0; line < GxGDE043A2_HEIGHT; line++)
    {
      uint32_t x = line * GxGDE043A2_ROW_BUFFER_SIZE;
      for (uint16_t i = 0; i < GxGDE043A2_ROW_BUFFER_SIZE; i++, x++)
      {
        row_buffer[i] = wave_end_table[ptr[x]][frame];
      }
      IO.send_row(row_buffer, GxGDE043A2_ROW_BUFFER_SIZE, GxGDE043A2_CL_DELAY);
    }
    IO.send_row(row_buffer, GxGDE043A2_ROW_BUFFER_SIZE, GxGDE043A2_CL_DELAY);
  }
  IO.delay35ns(0xfff);
  IO.powerOff();
}


void GxGDE043A2::fillScreenTest()
{
  for (uint32_t x = 0; x < GxGDE043A2_BUFFER_SIZE; x++)
  {
    if (x < GxGDE043A2_BUFFER_SIZE * 3 / 8) (*p_active_buffer)[x] = 0x00;
    else if (x < GxGDE043A2_BUFFER_SIZE / 2) (*p_active_buffer)[x] = 0x55;
    else if (x < GxGDE043A2_BUFFER_SIZE * 3 / 4) (*p_active_buffer)[x] = 0xCC;
    else (*p_active_buffer)[x] = 0xFF;
  }
}


