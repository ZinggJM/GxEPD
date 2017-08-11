// GxEPD_HD_TestExample : test example for HD e-Paper displays from Dalian Good Display Inc. (parallel interface).
//
// Created by Jean-Marc Zingg based on demo code from Good Display for red DESTM32-L board.
//
// To be used with "BLACK 407ZE (V3.0)" of "BLACK F407VE/ZE/ZG boards" of package "STM32GENERIC for STM32 boards" for Arduino.
// https://github.com/danieleff/STM32GENERIC
//
// The e-paper displays and demo board are available from:
//
// http://www.buy-lcd.com/index.php?route=product/product&path=2897_10571&product_id=22833
// or https://www.aliexpress.com/store/product/Epaper-demo-kit-for-6-800X600-epaper-display-GDE060BA/600281_32812255729.html
//
// http://www.buy-lcd.com/index.php?route=product/product&path=2897_8371&product_id=15441
// or https://www.aliexpress.com/store/product/GDE080A1-with-demo-8-epaper-display-panel-1024X768-with-demo/600281_32810750410.html

// include library, include base class, make path known
#include <GxEPD.h>

// select the display class to use, only one
//#include "GxGDE043A2/GxGDE043A2.cpp"
//#include "GxGDE060BA/GxGDE060BA.cpp"
#include "GxGDEW080T5/GxGDEW080T5.cpp"

// uncomment next line for drawBitmap() test
#include GxEPD_BitmapExamples

// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>

#include "GxIO/GxIO_DESTM32L/GxIO_DESTM32L.cpp"

GxIO_DESTM32L io;

GxEPD_Class display(io);

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
  showFont("FreeMonoBold9pt7b", &FreeMonoBold9pt7b);
  showFont("FreeMonoBold12pt7b", &FreeMonoBold12pt7b);
  showFont("FreeMonoBold18pt7b", &FreeMonoBold18pt7b);
  showFont("FreeMonoBold24pt7b", &FreeMonoBold24pt7b);
  delay(30000);
}

void showBitmapExample()
{
#ifdef _GxBitmapExamples_H_
  display.drawPicture(BitmapExample1, sizeof(BitmapExample1));
  delay(5000);
  display.erasePicture(BitmapExample1, sizeof(BitmapExample1));
  display.drawPicture(BitmapExample2, sizeof(BitmapExample2));
  delay(5000);
  display.erasePicture(BitmapExample2, sizeof(BitmapExample2));
  //display.setRotation(2);
  display.fillScreen(GxEPD_WHITE);
  display.drawBitmap(bwBitmap640x384_1, (GxEPD_WIDTH - 640) / 2, (GxEPD_HEIGHT - 384) / 2, 640, 384, GxEPD_BLACK);
  display.update();
  delay(5000);
  display.fillScreen(GxEPD_WHITE);
  display.drawBitmap(bwBitmap640x384_2, (GxEPD_WIDTH - 640) / 2, (GxEPD_HEIGHT - 384) / 2, 640, 384, GxEPD_BLACK, GxEPD::bm_flip_h);
  display.update();
  delay(10000);
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
  display.println("`abcdefghijklmno");
  display.println("pqrstuvwxyz{|}~ ");
  display.update();
  delay(10000);
}




