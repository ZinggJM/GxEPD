// PartialUpdateExample : example for Waveshare 1.54", 2.31" and 2.9" e-Paper and the same e-papers from Dalian Good Display Inc.
//
// Created by Jean-Marc Zingg based on demo code from Good Display for GDEP015OC1.
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

// mapping suggestion from Waveshare 2.9inch e-Paper to Wemos D1 mini
// BUSY -> D2, RST -> D4, DC -> D3, CS -> D8, CLK -> D5, DIN -> D7, GND -> GND, 3.3V -> 3.3V

// mapping suggestion from Waveshare 2.9inch e-Paper to generic ESP8266
// BUSY -> GPIO4, RST -> GPIO2, DC -> GPIO0, CS -> GPIO15, CLK -> GPIO14, DIN -> GPIO13, GND -> GND, 3.3V -> 3.3V

// mapping suggestion for ESP32, e.g. LOLIN32, see .../variants/.../pins_arduino.h for your board
// NOTE: there are variants with different pins for SPI ! CHECK SPI PINS OF YOUR BOARD
// BUSY -> 4, RST -> 16, DC -> 17, CS -> SS(5), CLK -> SCK(18), DIN -> MOSI(23), GND -> GND, 3.3V -> 3.3V

// mapping suggestion for AVR, UNO, NANO etc.
// BUSY -> 7, RST -> 9, DC -> 8, CS-> 10, CLK -> 13, DIN -> 11

// include library, include base class, make path known
#include <GxEPD.h>

// select the display class to use, only one
//#include <GxGDEP015OC1/GxGDEP015OC1.cpp>    // 1.54" b/w
//#include <GxGDE0213B1/GxGDE0213B1.cpp>      // 2.13" b/w
//#include <GxGDEH029A1/GxGDEH029A1.cpp>      // 2.9" b/w
//#include <GxGDEW042T2/GxGDEW042T2.cpp>      // 4.2" b/w
// these displays do not fully support partial update
//#include <GxGDEW0154Z17/GxGDEW0154Z17.cpp>  // 1.54" b/w/r 152x152
//#include <GxGDEW0213Z16/GxGDEW0213Z16.cpp>  // 2.13" b/w/r
//#include <GxGDEW029Z10/GxGDEW029Z10.cpp>    // 2.9" b/w/r
//#include <GxGDEW027C44/GxGDEW027C44.cpp>    // 2.7" b/w/r
//#include <GxGDEW027W3/GxGDEW027W3.cpp>      // 2.7" b/w
//#include <GxGDEW042Z15/GxGDEW042Z15.cpp>    // 4.2" b/w/r
//#include <GxGDEW0583T7/GxGDEW0583T7.cpp>    // 5.83" b/w
//#include <GxGDEW075T8/GxGDEW075T8.cpp>      // 7.5" b/w
//#include <GxGDEW075Z09/GxGDEW075Z09.cpp>    // 7.5" b/w/r

#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
#include <GxIO/GxIO.cpp>

// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMonoBold9pt7b.h>

#include GxEPD_BitmapExamples

#if defined(ESP8266)

// generic/common.h
//static const uint8_t SS    = 15; // D8
//static const uint8_t MOSI  = 13; // D7
//static const uint8_t MISO  = 12; // D6
//static const uint8_t SCK   = 14; // D5

GxIO_Class io(SPI, /*CS=D8*/ SS, /*DC=D3*/ 0, /*RST=D4*/ 2); // arbitrary selection of D3(=0), D4(=2), selected for default of GxEPD_Class
GxEPD_Class display(io /*RST=D4*/ /*BUSY=D2*/); // default selection of D4(=2), D2(=4)

#elif defined(ESP32)

// pins_arduino.h, e.g. LOLIN32
//static const uint8_t SS    = 5;
//static const uint8_t MOSI  = 23;
//static const uint8_t MISO  = 19;
//static const uint8_t SCK   = 18;

GxIO_Class io(SPI, /*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16); // arbitrary selection of 17, 16
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

GxIO_Class io(SPI, /*CS=*/ 4, /*DC=*/ 7, /*RST=*/ 6);
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

GxIO_Class io(SPI, /*CS=*/ SS, /*DC=*/ 3, /*RST=*/ 2);
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

GxIO_Class io(SPI, /*CS=*/ SS, /*DC=*/ PE15, /*RST=*/ PE14); // DC, RST as wired by DESPI-M01
GxEPD_Class display(io, /*RST=*/ PE14, /*BUSY=*/ PE13); // RST, BUSY as wired by DESPI-M01

#else

// pins_arduino.h, e.g. AVR
//#define PIN_SPI_SS    (10)
//#define PIN_SPI_MOSI  (11)
//#define PIN_SPI_MISO  (12)
//#define PIN_SPI_SCK   (13)

GxIO_Class io(SPI, /*CS=*/ SS, /*DC=*/ 8, /*RST=*/ 9); // arbitrary selection of 8, 9 selected for default of GxEPD_Class
GxEPD_Class display(io /*RST=9*/ /*BUSY=7*/); // default selection of (9), 7

#endif


//#define DEMO_DELAY 3*60 // seconds
//#define DEMO_DELAY 1*60 // seconds
#define DEMO_DELAY 30

void setup(void)
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");
  display.init(115200); // enable diagnostic output on Serial
  Serial.println("setup done");
}

void loop()
{
#if defined(__AVR)
  showPartialUpdatePaged();
#elif defined(_GxGDEW075Z09_H_) && (defined(ESP8266) || defined(ARDUINO_ARCH_STM32F1))
  showPartialUpdatePaged();
#elif defined(_GxGDEW075Z09_H_)
  showPartialUpdate_75Z09();
#else
  showPartialUpdate();
#endif
  delay(DEMO_DELAY * 1000);
}

#if !defined(__AVR)

void showPartialUpdate()
{
  // use asymmetric values for test
  uint16_t box_x = 10;
  uint16_t box_y = 15;
  uint16_t box_w = 70;
  uint16_t box_h = 20;
  uint16_t cursor_y = box_y + box_h - 6;
  float value = 13.95;
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setRotation(0);
  // draw background
  display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
  display.update();
  delay(2000);

  // partial update to full screen to preset for partial update of box window
  // (this avoids strange background effects)
  display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);

  // show where the update box is
  for (uint16_t r = 0; r < 4; r++)
  {
    display.setRotation(r);
    display.fillRect(box_x, box_y, box_w, box_h, GxEPD_BLACK);
    display.updateWindow(box_x, box_y, box_w, box_h, true);
    delay(1000);
    display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    display.updateWindow(box_x, box_y, box_w, box_h, true);
  }
  // show updates in the update box
  for (uint16_t r = 0; r < 4; r++)
  {
    // reset the background
    display.setRotation(0);
    display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
    display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);
    display.setRotation(r);
    for (uint16_t i = 1; i <= 10; i++)
    {
      display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
      display.setCursor(box_x, cursor_y);
      display.print(value * i, 2);
      display.updateWindow(box_x, box_y, box_w, box_h, true);
      delay(2000);
    }
    delay(2000);
    display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    display.updateWindow(box_x, box_y, box_w, box_h, true);
  }
  // should have checked this, too
  box_x = GxEPD_HEIGHT - box_x - box_w - 1; // not valid for all corners
  // should show on right side of long side
  // reset the background
  display.setRotation(0);
  display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
  display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);
  // show where the update box is
  for (uint16_t r = 0; r < 4; r++)
  {
    display.setRotation(r);
    display.fillRect(box_x, box_y, box_w, box_h, GxEPD_BLACK);
    display.updateWindow(box_x, box_y, box_w, box_h, true);
    delay(1000);
    display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    display.updateWindow(box_x, box_y, box_w, box_h, true);
  }
  // show updates in the update box
  for (uint16_t r = 0; r < 4; r++)
  {
    // reset the background
    display.setRotation(0);
    display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
    display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);
    display.setRotation(r);
    if (box_x >= display.width()) continue; // avoid delay
    for (uint16_t i = 1; i <= 10; i++)
    {
      display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
      display.setCursor(box_x, cursor_y);
      display.print(value * i, 2);
      display.updateWindow(box_x, box_y, box_w, box_h, true);
      delay(2000);
    }
    delay(2000);
    display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    display.updateWindow(box_x, box_y, box_w, box_h, true);
  }
  display.setRotation(0);
  display.powerDown();
}

#endif

#if defined(_GxGDEW075Z09_H_) && !(defined(ESP8266) || defined(ARDUINO_ARCH_STM32F1))

void showPartialUpdate_75Z09()
{
  // use asymmetric values for test
  uint16_t box_x = 10;
  uint16_t box_y = 15;
  uint16_t box_w = 70;
  uint16_t box_h = 20;
  uint16_t cursor_y = box_y + box_h - 6;
  float value = 13.95;
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setRotation(0);
  // draw background, partial update to full screen to preset for partial update of box window
  // (updateWindow() would clear display if partial update not set, to avoid strange background effect)
  display.drawExamplePicture_3C(BitmapPicture_3C, sizeof(BitmapPicture_3C), GxEPD::bm_partial_update);
  // show where the update box is
  for (uint16_t r = 0; r < 4; r++)
  {
    display.setRotation(r);
    display.fillRect(box_x, box_y, box_w, box_h, GxEPD_BLACK);
    display.updateWindow(box_x, box_y, box_w, box_h, true);
    delay(1000);
    display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    display.updateWindow(box_x, box_y, box_w, box_h, true);
  }
  // show updates in the update box
  for (uint16_t r = 0; r < 4; r++)
  {
    // reset the background
    display.setRotation(0);
    display.drawExamplePicture_3C(BitmapPicture_3C, sizeof(BitmapPicture_3C), GxEPD::bm_partial_update);
    display.setRotation(r);
    for (uint16_t i = 1; i <= 10; i++)
    {
      display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
      display.setCursor(box_x, cursor_y);
      display.print(value * i, 2);
      display.updateWindow(box_x, box_y, box_w, box_h, true);
      delay(2000);
    }
    delay(2000);
    display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    display.updateWindow(box_x, box_y, box_w, box_h, true);
  }
  // should have checked this, too
  box_x = GxEPD_HEIGHT - box_x - box_w - 1; // not valid for all corners
  // should show on right side of long side
  // reset the background
  display.setRotation(0);
  display.drawExamplePicture_3C(BitmapPicture_3C, sizeof(BitmapPicture_3C), GxEPD::bm_partial_update);
  // show where the update box is
  for (uint16_t r = 0; r < 4; r++)
  {
    display.setRotation(r);
    display.fillRect(box_x, box_y, box_w, box_h, GxEPD_BLACK);
    display.updateWindow(box_x, box_y, box_w, box_h, true);
    delay(1000);
    display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    display.updateWindow(box_x, box_y, box_w, box_h, true);
  }
  // show updates in the update box
  for (uint16_t r = 0; r < 4; r++)
  {
    // reset the background
    display.setRotation(0);
    display.drawExamplePicture_3C(BitmapPicture_3C, sizeof(BitmapPicture_3C), GxEPD::bm_partial_update);
    display.setRotation(r);
    if (box_x >= display.width()) continue; // avoid delay
    for (uint16_t i = 1; i <= 10; i++)
    {
      display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
      display.setCursor(box_x, cursor_y);
      display.print(value * i, 2);
      display.updateWindow(box_x, box_y, box_w, box_h, true);
      delay(2000);
    }
    delay(2000);
    display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    display.updateWindow(box_x, box_y, box_w, box_h, true);
  }
  display.setRotation(0);
  display.powerDown();
}

#endif

#if defined(__AVR) || defined(_GxGDEW075Z09_H_) && (defined(ESP8266) || defined(ARDUINO_ARCH_STM32F1))

// modified to avoid float; reduces program size ~2k (for GxGDEW042T2)

void showBlackBoxCallback(uint32_t v)
{
  uint16_t box_x = 10;
  uint16_t box_y = 15;
  uint16_t box_w = 70;
  uint16_t box_h = 20;
  display.fillRect(box_x, box_y, box_w, box_h, v);
}

void showValueBoxCallback(const uint32_t v)
{
  uint16_t box_x = 10;
  uint16_t box_y = 15;
  uint16_t box_w = 70;
  uint16_t box_h = 20;
  uint16_t cursor_y = box_y + box_h - 6;
  display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
  display.setCursor(box_x, cursor_y);
  display.print(v / 100);
  display.print(v % 100 > 9 ? "." : ".0");
  display.print(v % 100);
}

void showPartialUpdatePaged()
{
  uint16_t box_x = 10;
  uint16_t box_y = 15;
  uint16_t box_w = 70;
  uint16_t box_h = 20;
  uint16_t cursor_y = box_y + box_h - 6;
  uint32_t value = 1395;
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setRotation(0);
  // draw background
  display.drawExampleBitmap(BitmapExample1, sizeof(BitmapExample1));
  delay(2000);

  // partial update to full screen to preset for partial update of box window
  // (this avoids strange background effects)
  display.drawExampleBitmap(BitmapExample1, sizeof(BitmapExample1), GxEPD::bm_flip_x | GxEPD::bm_partial_update);
  delay(1000);

  // show where the update box is
  for (uint16_t r = 0; r < 4; r++)
  {
    display.setRotation(r);
    display.drawPagedToWindow(showBlackBoxCallback, box_x, box_y, box_w, box_h, GxEPD_BLACK);
    //display.drawPagedToWindow(showBlackBoxCallback, box_x, box_y, box_w, box_h, GxEPD_BLACK);
    delay(1000);
    display.drawPagedToWindow(showBlackBoxCallback, box_x, box_y, box_w, box_h, GxEPD_WHITE);
  }
  // show updates in the update box
  for (uint16_t r = 0; r < 4; r++)
  {
    display.setRotation(r);
    for (uint16_t i = 1; i <= 10; i++)
    {
      uint32_t v = value * i;
      display.drawPagedToWindow(showValueBoxCallback, box_x, box_y, box_w, box_h, v);
      delay(500);
    }
    delay(1000);
    display.drawPagedToWindow(showBlackBoxCallback, box_x, box_y, box_w, box_h, GxEPD_WHITE);
  }
  display.setRotation(0);
  display.powerDown();
}

#endif

