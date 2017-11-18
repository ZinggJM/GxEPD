/************************************************************************************
   GxEPD_SPI_MultiExample : test example for e-Paper displays from Dalian Good Display Co., Ltd.: www.good-display.com

   based on Demo Example from Good Display, now available on http://www.good-display.com/download_list/downloadcategoryid=34&isMode=false.html

   Author : J-M Zingg

   Version : 2.3

   Support: limited, provided as example, no claim to be fit for serious use

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

// Supporting Arduino Forum Topics:
// Waveshare e-paper displays with SPI: http://forum.arduino.cc/index.php?topic=487007.0
// Good Dispay ePaper for Arduino : https://forum.arduino.cc/index.php?topic=436411.0

// mapping suggestion from Waveshare 2.9inch e-Paper to Wemos D1 mini
// BUSY -> D2, RST -> D4, DC -> D3, CS -> D8, CLK -> D5, DIN -> D7, GND -> GND, 3.3V -> 3.3V

// mapping suggestion for ESP32, e.g. LOLIN32, see .../variants/.../pins_arduino.h for your board
// NOTE: there are variants with different pins for SPI ! CHECK SPI PINS OF YOUR BOARD
// BUSY -> 4, RST -> 16, DC -> 17, CS -> SS(5), CLK -> SCK(18), DIN -> MOSI(23), GND -> GND, 3.3V -> 3.3V

// mapping suggestion for AVR, UNO, NANO etc.
// BUSY -> 7, RST -> 9, DC -> 8, CS-> 10, CLK -> 13, DIN -> 11

// include library, include base class, make path known
#include <GxEPD.h>

// select the display classes to use
#include <GxGDEP015OC1/GxGDEP015OC1.cpp>    // 1.54" b/w
#include <GxGDEW0154Z04/GxGDEW0154Z04.cpp>  // 1.54" b/w/r
#include <GxGDE0213B1/GxGDE0213B1.cpp>      // 2.13" b/w
#include <GxGDEW0213Z16/GxGDEW0213Z16.cpp>  // 2.13" b/w/r
#include <GxGDEH029A1/GxGDEH029A1.cpp>      // 2.9" b/w
#include <GxGDEW029Z10/GxGDEW029Z10.cpp>    // 2.9" b/w/r
#include <GxGDEW027C44/GxGDEW027C44.cpp>    // 2.7" b/w/r
#include <GxGDEW042T2/GxGDEW042T2.cpp>      // 4.2" b/w
#include <GxGDEW042T2_FPU/GxGDEW042T2_FPU.cpp>      // 4.2" b/w
#include <GxGDEW075T8/GxGDEW075T8.cpp>      // 7.5" b/w

// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>

#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
#include <GxIO/GxIO.cpp>

#if defined(ESP8266)

// create GxIO_SPI instances for each display, each instance with different CS line;
// disable reset line to disable cross resets by multiple instances
// GxIO_SPI(SPIClass& spi, int8_t cs, int8_t dc, int8_t rst = -1, int8_t bl = -1);
GxIO_SPI io1(SPI, 15, 2, -1); // CS = D8(15), DC = D4(2), RST disabled
GxIO_SPI io2(SPI, 16, 2, -1); // CS = D0(16), DC = D4(2), RST disabled
GxIO_SPI io3(SPI, 5, 2, -1); // CS = D1(5), DC = D4(2), RST disabled

// create display class instances for each display, each instance with different BUSY line, or BUSY lines or-ed to one pin
// GxGDEP015OC1(GxIO& io, int8_t rst = 2, int8_t busy = 4);
//GxGDEP015OC1 display1(io1, -1, 4); // RST  disabled, BUSY = D2(4)
//GxGDEH029A1 display2(io2, -1, 12); // RST  disabled, BUSY = D6(12)

// BUSY lines can be or-ed with diodes and pulldown register for displays with BUSY active HIGH
GxGDEP015OC1 display1(io1, -1, 4); // RST  disabled, BUSY = D2(4)
GxGDEH029A1 display2(io2, -1, 4); // RST  disabled, BUSY = D2(4)
GxGDE0213B1 display3(io3, -1, 4); // RST  disabled, BUSY = D2(4)

#define RST_PIN 0 // D3(0)

#elif defined(ESP32)

// create GxIO_SPI instances for each display, each instance with different CS line;
// disable reset line to disable cross resets by multiple instances
// GxIO_SPI(SPIClass& spi, int8_t cs, int8_t dc, int8_t rst = -1, int8_t bl = -1);
GxIO_Class io1(SPI, 5, 17, -1); // CS = 5(SS), DC = 17, RST disabled
GxIO_Class io2(SPI, 0, 17, -1); // CS = 0, DC = 17, RST disabled
GxIO_Class io3(SPI, 2, 17, -1); // CS = 2, DC = 17, RST disabled
GxIO_Class io4(SPI, 13, 17, -1); // CS = 13, DC = 17, RST disabled

// create display class instances for each display, each instance with different BUSY line, or BUSY lines or-ed to one pin
// GxGDEP015OC1(GxIO& io, int8_t rst = D4, int8_t busy = D2);
// BUSY lines can be or-ed with diodes and pulldown resistor for displays with BUSY active HIGH
GxGDEP015OC1 display1(io1, -1, 4); // BUSY = 4, or-ed
GxGDEH029A1 display2(io2, -1, 4); // BUSY = 4, or-ed
GxGDE0213B1 display3(io3, -1, 4); // BUSY = 4, or-ed

// BUSY lines can be or-ed with diodes and pulldown resistor for displays with BUSY active LOW
//GxGDEW027C44 display4(io4, -1, 15); // BUSY = 15, active LOW
//GxGDEW029Z10 display4(io4, -1, 15); // BUSY = 15, or-ed
GxGDEW042T2 display4(io4, -1, 15); // BUSY = 15, or-ed
//GxGDEW042T2_FPU display4(io4, -1, 15); // BUSY = 15, or-ed
//GxGDEW075T8 display4(io4, -1, 15); // BUSY = 15, or-ed
//GxGDEW0154Z04 display4(io4, -1, 15); // BUSY = 15, or-ed
//GxGDEW0213Z16 display4(io4, -1, 15); // BUSY = 15, or-ed

#define RST_PIN 16

#endif

#if defined(_GxGDEP015OC1_H_)
namespace BMxGDEP015OC1
{
#include "GxGDEP015OC1/BitmapExamples.h"
void showBitmapExample(GxEPD& display)
{
  display.drawExampleBitmap(BitmapExample1, sizeof(BitmapExample1));
  delay(2000);
  display.drawExampleBitmap(BitmapExample2, sizeof(BitmapExample2));
  delay(2000);
  display.setRotation(0);
  display.fillScreen(GxEPD_WHITE);
  display.drawExampleBitmap(BitmapExample1, 0, 0, GxGDEP015OC1_WIDTH, GxGDEP015OC1_HEIGHT, GxEPD_BLACK);
  display.update();
  delay(2000);
}
// fix this later
#undef BitmapExample1
#undef BitmapExample2
#undef _GxBitmapExamples_H_
}
#endif

#if defined(_GxGDEW0154Z04_H_)
namespace BMxGDEW0154Z04
{
#include "GxGDEW0154Z04/BitmapExamples.h"
void showBitmapExample(GxEPD& display)
{
  display.drawPicture(BitmapWaveshare_black, BitmapWaveshare_red, sizeof(BitmapWaveshare_black), sizeof(BitmapWaveshare_red), GxEPD::bm_normal);
  delay(2000);
  display.drawExamplePicture(BitmapExample1, BitmapExample2, sizeof(BitmapExample1), sizeof(BitmapExample2));
  delay(2000);
}
// fix this later
#undef BitmapExample1
#undef BitmapExample2
#undef _BitmapWaveshare_H_
#undef _GxBitmapExamples_H_
}
#endif

#if defined(_GxGDE0213B1_H_)
namespace BMxGDE0213B1
{
#include "GxGDE0213B1/BitmapExamples.h"
void showBitmapExample(GxEPD& display)
{
  display.drawExampleBitmap(BitmapExample1, sizeof(BitmapExample1));
  delay(2000);
  display.drawExampleBitmap(BitmapExample2, sizeof(BitmapExample2));
  delay(2000);
  display.setRotation(0);
  display.fillScreen(GxEPD_WHITE);
  display.drawExampleBitmap(BitmapExample1, 0, 0, GxGDE0213B1_WIDTH, GxGDE0213B1_HEIGHT, GxEPD_BLACK);
  display.update();
  delay(2000);
}
#undef _GxBitmapExamples_H_
}
#endif

#if defined(_GxGDEW0213Z16_H_)
namespace BMxGDEW0213Z16
{
#include "GxGDEW0213Z16/BitmapExamples.h"
void showBitmapExample(GxEPD& display)
{
  display.drawPicture(BitmapWaveshare_black, BitmapWaveshare_red, sizeof(BitmapWaveshare_black), sizeof(BitmapWaveshare_red));
  delay(2000);
  display.drawExamplePicture(BitmapExample1, BitmapExample2, sizeof(BitmapExample1), sizeof(BitmapExample2));
  delay(2000);
  display.drawExamplePicture(BitmapExample3, BitmapExample4, sizeof(BitmapExample3), sizeof(BitmapExample4));
  delay(2000);
  display.drawExampleBitmap(BitmapWaveshare_black, sizeof(BitmapWaveshare_black));
  delay(2000);
  // example bitmaps for b/w/r are normal on b/w, but inverted on red
  display.drawExampleBitmap(BitmapExample1, sizeof(BitmapExample1));
  delay(2000);
  display.drawExampleBitmap(BitmapExample2, sizeof(BitmapExample2), GxEPD::bm_invert);
  delay(2000);
  display.drawExampleBitmap(BitmapExample1, 0, 0, GxGDEW0213Z16_WIDTH, GxGDEW0213Z16_HEIGHT, GxEPD_BLACK);
  display.update();
  delay(2000);
}
#undef _BitmapWaveshare_H_
#undef _GxBitmapExamples_H_
}
#endif

#if defined(_GxGDEH029A1_H_)
namespace BMxGDEH029A1
{
#include "GxGDEH029A1/BitmapExamples.h"
void showBitmapExample(GxEPD& display)
{
  display.drawExampleBitmap(BitmapExample1, sizeof(BitmapExample1));
  delay(2000);
  display.drawExampleBitmap(BitmapExample2, sizeof(BitmapExample2));
  delay(2000);
  display.setRotation(0);
  display.fillScreen(GxEPD_WHITE);
  display.drawExampleBitmap(BitmapExample1, 0, 0, GxGDEH029A1_WIDTH, GxGDEH029A1_HEIGHT, GxEPD_BLACK);
  display.update();
  delay(2000);
}
#undef _BitmapWaveshare_H_
#undef _GxBitmapExamples_H_
}
#endif

#if defined(_GxGDEW029Z10_H_)
namespace BMxGDEW029Z10
{
#include "GxGDEW029Z10/BitmapExamples.h"
void showBitmapExample(GxEPD& display)
{
  display.drawPicture(BitmapWaveshare_black, BitmapWaveshare_red, sizeof(BitmapWaveshare_black), sizeof(BitmapWaveshare_red));
  delay(2000);
  display.drawExamplePicture(BitmapExample1, BitmapExample2, sizeof(BitmapExample1), sizeof(BitmapExample2));
  delay(2000);
  display.drawExamplePicture(BitmapExample3, BitmapExample4, sizeof(BitmapExample3), sizeof(BitmapExample4));
  delay(2000);
  display.drawExampleBitmap(BitmapWaveshare_black, sizeof(BitmapWaveshare_black));
  delay(2000);
  // example bitmaps for b/w/r are normal on b/w, but inverted on red
  display.drawExampleBitmap(BitmapExample1, sizeof(BitmapExample1));
  delay(2000);
  display.drawExampleBitmap(BitmapExample2, sizeof(BitmapExample2), GxEPD::bm_invert);
  delay(2000);
  display.drawExampleBitmap(BitmapExample1, 0, 0, GxGDEW029Z10_WIDTH, GxGDEW029Z10_HEIGHT, GxEPD_BLACK);
  display.update();
  delay(2000);
}
#undef _BitmapWaveshare_H_
#undef _GxBitmapExamples_H_
}
#endif

#if defined(_GxGDEW027C44_H_)
namespace BMxGDEW027C44
{
#include "GxGDEW027C44/BitmapExamples.h"
void showBitmapExample(GxEPD& display)
{
  // draw black and red bitmap
  display.drawExamplePicture(BitmapExample1, BitmapExample2, sizeof(BitmapExample1), sizeof(BitmapExample2));
  delay(2000);
  display.drawExampleBitmap(BitmapExample1, 0, 0, GxGDEW027C44_WIDTH, GxGDEW027C44_HEIGHT, GxEPD_BLACK);
  display.update();
  delay(2000);
}
#undef _GxBitmapExamples_H_
}
#endif

#if defined(_GxGDEW042T2_H_)
namespace BMxGDEW042T2
{
#include "GxGDEW042T2/BitmapExamples.h"
void showBitmapExample(GxEPD& display)
{
  display.drawExampleBitmap(BitmapExample1, sizeof(BitmapExample1));
  delay(2000);
  display.drawExampleBitmap(BitmapExample2, sizeof(BitmapExample2));
  delay(2000);
  display.setRotation(0);
  display.fillScreen(GxEPD_WHITE);
  display.drawExampleBitmap(BitmapExample1, 0, 0, GxGDEW042T2_WIDTH, GxGDEW042T2_HEIGHT, GxEPD_BLACK);
  display.update();
  delay(2000);
}
#undef _GxBitmapExamples_H_
}
#endif

#if defined(_GxGDEW042T2_FPU_H_)
namespace BMxGDEW042T2_FPU
{
#include "GxGDEW042T2_FPU/BitmapExamples.h"
void showBitmapExample(GxEPD& display)
{
  display.drawExampleBitmap(BitmapExample1, sizeof(BitmapExample1));
  delay(2000);
  display.drawExampleBitmap(BitmapExample2, sizeof(BitmapExample2));
  delay(2000);
  display.setRotation(0);
  display.fillScreen(GxEPD_WHITE);
  display.drawExampleBitmap(BitmapExample1, 0, 0, GxGDEW042T2_FPU_WIDTH, GxGDEW042T2_FPU_HEIGHT, GxEPD_BLACK);
  display.update();
  delay(2000);
}
#undef _GxBitmapExamples_H_
}
#endif

#if defined(_GxGDEW075T8_H_)
namespace BMxGDEW075T8
{
#include "GxGDEW075T8/BitmapExamples.h"
void showBitmapExample(GxEPD& display)
{
  display.drawExampleBitmap(BitmapExample1, sizeof(BitmapExample1));
  delay(2000);
  display.drawExampleBitmap(BitmapExample2, sizeof(BitmapExample2));
  delay(2000);
  display.setRotation(0);
  display.fillScreen(GxEPD_WHITE);
  display.drawExampleBitmap(BitmapExample1, 0, 0, GxGDEW075T8_WIDTH, GxGDEW075T8_HEIGHT, GxEPD_BLACK);
  display.update();
  delay(2000);
}
#undef _GxBitmapExamples_H_
}
#endif

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");

  // one common reset for all displays
  pinMode(RST_PIN, OUTPUT);
  digitalWrite(RST_PIN, LOW);
  delay(20);
  digitalWrite(RST_PIN, HIGH);
  delay(20);

  display1.init();
  display2.init();
  display3.init();
#if defined(ESP32)
  display4.init();
#endif

  Serial.println("setup done");
}

void loop()
{
  Serial.println("BMxGDEP015OC1::showBitmapExample(display1)");
  BMxGDEP015OC1::showBitmapExample(display1);
  Serial.println("BMxGDEH029A1::showBitmapExample(display2)");
  BMxGDEH029A1::showBitmapExample(display2);
  Serial.println("BMxGDE0213B1::showBitmapExample(display3)");
  BMxGDE0213B1::showBitmapExample(display3);
  Serial.println("showFont(display1, \"FreeMonoBold9pt7b\", &FreeMonoBold9pt7b)");
  showFont(display1, "FreeMonoBold9pt7b", &FreeMonoBold9pt7b);
  Serial.println("showFont(display2, \"FreeMonoBold9pt7b\", &FreeMonoBold9pt7b)");
  showFont(display2, "FreeMonoBold9pt7b", &FreeMonoBold9pt7b);
  Serial.println("showFont(display3, \"FreeMonoBold9pt7b\", &FreeMonoBold9pt7b)");
  showFont(display3, "FreeMonoBold9pt7b", &FreeMonoBold9pt7b);
#if defined(ESP32)
  //Serial.println("BMxGDEW027C44::showBitmapExample(display4)");
  //BMxGDEW027C44::showBitmapExample(display4);
  Serial.println("BMxGDEW042T2::showBitmapExample(display4)");
  BMxGDEW042T2::showBitmapExample(display4);
  Serial.println("showFont(display4, \"FreeMonoBold9pt7b\", &FreeMonoBold9pt7b)");
  showFont(display4, "FreeMonoBold9pt7b", &FreeMonoBold9pt7b);
#endif
  delay(10000);
}

void showFont(GxEPD& display, const char name[], const GFXfont* f)
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
#if defined(_GxGDEW0154Z04_H_) || defined(_GxGDEW0213Z16_H_) || defined(_GxGDEW029Z10_H_) || defined(_GxGDEW027C44_H_)
  display.setTextColor(GxEPD_RED);
#endif
  display.println("`abcdefghijklmno");
  display.println("pqrstuvwxyz{|}~ ");
  display.update();
  delay(5000);
}



