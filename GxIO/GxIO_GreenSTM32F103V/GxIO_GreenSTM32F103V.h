// Class GxIO_GreenSTM32F103V : io class for parallel interface e-paper displays from Dalian Good Display Inc.
//
// Created by Jean-Marc Zingg based on demo code from Good Display for green DESTM32-L board.
//
// The GxIO_GreenSTM32F103V class is board specific and serves as IO channel for the display class.
//
// These classes can also serve as an example for other boards to use with parallel e-paper displays from Good Display,
// however this is not easy, because of the e-paper specific supply voltages.
//
// To be used with "Generic STM32F103V series" of package "STM32 Boards (STMduino.com)" for Arduino.
// https://github.com/rogerclarkmelbourne/Arduino_STM32
//
// The e-paper display and demo board is available from:
// http://www.buy-lcd.com/index.php?route=product/product&path=2897_8370&product_id=15273
//
// note: "This is a old version 6'' e-paper display, which will not be produced any more if needed quantity is less than 200Kpcs per order."
// The new red DESTM32-L board provides better performance and more resources (1MB FSMC SRAM) for the same price.
//
// Added to my library for reference, completeness and backup.

#ifndef _GxIO_GreenSTM32F103V_H
#define _GxIO_GreenSTM32F103V_H

#include <Arduino.h>

class GxIO_GreenSTM32F103V
{
  public:
    GxIO_GreenSTM32F103V();
    void init(void);
    void delay140ns(uint32_t nCount);
    void powerOn(void);
    void powerOff(void);
    void start_scan(void);
    void send_row(uint8_t row_data[], uint16_t row_size);
};

#endif

