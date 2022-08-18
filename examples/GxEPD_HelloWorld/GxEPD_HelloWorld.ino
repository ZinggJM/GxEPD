// GxEPD_Hello World Example by Jean-Marc Zingg

//
// Created by Jean-Marc Zingg based on demo code from Good Display,
// available on http://www.e-paper-display.com/download_list/downloadcategoryid=34&isMode=false.html
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

// mapping suggestion for Arduino MEGA
// BUSY -> 7, RST -> 9, DC -> 8, CS-> 53, CLK -> 52, DIN -> 51

// mapping suggestion for Arduino DUE
// BUSY -> 7, RST -> 9, DC -> 8, CS-> 77, CLK -> 76, DIN -> 75
// SPI pins are also on 6 pin 2x3 SPI header

// include library, include base class, make path known
#include <GxEPD.h>

// select the display class to use, only one
//#include <GxDEPG0150BN/GxDEPG0150BN.h>    // 1.50" b/w
//#include <GxGDEP015OC1/GxGDEP015OC1.h>    // 1.54" b/w
//#include <GxGDEH0154D67/GxGDEH0154D67.h>  // 1.54" b/w 200x200, SSD1681
//#include <GxGDEW0154T8/GxGDEW0154T8.h>    // 1.54" b/w 152x152 UC8151 (IL0373)
//#include <GxGDEW0154M09/GxGDEW0154M09.h>  // 1.54" b/w 200x200 JD79653A
//#include <GxGDEW0154M10/GxGDEW0154M10.h>  // 1.54" b/w 152x152 UC8151D
//#include <GxGDEW0154Z04/GxGDEW0154Z04.h>  // 1.54" b/w/r 200x200
//#include <GxGDEW0154Z17/GxGDEW0154Z17.h>  // 1.54" b/w/r 152x152
//#include <GxGDEH0154Z90/GxGDEH0154Z90.h>  // 1.54" b/w/r 200x200 SSD1681
//#include <GxGDEW0213I5F/GxGDEW0213I5F.h>  // 2.13" b/w 104x212 flexible
//#include <GxGDE0213B1/GxGDE0213B1.h>      // 2.13" b/w
//#include <GxGDEH0213B72/GxGDEH0213B72.h>  // 2.13" b/w new panel
//#include <GxGDEH0213B73/GxGDEH0213B73.h>  // 2.13" b/w newer panel
//#include <GxGDEM0213B74/GxGDEM0213B74.h>  // 2.13" b/w 128x250 SSD1680
//#include <GxGDEW0213Z16/GxGDEW0213Z16.h>  // 2.13" b/w/r
//#include <GxGDEH0213Z19/GxGDEH0213Z19.h>  // 2.13" b/w/r UC8151D
//#include <GxGDEW0213T5D/GxGDEW0213T5D.h>  // 2.13" b/w 104x212 UC8151D
//#include <GxDEPG0213BN/GxDEPG0213BN.h>    // 2.13" b/w 128x250, SSD1680, TTGO T5 V2.4.1, V2.3.1
//#include <GxGDEH029A1/GxGDEH029A1.h>      // 2.9" b/w
//#include <GxGDEW029T5/GxGDEW029T5.h>      // 2.9" b/w UC8151 (IL0373)
//#include <GxGDEW029T5D/GxGDEW029T5D.h>    // 2.9" b/w UC8151D
//#include <GxGDEM029T94/GxGDEM029T94.h>    // 2.9" b/w
//#include <GxDEPG0290BS/GxDEPG0290BS.h>    // 2.9" b/w Waveshare variant, TTGO T5 V2.4.1 2.9"
//#include <GxGDEW029Z10/GxGDEW029Z10.h>    // 2.9" b/w/r
//#include <GxGDEH029Z13/GxGDEH029Z13.h>    // 2.9" b/w/r UC8151D
//#include <GxGDEW026T0/GxGDEW026T0.h>      // 2.6" b/w
//#include <GxDEPG0266BN/GxDEPG0266BN.h>      // 2.66" b/w 152x296, SSD1680, TTGO T5 V2.66, TTGO T5 V2.4.1
//#include <GxGDEW027C44/GxGDEW027C44.h>    // 2.7" b/w/r
//#include <GxGDEW027W3/GxGDEW027W3.h>      // 2.7" b/w
//#include <GxGDEY027T91/GxGDEY027T91.h>    // 2.7" b/w
//#include <GxGDEW0371W7/GxGDEW0371W7.h>    // 3.7" b/w
//#include <GxGDEW042T2/GxGDEW042T2.h>      // 4.2" b/w
//#include <GxGDEW042Z15/GxGDEW042Z15.h>    // 4.2" b/w/r
//#include <GxGDEW0583T7/GxGDEW0583T7.h>    // 5.83" b/w
//#include <GxGDEW075T8/GxGDEW075T8.h>      // 7.5" b/w
//#include <GxGDEW075T7/GxGDEW075T7.h>      // 7.5" b/w 800x480
//#include <GxGDEW075Z09/GxGDEW075Z09.h>    // 7.5" b/w/r
//#include <GxGDEW075Z08/GxGDEW075Z08.h>    // 7.5" b/w/r 800x480

#include GxEPD_BitmapExamples

// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>


#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

#if defined(ESP8266)

// for SPI pin definitions see e.g.:
// C:\Users\xxx\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\2.4.2\variants\generic\common.h

GxIO_Class io(SPI, /*CS=D8*/ SS, /*DC=D3*/ 0, /*RST=D4*/ 2); // arbitrary selection of D3(=0), D4(=2), selected for default of GxEPD_Class
GxEPD_Class display(io, /*RST=D4*/ 2, /*BUSY=D2*/ 4); // default selection of D4(=2), D2(=4)
// Heltec E-Paper 1.54" b/w without RST, BUSY
//GxEPD_Class display(io, /*RST=D4*/ -1, /*BUSY=D2*/ -1); // no RST, no BUSY
// Waveshare e-Paper ESP8266 Driver Board
//GxIO_Class io(SPI, 15, 4, 5);
//GxEPD_Class display(io, 5, 16);

#elif defined(ESP32)

// for SPI pin definitions see e.g.:
// C:\Users\xxx\Documents\Arduino\hardware\espressif\esp32\variants\lolin32\pins_arduino.h

GxIO_Class io(SPI, /*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16); // arbitrary selection of 17, 16
GxEPD_Class display(io, /*RST=*/ 16, /*BUSY=*/ 4); // arbitrary selection of (16), 4
// for LILYGO® TTGO T5 2.66 board uncomment next two lines instead of previous two lines
//GxIO_Class io(SPI, /*CS=5*/ SS, /*DC=*/ 19, /*RST=*/ 4); // LILYGO® TTGO T5 2.66
//GxEPD_Class display(io, /*RST=*/ 4, /*BUSY=*/ 34); // LILYGO® TTGO T5 2.66

#elif defined(ARDUINO_ARCH_SAMD)

// for SPI pin definitions see e.g.:
// C:\Users\xxx\AppData\Local\Arduino15\packages\arduino\hardware\samd\1.6.19\variants\mkr1000\variant.h
// C:\Users\xxx\AppData\Local\Arduino15\packages\arduino\hardware\samd\1.6.19\variants\mkrzero\variant.h

GxIO_Class io(SPI, /*CS=*/ 4, /*DC=*/ 7, /*RST=*/ 6);
GxEPD_Class display(io, /*RST=*/ 6, /*BUSY=*/ 5);

#elif defined(ARDUINO_GENERIC_STM32F103C) && defined(MCU_STM32F103C8)

// STM32 Boards(STM32duino.com) Generic STM32F103C series STM32F103C8
// for SPI pin definitions see e.g.:
// C:\Users\xxx\Documents\Arduino\hardware\Arduino_STM32\STM32F1\variants\generic_stm32f103c\variant.h
// C:\Users\xxx\Documents\Arduino\hardware\Arduino_STM32\STM32F1\variants\generic_stm32f103c\board\board.h

// new mapping suggestion for STM32F1, e.g. STM32F103C8T6 "BluePill"
// BUSY -> A1, RST -> A2, DC -> A3, CS-> A4, CLK -> A5, DIN -> A7

GxIO_Class io(SPI, /*CS=*/ SS, /*DC=*/ 3, /*RST=*/ 2);
GxEPD_Class display(io, /*RST=*/ 2, /*BUSY=*/ 1);

#elif defined(ARDUINO_GENERIC_STM32F103V) && defined(MCU_STM32F103VB)

// STM32 Boards(STM32duino.com) Generic STM32F103V series STM32F103VB
// for SPI pin definitions see e.g.:
// C:\Users\xxx\Documents\Arduino\hardware\Arduino_STM32\STM32F1\variants\generic_stm32f103vb\variant.h
// C:\Users\xxx\Documents\Arduino\hardware\Arduino_STM32\STM32F1\variants\generic_stm32f103vb\board\board.h

// Good Display DESPI-M01
// note: needs jumper wires from SS=PA4->CS, SCK=PA5->SCK, MOSI=PA7->SDI

GxIO_Class io(SPI, /*CS=*/ SS, /*DC=*/ PE15, /*RST=*/ PE14); // DC, RST as wired by DESPI-M01
GxEPD_Class display(io, /*RST=*/ PE14, /*BUSY=*/ PE13); // RST, BUSY as wired by DESPI-M01

#elif defined(ARDUINO_AVR_MEGA2560)

// for SPI pin definitions see e.g.:
// C:\Users\xxx\AppData\Local\Arduino15\packages\arduino\hardware\avr\1.6.21\variants\mega\pins_arduino.h

// select one, depending on your CS connection
GxIO_Class io(SPI, /*CS=*/ SS, /*DC=*/ 8, /*RST=*/ 9); // arbitrary selection of 8, 9 selected for default of GxEPD_Class
//GxIO_Class io(SPI, /*CS=*/ 10, /*DC=*/ 8, /*RST=*/ 9); // arbitrary selection of 8, 9, CS on 10 (for CS same as on UNO, for SPI on ICSP use)

GxEPD_Class display(io, /*RST=*/ 9, /*BUSY=*/ 7); // default selection of (9), 7

#else

// for SPI pin definitions see e.g.:
// C:\Users\xxx\AppData\Local\Arduino15\packages\arduino\hardware\avr\1.6.21\variants\standard\pins_arduino.h

GxIO_Class io(SPI, /*CS=*/ SS, /*DC=*/ 8, /*RST=*/ 9); // arbitrary selection of 8, 9 selected for default of GxEPD_Class
GxEPD_Class display(io, /*RST=*/ 9, /*BUSY=*/ 7); // default selection of (9), 7

#endif


void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");

  display.init(115200); // enable diagnostic output on Serial

#if defined(__AVR) || false
  display.drawPaged(drawHelloWorld);
#elif (defined(_GxGDEW075Z09_H_) || defined(_GxGDEW075Z08_H_)) && (defined(ESP8266) || defined(ARDUINO_ARCH_STM32F1))
  display.drawPaged(drawHelloWorld);
#elif defined(_GxGDEW075Z09_H_)
  display.drawPaged(drawHelloWorld);
#else
  drawHelloWorld();
  display.update();
#endif
  display.powerDown();

  Serial.println("setup done");
}

void loop() {};

const char HelloWorld[] = "Hello World!";

void drawHelloWorld()
{
  //Serial.println("drawHelloWorld");
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);
  // center bounding box by transposition of origin:
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  display.fillScreen(GxEPD_WHITE);
  display.setCursor(x, y);
  display.print(HelloWorld);
  //Serial.println("drawHelloWorld done");
}

void drawHelloWorldForDummies()
{
  // This example function/method can be used with full buffered graphics AND/OR paged drawing graphics
  // for paged drawing it is to be used as callback function
  // it will be executed once or multiple times, as many as needed,
  // in case of full buffer it can be called directly, or as callback
  // IMPORTANT: each iteration needs to draw the same, to avoid strange effects
  // use a copy of values that might change, don't read e.g. from analog or pins in the loop!
  //Serial.println("drawHelloWorldForDummies");
  const char text[] = "Hello World!";
  // most e-papers have width < height (portrait) as native orientation, especially the small ones
  // in GxEPD rotation 0 is used for native orientation (most TFT libraries use 0 fix for portrait orientation)
  // set rotation to 1 (rotate right 90 degrees) to have enough space on small displays (landscape)
  display.setRotation(1);
  // select a suitable font in Adafruit_GFX
  display.setFont(&FreeMonoBold9pt7b);
  // on e-papers black on white is more pleasant to read
  display.setTextColor(GxEPD_BLACK);
  // Adafruit_GFX has a handy method getTextBounds() to determine the boundary box for a text for the actual font
  int16_t tbx, tby; uint16_t tbw, tbh; // boundary box window
  display.getTextBounds(text, 0, 0, &tbx, &tby, &tbw, &tbh); // it works for origin 0, 0, fortunately (negative tby!)
  // center bounding box by transposition of origin:
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  display.fillScreen(GxEPD_WHITE); // set the background to white (fill the buffer with value for white)
  display.setCursor(x, y); // set the postition to start printing text
  display.print(text); // print some text
  //Serial.println("drawHelloWorldForDummies done");
}
