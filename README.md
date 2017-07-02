# GxEPD
A simple E-Paper display library with common base class and separate IO class for Arduino.


The E-Paper display base class is a subclass of Adafruit_GFX, to have graphics and text rendering.

It needs roughly 20kB available RAM to buffer the black/white image for the SPI displays.
ESP8266 or STM32 systems have just enough free RAM, e.g. Arduino Due, ESP8266 or STM32.
I use it with Wemos D1 mini, STM32F103RB-Nucleo, and STMF103C8T6 (BluePill) systems.

Initial support is for E-Paper displays from Dalian Good Display Inc. with SPI:

GDEW042T2 4.2 inch 400 x 300 pixel black/white

GDEW075T8 7.5 inch 640 x 384 pixel black/white

Added GDEP015OC1 1.54 inch 200 x 200 pixel black/white (initial version, no partial update)

Added GDEW027C44 2.7 inch 176 x 264 pixel black/white/red

Added example IoT_SHT31LP_Example_1.54inchEPD

Added GDE0213B1 2.13 inch 128 x 250 pixel black/white

These display can be connected using the DESTM32-S2 connection board to power and SPI.

The GxGDEP015OC1 class can also be used with Waveshare 1.54inch e-Paper SPI display.
The GxGDE0213B1  class can also be used with Waveshare 2.13inch e-Paper SPI display.

Added support for HD E-Paper displays from Dalian Good Display Inc. with parallel interface.

GDE060BA 6 inch 800 x 600 pixel 4 gray level

GDEW080T5 8 inch 1024 x 768 pixel 4 gray level

These display can be used with the red DESTM32-L evaluation board, it has 1MB FSMC SRAM on board.
The library classes for these display can be used with the STM32GENERIC package for Arduino IDE.
