# GxEPD
A simple E-Paper display library with common base class and separate IO class for Arduino.


The E-Paper display base class is a subclass of Adafruit_GFX, to have graphics and text rendering.

It needs roughly 20kB available RAM to buffer the black/white image for the SPI displays.
ESP8266 or STM32 systems have just enough free RAM, e.g. Arduino Due, ESP8266 or STM32.
I use it with Wemos D1 mini, STM32F103RB-Nucleo, and STMF103C8T6 (BluePill) systems.

Supporting Arduino Forum Topics:

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

mapping from Waveshare 2.9inch e-Paper to Wemos D1 mini:
- BUSY -> D2, RST -> D4, DC -> D3, CS -> D8, CLK -> D5, DIN -> D7, GND -> GND, 3.3V -> 3.3V

mapping example for AVR, UNO, NANO etc.:
- BUSY -> 7, RST -> 9, DC -> 8, C S-> 10, CLK -> 13, DIN -> 11

Added classes for Waveshare and Good Display black / white / red SPI e-Paper displays:

- GxGDEW0154Z04 1.54 inch 200 x 200 3 color SPI display
- GxGDEW0213Z16 2.13 inch 104 x 212 3 color SPI display
- GxGDEW029Z10  2.9  inch 128 x 296 3 color SPI display

Note for use with package ESP8266 or ESP32:
- some ESP libraries #undef min and max, #include these libraries AFTER the GxEPD display class!

Added Fast Partial Update variant GxGDEW042T2_FPU for 4.2 inch black/white display
- NOTE: This Fast Partial Update variant works with an experimental partial update waveform table
- Side effects and life expectancy with this LUT are unknown, as it is NOT from the manufacturer!

Added Support for multiple e-paper displays on one Arduino (with enough RAM, e.g. ESP32)

--------------------------------------------------------------------------------------------

Added support for HD E-Paper displays from Dalian Good Display Inc. with parallel interface.

- GDE060BA 6 inch 800 x 600 pixel 4 gray level
- GDEW080T5 8 inch 1024 x 768 pixel 4 gray level

These display can be used with the red DESTM32-L evaluation board, it has 1MB FSMC SRAM on board.

The library classes for these display can be used with the STM32GENERIC package for Arduino IDE.

- Added GxGDE043A2 4.3 inch 800 x 600 pixel 4 gray level, with unresolved degradation issue

