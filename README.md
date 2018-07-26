# GxEPD
A simple E-Paper display library with common base class and separate IO class for Arduino.

## For SPI e-paper displays from Dalian Good Display 
## and SPI e-paper boards from Waveshare

### important note :
### - these displays are for 3.3V supply and 3.3V data lines
### - never connect data lines directly to 5V Arduino data pins, you may use e.g. 4k7 series resistor
### - do not forget to connect GND

### Paged Drawing, Picture Loop for AVR
#### - This library uses paged drawing to cope with RAM restriction and missing single pixel update support
#### - Paged drawing is implemented using callbacks to callback functions in the user application code,
#### - the picture loop is internal to the display classes and calls the callback function as many times as needed,
#### - this is a different implementation compared to the picture loop in U8G2 (Oliver Kraus)
#### - see also https://github.com/olikraus/u8glib/wiki/tpictureloop


### The E-Paper display base class is a subclass of Adafruit_GFX, to have graphics and text rendering.

It needs up to 30kB available RAM to buffer the black/white image for the SPI displays.
ESP8266 or STM32 systems have just enough free RAM, e.g. Arduino Due, ESP8266 or STM32.
I use it with Wemos D1 mini, STM32F103RB-Nucleo, and STMF103C8T6 (BluePill) systems.
- Paged Drawing is available to cope with RAM restriction on AVR processors.

### Supporting Arduino Forum Topics:

- Waveshare e-paper displays with SPI: http://forum.arduino.cc/index.php?topic=487007.0
- Good Dispay ePaper for Arduino : https://forum.arduino.cc/index.php?topic=436411.0

Initial support is for E-Paper displays from Dalian Good Display Inc. with SPI:

These display can be connected using the DESTM32-S2 connection board to power and SPI.
- GDEW042T2 4.2 inch 400 x 300 pixel black/white
- GDEW075T8 7.5 inch 640 x 384 pixel black/white
- Added GDEP015OC1 1.54 inch 200 x 200 pixel black/white
- Added GDEW027C44 2.7 inch 176 x 264 pixel black/white/red
- Added example IoT_SHT31LP_Example_1.54inchEPD
- Added GDE0213B1 2.13 inch 128 x 250 pixel black/white
- Added GDEH029A1 2.9 inch 128 x 296 pixel black/white

The following classes can be used with Waveshare e-Paper displays:

- The GxGDEP015OC1 class can be used with Waveshare 1.54inch e-Paper SPI display.
- The GxGDE0213B1 class can be used with Waveshare 2.13inch e-Paper SPI display.
- The GxGDEH029A1 class can be used with Waveshare 2.9inch e-Paper SPI display.
- The GxGDEW042T2 class can be used with Waveshare 4.2inch e-Paper SPI display.
- The GxGDEW027C44 class may be usable with the Waveshare 264x176 2.7inch 3 color E-Ink display. 

Support for partial update and paged drawing (AVR, low RAM).
- To use on AVR (UNO, NANO) Arduino IDE 1.8.x is required (optimizing linker) for code space.

mapping suggestion from Waveshare SPI e-Paper to Wemos D1 mini:
- BUSY -> D2, RST -> D4, DC -> D3, CS -> D8, CLK -> D5, DIN -> D7, GND -> GND, 3.3V -> 3.3V

mapping suggestion from Waveshare SPI e-Paper to ESP8266 NodeMCU:
- BUSY -> D2, RST -> D4, DC -> D3, CS -> D8, CLK -> D5, DIN -> D7, GND -> GND, 3.3V -> 3.3V

mapping suggestion from Waveshare SPI e-Paper to generic ESP8266:
- BUSY -> GPIO4, RST -> GPIO2, DC -> GPIO0, CS -> GPIO15, CLK -> GPIO14, DIN -> GPIO13, GND -> GND, 3.3V -> 3.3V

mapping suggestion (by G6EJD) from Waveshare SPI e-Paper to ESP8266 Huzzah:
- BUSY -> 03, RST -> 15, DC -> 02, CS -> 00, CLK -> 14, DIN -> 13, GND -> GND, 3.3V -> 3.3V

new mapping suggestion for STM32F1, e.g. STM32F103C8T6 "BluePill"
- BUSY -> A1, RST -> A2, DC -> A3, CS-> A4, CLK -> A5, DIN -> A7

mapping example for AVR, UNO, NANO etc.:
- BUSY -> 7, RST -> 9, DC -> 8, C S-> 10, CLK -> 13, DIN -> 11

Added classes for Waveshare and Good Display black / white / red SPI e-Paper displays:

- GxGDEW0154Z04 1.54 inch 200 x 200 3 color SPI display
- GxGDEW0213Z16 2.13 inch 104 x 212 3 color SPI display
- GxGDEW029Z10  2.9  inch 128 x 296 3 color SPI display

Added Fast Partial Update variant GxGDEW042T2_FPU for 4.2 inch black/white display
- NOTE: This Fast Partial Update variant works with an experimental partial update waveform table
- Side effects and life expectancy with this LUT are unknown, as it is NOT from the manufacturer!

Added Support for multiple e-paper displays on one Arduino (with enough RAM, e.g. ESP32)

Added GxGDEW042Z15 display class for 4.2inch 400 x 300 black / white / red e-Paper
- The display class supports partial update, but display update is full screen (controller issue)

Added GxGDEW027W3 display class for 2.7inch 264 x 176 black / white e-Paper

### Version 2.3.15
- added GxEPD_SD_Example and GxEPD_WiFi_Example
- GxEPD_SD_Example replaces GxEPD_SD_BitmapExample with more BMP depths
- GxEPD_SD_Example does not work on small RAM AVR
- GxEPD_WiFi_Example only for ESP32 and ESP8286
- ESP8266 does not work reliable with big BMP download (known ESP8266 package issue)
#### Version 2.3.14
- added GxEPD_SD_BitmapExample
### Version 2.3.13
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
--------------------------------------------------------------------------------------------

## For HD E-Paper displays from Dalian Good Display Inc. with parallel interface.

- GDE060BA 6 inch 800 x 600 pixel 4 gray level
- GDEW080T5 8 inch 1024 x 768 pixel 4 gray level

These display can be used with the red DESTM32-L evaluation board, it has 1MB FSMC SRAM on board.

The library classes for these display can be used with the STM32GENERIC package for Arduino IDE.

- Added GxGDE043A2 4.3 inch 800 x 600 pixel 4 gray level, with unresolved degradation issue

