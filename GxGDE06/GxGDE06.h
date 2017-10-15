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

#ifndef _GxGDE06_H_
#define _GxGDE06_H_

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include "../GxEPD.h"
#include "../GxIO/GxIO_GreenSTM32F103V/GxIO_GreenSTM32F103V.h"

#define GxGDE06_WIDTH 800
#define GxGDE06_HEIGHT 600
//#define GxGDE06_BUFFER_SIZE (GxGDE06_WIDTH * GxGDE06_HEIGHT / 8) // b/w only
// reduce slightly to fit into processor RAM
#define GxGDE06_BUFFER_SIZE (GxGDE06_WIDTH * GxGDE06_HEIGHT / 8 - 6200) // b/w only

#define GxGDE06_FRAME_BEGIN_SIZE    10
const uint8_t wave_begin[4][GxGDE06_FRAME_BEGIN_SIZE] =
{
  0, 0, 0, 0, 1, 2, 2, 2, 2, 0,           //GC0->GC3
  0, 0, 0, 1, 1, 2, 2, 2, 2, 0,           //GC1->GC3
  0, 0, 1, 1, 1, 2, 2, 2, 2, 0,           //GC2->GC3
  0, 1, 1, 1, 1, 2, 2, 2, 2, 0,           //GC3->GC3
};

#define GxGDE06_FRAME_END_SIZE 15
const uint8_t wave_end[4][GxGDE06_FRAME_END_SIZE] =
{
  0, 1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0,        //GC3->GC0
  0, 1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 0, 0, 0, 0,        //GC3->GC1
  0, 1, 1, 1, 1, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0,        //GC3->GC2
  0, 1, 1, 1, 1, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0,        //GC3->GC3
};

const uint8_t bw2grey[] = {
  0b00000000, 0b00000011, 0b00001100, 0b00001111, 
  0b00110000, 0b00110011, 0b00111100, 0b00111111,
  0b11000000, 0b11000011, 0b11001100, 0b11001111,
  0b11110000, 0b11110011, 0b11111100, 0b11111111,
};

// fixed, multiple of 64 and >= GxGDE06_WIDTH / 4
#define WAVE_TABLE_SIZE 256

#define GxGDE06_ROW_BUFFER_SIZE (GxGDE06_WIDTH / 4)

typedef uint8_t epd_buffer_type[GxGDE06_BUFFER_SIZE];

class GxGDE06 : public GxEPD
{
  public:
    GxGDE06(GxIO_GreenSTM32F103V& io);
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void init(void);
    void fillScreen(uint16_t color); // to buffer
    void update(void);
    // monochrome bitmap to buffer, may be cropped, drawPixel() used, update needed, signature like Adafruit_GFX
    void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);
    // to buffer, may be cropped, drawPixel() used, update needed, new signature, may support some bm_modes
    void drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t m = bm_normal);
    // to full screen, filled with white if size is less, no update needed
    void drawPicture(const uint8_t *picture, uint32_t size); // 4 gray levels
    // to full screen, filled with white if size is less, no update needed
    void drawBitmap(const uint8_t *bitmap, uint32_t size, int16_t m = bm_normal); // black/white, parameter m ignored
    // undo last drawBitmap to prepare for next drawBitmap (turn display white);
    // any bitmap can be used, but real last bitmap gives slightly better result
    void erasePicture(const uint8_t *picture, uint32_t size);
    void eraseBitmap(const uint8_t *bitmap, uint32_t size);
    void clearDisplay(); // alternative to eraseBitmap, without bitmap
    void DisplayTestPicture(const uint8_t *picture);
    void fillScreenTest();
    void eraseDisplay(bool using_partial_update = false){clearDisplay();}; // parameter ignored
    // partial update, not implemented
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true){};
    // paged drawing, for limited RAM, drawCallback() is called GxGDEH029A1_PAGES times
    // each call of drawCallback() should draw the same
    void drawPaged(void (*drawCallback)(void)){};
    void drawCornerTest(uint8_t em = 0x01){};
  private:
    void init_wave_table(void);
    void clear_display();
  private:
    uint8_t wave_begin_table[WAVE_TABLE_SIZE][GxGDE06_FRAME_BEGIN_SIZE];
    uint8_t wave_end_table[WAVE_TABLE_SIZE][GxGDE06_FRAME_END_SIZE];
    uint8_t row_buffer[GxGDE06_ROW_BUFFER_SIZE];
    epd_buffer_type epd_buffer;
    GxIO_GreenSTM32F103V& IO;
};

#define GxEPD_Class GxGDE06

#define GxEPD_WIDTH GxGDE06_WIDTH
#define GxEPD_HEIGHT GxGDE06_HEIGHT
#define GxEPD_BitmapExamples <GxGDE06/BitmapExamples.h>
#define GxEPD_BitmapExamplesQ "GxGDE06/BitmapExamples.h"

#endif

