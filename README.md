# GxEPD
A simple E-Paper display library with common base class and separate IO class for Arduino.

## For SPI e-paper displays from Dalian Good Display 
## and SPI e-paper boards from Waveshare

### important note :
### - these displays are for 3.3V supply and 3.3V data lines
### - never connect data lines directly to 5V Arduino data pins, you may use e.g. 4k7 series resistor
### - do not forget to connect GND

### Paged Drawing, Picture Loop for AVR
- This library uses paged drawing to cope with RAM restriction and missing single pixel update support
- Paged drawing is implemented using callbacks to callback functions in the user application code,
- the picture loop is internal to the display classes and calls the callback function as many times as needed,
- this is a different implementation compared to the picture loop in U8G2 (Oliver Kraus)
- see also https://github.com/olikraus/u8glib/wiki/tpictureloop

### The E-Paper display base class is a subclass of Adafruit_GFX, to have graphics and text rendering.

- It needs up to 30kB available RAM to buffer the black/white image for the SPI displays, double for 3-color.
- ESP8266, ESP32 , STM32 systems,  Arduino Due e.g.  have enough free RAM for full graphics buffer.
- Paged Drawing is available to cope with RAM restriction on AVR processors.

### Supporting Arduino Forum Topics:

- Waveshare e-paper displays with SPI: http://forum.arduino.cc/index.php?topic=487007.0
- Good Dispay ePaper for Arduino : https://forum.arduino.cc/index.php?topic=436411.0

### Supported SPI e-paper panels from Good Display:
- GDEP015OC1      1.54" b/w
- GDEW0154Z04   1.54" b/w/r 200x200
- GDEW0154Z17   1.54" b/w/r 152x152
- GDE0213B1         2.13" b/w
- GDEW0213Z16   2.13" b/w/r
- GDEH029A1        2.9" b/w
- GDEW029Z10     2.9" b/w/r
- GDEW027C44     2.7" b/w/r
- GDEW027W3      2.7" b/w
- GDEW042T2        4.2" b/w
- GDEW042Z15      4.2" b/w/r
- GDEW0583T7      5.83" b/w
- GDEW075T8        7.5" b/w
- GDEW075Z09      7.5" b/w/r
#### Supported SPI e-paper panels & boards from Waveshare: compare with Good Display, same panel

### I can and will only support e-paper panels I have!
- promotion panels from suppliers are welcome, to add support to GxEPD
- donation panels from users are welcome, to add support to GxEPD

### for pin mapping suggestions see ConnectingHardware.md


### Version 3.0.3
- fix GxIO_SPI: avoid nesting SPI transactions causing deadlock
#### Version 3.0.2
- added GxGDEW0213I5F for 2.13" b/w 104x212 flexible display, initial tests successful
- updated GxIO_SPI to use SPI transactions, for SPI speed with multiple devices, NUCLEO-64
- updated GxEPD_WiFi_Example to use BearSSL on ESP8266, for large bitmap downloads
#### Version 3.0.1
- update comments in GxFont_GFX_Example for UTF-8 use, e.g. Umlauts ÄÖÜäéöü
- NOTE: you need to SAVE the modified example to a saveable location for UTF-8 characters to work
#### Version 3.0.0
- new src directory structure avoids the need for .cpp includes
- only header files includes are needed
- Arduino IDE 1.8.x automatically includes needed code from .cpp files
- DO NOT include .cpp files with this version, linker would complain
- src directory structure makes GxEPD a more usual Arduino Library
#### Version 2.3.17
- removed GxGDEW0213I5F for 2.13" b/w 104x212 flexible display
- did not work, as started from 3-c source, controller is IL0373
#### Version 2.3.16
- added GxGDEW0213I5F for 2.13" b/w 104x212 flexible display
- NOT tested on GxGDEW0213I5F, I don't have this display
- tested on 2.9" 3-color display, has same controller IL0373
#### Version 2.3.15
- added GxEPD_SD_Example and GxEPD_WiFi_Example
- GxEPD_SD_Example replaces GxEPD_SD_BitmapExample with more BMP depths
- GxEPD_SD_Example does not work on small RAM AVR
- GxEPD_WiFi_Example only for ESP32 and ESP8286
- ESP8266 does not work reliable with big BMP download (known ESP8266 package issue)
#### Version 2.3.14
- added GxEPD_SD_BitmapExample
#### Version 2.3.13
- added "no BUSY" support to GxGDEP015OC1 for Heltec E-Paper 1.54" b/w without BUSY
- set BUSY parameter to -1 for Heltec E-Paper 1.54" b/w without BUSY
#### Version 2.3.12
- added GxGDEW0583T7 for 5.83" b/w 600x448 display
- my GDEW0583T7 panel has a refresh time of ~15 seconds
- please report if you got this display with faster refresh time, or got faster driver or demo
#### Version 2.3.11
- fix GxGDEW042T2 to avoid double full refresh after reset (deep sleep wakeup)
#### Version 2.3.10
- added GxGDEW0154Z17 for 1.54" 3-color 152x152 display
- NOT tested on GDEW0154Z17, I don't have this display
- tested on 2.9" 3-color display, same controller IL0373
- use GxEPD_RED to get yellow on yellow 3-color e-paper
#### Version 2.3.9
- new version for 7.5" 3-color display GxGDEW075Z09
- GxGDEW075Z09 runs with full buffer on ESP32, Arduino Due, STM32F4
- runs with reduced buffer on ESP8266, STM32F1, AVR : will show buffer content as stripes
- supports paged display on AVR, ESP8266, STM32F1
#### Version 2.3.8
- Serial Diagnostic Output selectable by parameter of init() call:
- void init(uint32_t serial_diag_bitrate = 0); // = 0 : disabled
#### Version 2.3.7
- additional font support, e.g. from https://github.com/olikraus/U8g2_for_Adafruit_GFX
#### Version 2.3.6
- fixes and cleanup
#### Version 2.3.5
- GxFont_GFX : Font Rendering Graphics Switch and Bridge Class

