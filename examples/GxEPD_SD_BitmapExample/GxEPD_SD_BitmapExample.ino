// GxEPD_SD_BitmapExample : test example for e-Paper displays from Waveshare and from Dalian Good Display Inc.
//
// Created by Jean-Marc Zingg based on demo code from Good Display,
// available on http://www.good-display.com/download_list/downloadcategoryid=34&isMode=false.html
//
// and of spitftbitmap.ino from Adafruit, part of Adafruit_ST7735_Library. See copyright notice at the end.
//
// The e-paper displays are available from:
//
// https://www.aliexpress.com/store/product/Wholesale-1-54inch-E-Ink-display-module-with-embedded-controller-200x200-Communicate-via-SPI-interface-Supports/216233_32824535312.html
//
// http://www.buy-lcd.com/index.php?route=product/product&path=2897_8363&product_id=35120
// or https://www.aliexpress.com/store/product/E001-1-54-inch-partial-refresh-Small-size-dot-matrix-e-paper-display/600281_32815089163.html
//

// Supporting Arduino Forum Topics:
// Waveshare e-paper displays with SPI: http://forum.arduino.cc/index.php?topic=487007.0
// Good Dispay ePaper for Arduino : https://forum.arduino.cc/index.php?topic=436411.0

// mapping suggestion from Waveshare SPI e-Paper to Wemos D1 mini
// BUSY -> D2, RST -> D4, DC -> D3, CS -> D8, CLK -> D5, DIN -> D7, GND -> GND, 3.3V -> 3.3V

// mapping suggestion from Waveshare SPI e-Paper to generic ESP8266
// BUSY -> GPIO4, RST -> GPIO2, DC -> GPIO0, CS -> GPIO15, CLK -> GPIO14, DIN -> GPIO13, GND -> GND, 3.3V -> 3.3V

// mapping suggestion for ESP32, e.g. LOLIN32, see .../variants/.../pins_arduino.h for your board
// NOTE: there are variants with different pins for SPI ! CHECK SPI PINS OF YOUR BOARD
// BUSY -> 4, RST -> 16, DC -> 17, CS -> SS(5), CLK -> SCK(18), DIN -> MOSI(23), GND -> GND, 3.3V -> 3.3V

// new mapping suggestion for STM32F1, e.g. STM32F103C8T6 "BluePill"
// BUSY -> A1, RST -> A2, DC -> A3, CS-> A4, CLK -> A5, DIN -> A7

// mapping suggestion for AVR, UNO, NANO etc.
// BUSY -> 7, RST -> 9, DC -> 8, CS-> 10, CLK -> 13, DIN -> 11
//
// **** NOTE that the mapping suggestion may need modification depending on SD board used! ****
// ********************************************************************************************
//

#define SD_CS SS  // e.g. for RobotDyn Wemos D1 mini SD board
#define EPD_CS D1 // alternative I use with RobotDyn Wemos D1 mini SD board

#include <SPI.h>
#include <SD.h>

// include library, include base class, make path known
#include <GxEPD.h>

// select the display class to use, only one
#include <GxGDEP015OC1/GxGDEP015OC1.cpp>    // 1.54" b/w
//#include <GxGDEW0154Z04/GxGDEW0154Z04.cpp>  // 1.54" b/w/r 200x200
//#include <GxGDEW0154Z17/GxGDEW0154Z17.cpp>  // 1.54" b/w/r 152x152
//#include <GxGDE0213B1/GxGDE0213B1.cpp>      // 2.13" b/w
//#include <GxGDEW0213Z16/GxGDEW0213Z16.cpp>  // 2.13" b/w/r
//#include <GxGDEH029A1/GxGDEH029A1.cpp>      // 2.9" b/w
//#include <GxGDEW029Z10/GxGDEW029Z10.cpp>    // 2.9" b/w/r
//#include <GxGDEW027C44/GxGDEW027C44.cpp>    // 2.7" b/w/r
//#include <GxGDEW027W3/GxGDEW027W3.cpp>      // 2.7" b/w
//#include <GxGDEW042T2/GxGDEW042T2.cpp>      // 4.2" b/w
//#include <GxGDEW042Z15/GxGDEW042Z15.cpp>    // 4.2" b/w/r
//#include <GxGDEW0583T7/GxGDEW0583T7.cpp>    // 5.83" b/w
//#include <GxGDEW075T8/GxGDEW075T8.cpp>      // 7.5" b/w
//#include <GxGDEW075Z09/GxGDEW075Z09.cpp>    // 7.5" b/w/r

#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
#include <GxIO/GxIO.cpp>

#if defined(ESP8266)

// generic/common.h
//static const uint8_t SS    = 15; // D8
//static const uint8_t MOSI  = 13; // D7
//static const uint8_t MISO  = 12; // D6
//static const uint8_t SCK   = 14; // D5

GxIO_Class io(SPI, /*CS=D8*/ EPD_CS, /*DC=D3*/ 0, /*RST=D4*/ 2); // arbitrary selection of D3(=0), D4(=2), selected for default of GxEPD_Class
GxEPD_Class display(io /*RST=D4*/ /*BUSY=D2*/); // default selection of D4(=2), D2(=4)
// Heltec E-Paper 1.54" b/w without BUSY
//GxEPD_Class display(io, /*RST=D4*/ 2, /*BUSY=D2*/ -1); // default selection of D4(=2), no BUSY

#elif defined(ESP32)

// pins_arduino.h, e.g. LOLIN32
//static const uint8_t SS    = 5;
//static const uint8_t MOSI  = 23;
//static const uint8_t MISO  = 19;
//static const uint8_t SCK   = 18;

GxIO_Class io(SPI, /*CS=5*/ EPD_CS, /*DC=*/ 17, /*RST=*/ 16); // arbitrary selection of 17, 16
GxEPD_Class display(io, /*RST=*/ 16, /*BUSY=*/ 4); // arbitrary selection of (16), 4

#elif defined(ARDUINO_ARCH_SAMD)

// variant.h of MKR1000
//#define PIN_SPI_MISO  (10u)
//#define PIN_SPI_MOSI  (8u)
//#define PIN_SPI_SCK   (9u)
//#define PIN_SPI_SS    (24u) // should be 4?
// variant.h of MKRZERO
//#define PIN_SPI_MISO  (10u)
//#define PIN_SPI_MOSI  (8u)
//#define PIN_SPI_SCK   (9u)
//#define PIN_SPI_SS    (4u)

GxIO_Class io(SPI, /*CS=*/ EPD_CS, /*DC=*/ 7, /*RST=*/ 6);
GxEPD_Class display(io, /*RST=*/ 6, /*BUSY=*/ 5);

#elif defined(ARDUINO_GENERIC_STM32F103C) && defined(MCU_STM32F103C8)

// STM32 Boards(STM32duino.com) Generic STM32F103C series STM32F103C8
// aka BluePill
// board.h
//#define BOARD_SPI1_NSS_PIN        PA4
//#define BOARD_SPI1_MOSI_PIN       PA7
//#define BOARD_SPI1_MISO_PIN       PA6
//#define BOARD_SPI1_SCK_PIN        PA5
//enum {
//    PA0, PA1, PA2, PA3, PA4, PA5, PA6, PA7, PA8, PA9, PA10, PA11, PA12, PA13,PA14,PA15,
//  PB0, PB1, PB2, PB3, PB4, PB5, PB6, PB7, PB8, PB9, PB10, PB11, PB12, PB13,PB14,PB15,
//  PC13, PC14,PC15
//};
// variant.h
//static const uint8_t SS   = BOARD_SPI1_NSS_PIN;
//static const uint8_t SS1  = BOARD_SPI2_NSS_PIN;
//static const uint8_t MOSI = BOARD_SPI1_MOSI_PIN;
//static const uint8_t MISO = BOARD_SPI1_MISO_PIN;
//static const uint8_t SCK  = BOARD_SPI1_SCK_PIN;

// new mapping suggestion for STM32F1, e.g. STM32F103C8T6 "BluePill"
// BUSY -> A1, RST -> A2, DC -> A3, CS-> A4, CLK -> A5, DIN -> A7

GxIO_Class io(SPI, /*CS=*/ EPD_CS, /*DC=*/ 3, /*RST=*/ 2);
GxEPD_Class display(io, /*RST=*/ 2, /*BUSY=*/ 1);

#elif defined(ARDUINO_GENERIC_STM32F103V) && defined(MCU_STM32F103VB)

// board.h
//#define BOARD_SPI1_NSS_PIN        PA4
//#define BOARD_SPI1_MOSI_PIN       PA7
//#define BOARD_SPI1_MISO_PIN       PA6
//#define BOARD_SPI1_SCK_PIN        PA5

// STM32 Boards(STM32duino.com) Generic STM32F103V series STM32F103VB
// Good Display DESPI-M01
// note: needs jumper wires from SS=PA4->CS, SCK=PA5->SCK, MOSI=PA7->SDI

GxIO_Class io(SPI, /*CS=*/ EPD_CS, /*DC=*/ PE15, /*RST=*/ PE14); // DC, RST as wired by DESPI-M01
GxEPD_Class display(io, /*RST=*/ PE14, /*BUSY=*/ PE13); // RST, BUSY as wired by DESPI-M01

#else

// pins_arduino.h, e.g. AVR
//#define PIN_SPI_SS    (10)
//#define PIN_SPI_MOSI  (11)
//#define PIN_SPI_MISO  (12)
//#define PIN_SPI_SCK   (13)

GxIO_Class io(SPI, /*CS=*/ EPD_CS, /*DC=*/ 8, /*RST=*/ 9); // arbitrary selection of 8, 9 selected for default of GxEPD_Class
GxEPD_Class display(io /*RST=9*/ /*BUSY=7*/); // default selection of (9), 7

#endif

void setup(void)
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");

  display.init(115200); // enable diagnostic output on Serial

  Serial.print("Initializing SD card...");
  if (!SD.begin(SD_CS))
  {
    Serial.println("SD failed!");
    return;
  }
  Serial.println("SD OK!");

  // change the name here!
  drawBitmapFromSD("parrot.bmp", 0, 0);
  delay(5000);
  drawBitmapFromSD("betty_1.bmp", 0, 0);
  // wait 5 seconds
  delay(5000);
}

void loop()
{
}

#if defined(__AVR) //|| true

struct Parameters
{
  char* filename;
  uint8_t x;
  uint8_t y;
};

void drawBitmapFromSD_Callback(const void* params)
{
  const Parameters* p = reinterpret_cast<const Parameters*>(params);
  bmpDraw(p->filename, p->x, p->y);
}

void drawBitmapFromSD(char *filename, uint8_t x, uint8_t y)
{
  Parameters params{filename, x, y};
  display.drawPaged(drawBitmapFromSD_Callback, &params);
}

#else

void drawBitmapFromSD(char *filename, uint8_t x, uint8_t y)
{
  bmpDraw(filename, x, y);
  display.update();
}

#endif

#define BUFFPIXEL 20

void bmpDraw(char *filename, uint8_t x, uint8_t y)
{
  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3 * BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint16_t r, g, b;               // uint16_t for calculations
  uint32_t pos = 0, startTime = millis();

  if ((x >= display.width()) || (y >= display.height())) return;

  Serial.println();
  Serial.print("Loading image '");
  Serial.print(filename);
  Serial.println('\'');

  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL)
  {
    Serial.print("File not found");
    return;
  }

  // Parse BMP header
  if (read16(bmpFile) == 0x4D42) // BMP signature
  {
    Serial.print("File size: "); Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    Serial.print("Image Offset: "); Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    Serial.print("Header size: "); Serial.println(read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if (read16(bmpFile) == 1) // # planes -- must be '1'
    {
      bmpDepth = read16(bmpFile); // bits per pixel
      Serial.print("Bit Depth: "); Serial.println(bmpDepth);
      Serial.print("Image size: ");
      Serial.print(bmpWidth);
      Serial.print('x');
      Serial.println(bmpHeight);
      if (read32(bmpFile) == 0) // 0 = uncompressed
      {
        switch (bmpDepth)
        {
          case 1: // one bit per pixel b/w format
            {
              goodBmp = true; // Supported BMP format -- proceed!
              // BMP rows are padded (if needed) to 4-byte boundary
              //rowSize = (((bmpWidth + 7) / 8) + 3) & ~3;
              rowSize = (bmpWidth * bmpDepth / 8 + 3) & ~3;
              // If bmpHeight is negative, image is in top-down order.
              // This is not canon but has been observed in the wild.
              if (bmpHeight < 0)
              {
                bmpHeight = -bmpHeight;
                flip      = false;
              }
              // Crop area to be loaded
              w = bmpWidth;
              h = bmpHeight;
              if ((x + w - 1) >= display.width())  w = display.width()  - x;
              if ((y + h - 1) >= display.height()) h = display.height() - y;
              for (row = 0; row < h; row++) // For each scanline...
              {
                if (flip) // Bitmap is stored bottom-to-top order (normal BMP)
                  pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
                else     // Bitmap is stored top-to-bottom
                  pos = bmpImageoffset + row * rowSize;
                if (bmpFile.position() != pos)
                { // Need seek?
                  bmpFile.seek(pos);
                  buffidx = sizeof(sdbuffer); // Force buffer reload
                }
                uint8_t bits;
                for (col = 0; col < w; col++) // For each pixel...
                {
                  // Time to read more pixel data?
                  if (buffidx >= sizeof(sdbuffer))
                  {
                    // Indeed
                    bmpFile.read(sdbuffer, sizeof(sdbuffer));
                    buffidx = 0; // Set index to beginning
                  }
                  if (0 == col % 8)
                  {
                    bits = sdbuffer[buffidx++];
                  }
                  uint16_t bw_color = bits & 0x80 ? GxEPD_WHITE : GxEPD_BLACK;
                  display.drawPixel(col, row, bw_color);
                  bits <<= 1;
                } // end pixel
              } // end scanline
              Serial.print("Loaded in ");
              Serial.print(millis() - startTime);
              Serial.println(" ms");
            } // end goodBmp
            break;
          case 24: // standard BMP format
            {
              goodBmp = true; // Supported BMP format -- proceed!
              // BMP rows are padded (if needed) to 4-byte boundary
              //rowSize = (bmpWidth * 3 + 3) & ~3;
              rowSize = (bmpWidth * bmpDepth / 8 + 3) & ~3;
              // If bmpHeight is negative, image is in top-down order.
              // This is not canon but has been observed in the wild.
              if (bmpHeight < 0)
              {
                bmpHeight = -bmpHeight;
                flip      = false;
              }
              // Crop area to be loaded
              w = bmpWidth;
              h = bmpHeight;
              if ((x + w - 1) >= display.width())  w = display.width()  - x;
              if ((y + h - 1) >= display.height()) h = display.height() - y;
              for (row = 0; row < h; row++) // For each scanline...
              {
                if (flip) // Bitmap is stored bottom-to-top order (normal BMP)
                  pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
                else     // Bitmap is stored top-to-bottom
                  pos = bmpImageoffset + row * rowSize;
                if (bmpFile.position() != pos)
                { // Need seek?
                  bmpFile.seek(pos);
                  buffidx = sizeof(sdbuffer); // Force buffer reload
                }
                for (col = 0; col < w; col++) // For each pixel...
                {
                  // Time to read more pixel data?
                  if (buffidx >= sizeof(sdbuffer))
                  {
                    // Indeed
                    bmpFile.read(sdbuffer, sizeof(sdbuffer));
                    buffidx = 0; // Set index to beginning
                  }
                  // Convert pixel from BMP to TFT format, push to display
                  b = sdbuffer[buffidx++];
                  g = sdbuffer[buffidx++];
                  r = sdbuffer[buffidx++];
                  uint16_t bw_color = ((r + g + b) / 3 > 0xFF  / 2) ? GxEPD_WHITE : GxEPD_BLACK;
                  display.drawPixel(col, row, bw_color);
                } // end pixel
              } // end scanline
              Serial.print("Loaded in ");
              Serial.print(millis() - startTime);
              Serial.println(" ms");
            } // end goodBmp
            break;
        }
      }
    }
  }

  bmpFile.close();
  if (!goodBmp) Serial.println("BMP format not recognized.");
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File f)
{
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File f)
{
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

/***************************************************
  This is a library for the Adafruit 1.8" SPI display.

  This library works with the Adafruit 1.8" TFT Breakout w/SD card
  ----> http://www.adafruit.com/products/358
  The 1.8" TFT shield
  ----> https://www.adafruit.com/product/802
  The 1.44" TFT breakout
  ----> https://www.adafruit.com/product/2088
  as well as Adafruit raw 1.8" TFT display
  ----> http://www.adafruit.com/products/618

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

