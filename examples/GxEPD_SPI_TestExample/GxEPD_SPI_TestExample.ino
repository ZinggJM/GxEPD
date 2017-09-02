/************************************************************************************
   GxEPD_SPI_TestExample : test example for e-Paper displays from Dalian Good Display Co., Ltd.: www.good-display.com

   based on Demo Example from Good Display, now available on http://www.good-display.com/download_list/downloadcategoryid=34&isMode=false.html

   Author : J-M Zingg

   modified by :

   Version : 2.0

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

// Supporting Arduino Forum Topics:
// Waveshare e-paper displays with SPI: http://forum.arduino.cc/index.php?topic=487007.0
// Good Dispay ePaper for Arduino : https://forum.arduino.cc/index.php?topic=436411.0

// mapping from Waveshare 2.9inch e-Paper to Wemos D1 mini
// BUSY -> D2, RST -> D4, DC -> D3, CS -> D8, CLK -> D5, DIN -> D7, GND -> GND, 3.3V -> 3.3V

// mapping example for AVR, UNO, NANO etc.
// BUSY -> 7, RST -> 9, DC -> 8, C S-> 10, CLK -> 13, DIN -> 11

// include library, include base class, make path known
#include <GxEPD.h>

// select the display class to use, only one
//#include <GxGDEP015OC1/GxGDEP015OC1.cpp>
//#include <GxGDE0213B1/GxGDE0213B1.cpp>
#include <GxGDEH029A1/GxGDEH029A1.cpp>
//#include <GxGDEW027C44/GxGDEW027C44.cpp>
//#include <GxGDEW042T2/GxGDEW042T2.cpp>
//#include <GxGDEW075T8/GxGDEW075T8.cpp>

// uncomment next line for drawBitmap() test, (consumes RAM on ESP8266)
#include GxEPD_BitmapExamples

// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>


#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
#include <GxIO/GxIO.cpp>

#if defined(ESP8266)

// generic/common.h
//static const uint8_t SS    = 15;
//static const uint8_t MOSI  = 13;
//static const uint8_t MISO  = 12;
//static const uint8_t SCK   = 14;
// pins_arduino.h
//static const uint8_t D8   = 15;
//static const uint8_t D7   = 13;
//static const uint8_t D6   = 12;
//static const uint8_t D5   = 14;

// GxIO_SPI(SPIClass& spi, int8_t cs, int8_t dc, int8_t rst = -1, int8_t bl = -1);
GxIO_Class io(SPI, SS, D3, D4); // arbitrary selection of D3, D4 selected for default of GxEPD_Class
// GxGDEP015OC1(GxIO& io, uint8_t rst = D4, uint8_t busy = D2);
GxEPD_Class display(io); // default selection of D4, D2
// my IoT connection, busy on MISO
//GxEPD_Class display(io, D4, D6);

#elif defined(ESP32)

// pins_arduino.h, e.g. LOLIN32
//static const uint8_t SS    = 5;
//static const uint8_t MOSI  = 23;
//static const uint8_t MISO  = 19;
//static const uint8_t SCK   = 18;

// GxIO_SPI(SPIClass& spi, int8_t cs, int8_t dc, int8_t rst = -1, int8_t bl = -1);
GxIO_Class io(SPI, SS, 17, 16); // arbitrary selection of 17, 16
// GxGDEP015OC1(GxIO& io, uint8_t rst = D4, uint8_t busy = D2);
GxEPD_Class display(io, 16, 4); // arbitrary selection of (16), 4

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

GxIO_Class io(SPI, 4, 7, 6);
GxEPD_Class display(io, 6, 5);

#elif defined(_BOARD_GENERIC_STM32F103C_H_)

// STM32 Boards (STM32duino.com)
// Generic STM32F103C series
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

// GxIO_SPI(SPIClass& spi, int8_t cs, int8_t dc, int8_t rst = -1, int8_t bl = -1);
GxIO_Class io(SPI, SS, 8, 9);
// GxGDEP015OC1(GxIO& io, uint8_t rst = 9, uint8_t busy = 7);
GxEPD_Class display(io, 9, 3);

#else

// pins_arduino.h, e.g. AVR
//#define PIN_SPI_SS    (10)
//#define PIN_SPI_MOSI  (11)
//#define PIN_SPI_MISO  (12)
//#define PIN_SPI_SCK   (13)

GxIO_Class io(SPI, SS, 8, 9); // arbitrary selection of 8, 9 selected for default of GxEPD_Class
//GxIO_DESTM32L io;
//GxIO_GreenSTM32F103V io;
GxEPD_Class display(io);

#endif


void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");

  display.init();

  Serial.println("setup done");
}

void loop()
{
  showBitmapExample();
#if defined(_GxGDEP015OC1_H_) || defined(_GxGDE0213B1_H_) || defined(_GxGDEH029A1_H_)
  showGridIcons();
#endif
  //drawCornerTest();
  showFont("FreeMonoBold9pt7b", &FreeMonoBold9pt7b);
  showFont("FreeMonoBold12pt7b", &FreeMonoBold12pt7b);
  //showFont("FreeMonoBold18pt7b", &FreeMonoBold18pt7b);
  //showFont("FreeMonoBold24pt7b", &FreeMonoBold24pt7b);
  delay(10000);
}

#if defined(_GxGDEP015OC1_H_) || defined(_GxGDE0213B1_H_) || defined(_GxGDEH029A1_H_)
#include "IMG_0001.h"
#endif

void showBitmapExample()
{
#ifdef _GxBitmapExamples_H_
#ifdef _GxGDEW027C44_H_
  // draw black and red bitmap
  display.drawPicture(BitmapExample1, BitmapExample2, sizeof(BitmapExample1));
  delay(2000);
  display.setRotation(3);
#elif defined(_GxGDE0213B1_H_)
  display.drawBitmap(BitmapExample1, sizeof(BitmapExample1));
  delay(2000);
  display.drawBitmap(BitmapExample2, sizeof(BitmapExample2));
  delay(2000);
  display.drawBitmap(first, sizeof(first));
  delay(2000);
#if !defined(__AVR)
  display.drawBitmap(second, sizeof(second));
  delay(2000);
  display.drawBitmap(third, sizeof(third));
  delay(2000);
#endif
#elif defined(_GxGDEW042T2_H_) && defined(__AVR)
  display.drawBitmap(BitmapExample1, sizeof(BitmapExample1));
  delay(2000);
  display.fillScreen(GxEPD_WHITE);
  display.drawBitmap(0, 0, BitmapExample1, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
  display.update();
  delay(10000);
#else
  display.drawBitmap(BitmapExample1, sizeof(BitmapExample1));
  delay(2000);
  display.drawBitmap(BitmapExample2, sizeof(BitmapExample2));
  delay(2000);
  display.setRotation(2);
  display.fillScreen(GxEPD_WHITE);
  // display.drawBitmap(0, 0, BitmapExample1, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK); // old signature
  // to buffer, may be cropped, drawPixel() used, update needed, new signature, mirror default set for example bitmaps (display class dependent)
  // void  drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, bool mirror = true);
  display.drawBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK); // new signature
  display.update();
  delay(10000);
#if defined(_GxGDEP015OC1_H_) || defined(_GxGDE0213B1_H_) || defined(_GxGDEH029A1_H_)
  display.fillScreen(GxEPD_WHITE);
  // thanks to bytecrusher: http://forum.arduino.cc/index.php?topic=487007.msg3367378#msg3367378
  display.drawBitmap(gImage_IMG_0001, 50, 50, 64, 180, GxEPD_BLACK); // new signature
  display.update();
  delay(10000);
#endif
#endif
#endif
}

void showFont(const char name[], const GFXfont* f)
{
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(f);
  display.setCursor(0, 0);
  display.println();
  display.println(name);
  display.println(" !\"#$%&'()*+,-./");
  display.println("0123456789:;<=>?");
  display.println("@ABCDEFGHIJKLMNO");
  display.println("PQRSTUVWXYZ[\\]^_");
#ifdef _GxGDEW027C44_H_
  display.setTextColor(GxEPD_RED);
#endif
  display.println("`abcdefghijklmno");
  display.println("pqrstuvwxyz{|}~ ");
  display.update();
  delay(5000);
}

void drawCornerTest()
{
#if defined(_GxGDE0213B1_H_) || defined(_GxGDEH029A1_H_)
  display.drawCornerTest();
  delay(5000);
#endif
  for (uint16_t r = 0; r < 4; r++)
  {
    display.setRotation(r);
    display.fillScreen(GxEPD_WHITE);
    display.fillRect(0, 0, 8, 8, GxEPD_BLACK);
    display.fillRect(display.width() - 18, 0, 16, 16, GxEPD_BLACK);
    display.fillRect(display.width() - 25, display.height() - 25, 24, 24, GxEPD_BLACK);
    display.fillRect(0, display.height() - 33, 32, 32, GxEPD_BLACK);
    display.update();
    delay(5000);
  }
}

#if defined(_GxGDEP015OC1_H_) || defined(_GxGDE0213B1_H_) || defined(_GxGDEH029A1_H_)

#include "imglib/gridicons_add_image.h"
#include "imglib/gridicons_add_outline.h"
#include "imglib/gridicons_add.h"
#include "imglib/gridicons_align_center.h"
#include "imglib/gridicons_align_image_center.h"
#include "imglib/gridicons_align_image_left.h"
#include "imglib/gridicons_align_image_none.h"
#include "imglib/gridicons_align_image_right.h"
#include "imglib/gridicons_align_justify.h"
#include "imglib/gridicons_align_left.h"
#include "imglib/gridicons_align_right.h"
#include "imglib/gridicons_arrow_down.h"
#include "imglib/gridicons_arrow_left.h"
#include "imglib/gridicons_arrow_right.h"
#include "imglib/gridicons_arrow_up.h"

void showGridIcons()
{
  display.fillScreen(GxEPD_WHITE);
  display.drawBitmap(0, 0, gridicons_add_image, 24, 24, GxEPD_BLACK);
  display.drawBitmap(25, 0, gridicons_add_outline, 24, 24, GxEPD_BLACK);
  display.drawBitmap(50, 0, gridicons_add, 24, 24, GxEPD_BLACK);
  display.drawBitmap(75, 0, gridicons_align_center, 24, 24, GxEPD_BLACK);
  display.drawBitmap(0, 25, gridicons_align_image_center, 24, 24, GxEPD_BLACK);
  display.drawBitmap(25, 25, gridicons_align_image_left, 24, 24, GxEPD_BLACK);
  display.drawBitmap(50, 25, gridicons_align_image_none, 24, 24, GxEPD_BLACK);
  display.drawBitmap(75, 25, gridicons_align_image_right, 24, 24, GxEPD_BLACK);
  display.drawBitmap(0, 50, gridicons_align_justify, 24, 24, GxEPD_BLACK);
  display.drawBitmap(25, 50, gridicons_align_left, 24, 24, GxEPD_BLACK);
  display.drawBitmap(50, 50, gridicons_align_right, 24, 24, GxEPD_BLACK);
  display.drawBitmap(75, 50, gridicons_arrow_down, 24, 24, GxEPD_BLACK);
  display.drawBitmap(0, 75, gridicons_arrow_left, 24, 24, GxEPD_BLACK);
  display.drawBitmap(25, 75, gridicons_arrow_right, 24, 24, GxEPD_BLACK);
  display.drawBitmap(50, 75, gridicons_arrow_up, 24, 24, GxEPD_BLACK);
  display.update();
  delay(1000);
}

#endif

