// Class GxIO_DESTM32L : io class for parallel interface e-paper displays from Dalian Good Display Inc.
//
// Created by Jean-Marc Zingg based on demo code from Good Display for red DESTM32-L board.
//
// The GxIO_DESTM32L is board specific and serves as IO channel for the display class.
//
// This classes can also serve as an example for other boards to use with parallel e-paper displays from Good Display,
// however this is not easy, because of the e-paper specific supply voltages and big RAM buffer needed.
//
// To be used with "BLACK 407ZE (V3.0)" of "BLACK F407VE/ZE/ZG boards" of package "STM32GENERIC for STM32 boards" for Arduino.
// https://github.com/danieleff/STM32GENERIC
//
// The e-paper display and demo board is available from:
// http://www.buy-lcd.com/index.php?route=product/product&path=2897_10571&product_id=22833
// or https://www.aliexpress.com/store/product/Epaper-demo-kit-for-6-800X600-epaper-display-GDE060BA/600281_32812255729.html

#include "GxIO_DESTM32L.h"
#include "fmsc_sram_init.h"

#define EPD_CL_PORT          GPIOE
#define EPD_CL_BIT          (0x1<<5)    //PE5     CL     WR
#define EPD_CL_PIN          PE5
#define EPD_OE_PORT         GPIOE
#define EPD_OE_BIT          (0x1<<4)    //PE4     OE     OE
#define EPD_OE_PIN          PE4
#define EPD_LE_PORT         GPIOE
#define EPD_LE_BIT          (0x1<<6)    //PE6     LE     LE
#define EPD_LE_PIN          PE6
#define EPD_SHR_PORT        GPIOB
#define EPD_SHR_BIT         (0x1<<7)    //PB7     SHR  
#define EPD_SHR_PIN         PB7
#define EPD_GMODE1_PORT     GPIOF
#define EPD_GMODE1_BIT      (0x1<<7)    //PF7       MODE1
#define EPD_GMODE1_PIN      PF7
#define EPD_GMODE2_PORT     GPIOF
#define EPD_GMODE2_BIT      (0x1<<10)   //PF10        MODE2
#define EPD_GMODE2_PIN      PF10
#define EPD_XRL_PORT        GPIOF
#define EPD_XRL_BIT         (0x1<<9)    //PF9       L/R
#define EPD_XRL_PIN         PF9

#define EPD_DB_PORT         GPIOC

#define EPD_XSPV_PORT       GPIOF
#define EPD_XSPV_BIT        (0x1<<6)    //PF6   SPV
#define EPD_XSPV_PIN        PF6
#define EPD_CLK_PORT        GPIOF
#define EPD_CLK_BIT         (0x1<<8)    //PF8   CPV
#define EPD_CLK_PIN         PF8
#define EPD_SPH_PORT        GPIOE
#define EPD_SPH_BIT         (0x1<<3)    //PE3   SPH
#define EPD_SPH_PIN         PE3

#define VCOM_CTR_PORT       GPIOG
#define VCOM_CTR_BIT        (0x1<<12)   //PG12      4053 S2
#define VCOM_CTR_PIN        PG12
#define ASW1_CTR_PORT       GPIOG
#define ASW1_CTR_BIT        (0x1<<11)   //PG11      4053  S1
#define ASW1_CTR_PIN        PG11
#define ASW3_CTR_PORT       GPIOG
#define ASW3_CTR_BIT        (0x1<<13)   //PG13      4053  S3
#define ASW3_CTR_PIN        PG13

#define VNEGGVEE_CTR_PORT   GPIOB
#define VNEGGVEE_CTR_BIT    (0x1<<9)    //PB9     AME5142
#define VNEGGVEE_CTR_PIN    PB9
#define VPOS15_CTR_PORT     GPIOG
#define VPOS15_CTR_BIT      (0x1<<14)   //PG14    +15V
#define VPOS15_CTR_PIN      PG14
#define GVDD22_CTR_PORT     GPIOB
#define GVDD22_CTR_BIT      (0x1<<8)    //PB8     +22V
#define GVDD22_CTR_PIN      PB8

#define EPD_CL_H            EPD_CL_PORT->BSRR = EPD_CL_BIT;     delay35ns(1);
#define EPD_CL_L            EPD_CL_PORT->BSRR = EPD_CL_BIT<<16; delay35ns(1);

#define EPD_LE_H            EPD_LE_PORT->BSRR = EPD_LE_BIT
#define EPD_LE_L            EPD_LE_PORT->BSRR = EPD_LE_BIT<<16

#define EPD_OE_H            EPD_OE_PORT->BSRR = EPD_OE_BIT
#define EPD_OE_L            EPD_OE_PORT->BSRR = EPD_OE_BIT<<16

#define EPD_SPH_H           EPD_SPH_PORT->BSRR = EPD_SPH_BIT
#define EPD_SPH_L           EPD_SPH_PORT->BSRR = EPD_SPH_BIT<<16

#define EPD_XSPV_H          EPD_XSPV_PORT->BSRR = EPD_XSPV_BIT
#define EPD_XSPV_L          EPD_XSPV_PORT->BSRR = EPD_XSPV_BIT<<16

#define EPD_CLK_H           EPD_CLK_PORT->BSRR = EPD_CLK_BIT
#define EPD_CLK_L           EPD_CLK_PORT->BSRR = EPD_CLK_BIT<<16

GxIO_DESTM32L::GxIO_DESTM32L()
{
  _pwr_led = PB12;
}

void GxIO_DESTM32L::init(uint8_t power_on_led)
{
  if ((power_on_led >= PB12) && (power_on_led <= PB15)) _pwr_led = power_on_led;
  pinMode(_pwr_led, OUTPUT);
  digitalWrite(_pwr_led, LOW);
  // FSMC SRAM
  fmsc_sram_init();
  //PORT_B_PINS
  pinMode(GVDD22_CTR_PIN, OUTPUT);
  pinMode(VNEGGVEE_CTR_PIN, OUTPUT);
  pinMode(EPD_SHR_PIN, OUTPUT);
  //PORT_C_PINS
  for (uint8_t p = PC0; p <= PC7; p++)
  {
    pinMode(p, OUTPUT);
  }
  //PORT_E_PINS
  pinMode(EPD_CL_PIN, OUTPUT);
  pinMode(EPD_OE_PIN, OUTPUT);
  pinMode(EPD_LE_PIN, OUTPUT);
  pinMode(EPD_SPH_PIN, OUTPUT);
  //PORT_F_PINS
  pinMode(EPD_GMODE1_PIN, OUTPUT);
  pinMode(EPD_GMODE2_PIN, OUTPUT);
  pinMode(EPD_XRL_PIN, OUTPUT);
  pinMode(EPD_XSPV_PIN, OUTPUT);
  pinMode(EPD_CLK_PIN, OUTPUT);
  //PORT_G_PINS
  pinMode(VCOM_CTR_PIN, OUTPUT);
  pinMode(ASW1_CTR_PIN, OUTPUT);
  pinMode(ASW3_CTR_PIN, OUTPUT);
  pinMode(VPOS15_CTR_PIN, OUTPUT);
  powerOff();
  //EPD_Init(void)
  digitalWrite(EPD_SHR_PIN, LOW);     //Shift direction source driver
  digitalWrite(EPD_GMODE1_PIN, HIGH); //one pulse mode
  digitalWrite(EPD_GMODE2_PIN, HIGH); //one pulse mode
  digitalWrite(EPD_XRL_PIN, HIGH);    //Shift direction gate driver
  powerOff();
  EPD_LE_L;
  EPD_CL_L;
  EPD_OE_L;
  EPD_SPH_H;
  EPD_XSPV_H;
  EPD_CLK_L;
}

void GxIO_DESTM32L::powerOn(void)
{
  digitalWrite(_pwr_led, LOW);
  digitalWrite(VNEGGVEE_CTR_PIN, HIGH);
  delay35ns(0xf);
  digitalWrite(VPOS15_CTR_PIN, HIGH);
  delay35ns(0xf);
  digitalWrite(GVDD22_CTR_PIN, HIGH);
  delay35ns(0xf);
  digitalWrite(VCOM_CTR_PIN, HIGH);
  delay35ns(0xff);
}

void GxIO_DESTM32L::powerOff(void)
{
  digitalWrite(VCOM_CTR_PIN, LOW);
  delay35ns(0x2ff);
  digitalWrite(GVDD22_CTR_PIN, LOW);
  delay35ns(0x2f);
  digitalWrite(VPOS15_CTR_PIN, LOW);
  delay35ns(0x2f);
  digitalWrite(VNEGGVEE_CTR_PIN, LOW);
  delay35ns(0x2ff);
  digitalWrite(_pwr_led, HIGH);
}

void GxIO_DESTM32L::start_scan(void)
{
  EPD_XSPV_H;

  uint16_t repeat = 2;
  while (repeat--)
  {
    EPD_CLK_L;
    delay35ns(0xf);
    EPD_CLK_H;
    delay35ns(0xf);
  }

  EPD_XSPV_L;

  repeat = 2;
  while (repeat--)
  {
    EPD_CLK_L;
    delay35ns(0xf);
    EPD_CLK_H;
    delay35ns(0xf);
  }

  EPD_XSPV_H;

  repeat = 2;
  while (repeat--)
  {
    EPD_CLK_L;
    delay35ns(0xf);
    EPD_CLK_H;
    delay35ns(0xf);
  }
}

void GxIO_DESTM32L::send_row(uint8_t row_data[], uint16_t row_size, uint32_t delay_time)
{
  EPD_LE_H;
  EPD_CL_L;
  EPD_CL_H;
  EPD_CL_L;
  EPD_CL_H;

  EPD_LE_L;
  EPD_CL_L;
  EPD_CL_H;
  EPD_CL_L;
  EPD_CL_H;

  EPD_OE_H;
  EPD_CL_L;
  EPD_CL_H;
  EPD_CL_L;
  EPD_CL_H;
  EPD_SPH_L;

  for (uint32_t column = 0; column < row_size; column++)
  {
    EPD_DB_PORT->BSRR = 0xFF << 16; // reset data bits
    EPD_DB_PORT->BSRR = (uint16_t)row_data[column]; // set data bits
    EPD_CL_L;
    delay35ns(delay_time);
    EPD_CL_H;
    delay35ns(delay_time);
  }

  EPD_SPH_H;
  EPD_CL_L;
  EPD_CL_H;
  EPD_CL_L;
  EPD_CL_H;

  EPD_CLK_L;
  EPD_OE_L;
  EPD_CL_L;
  EPD_CL_H;
  EPD_CL_L;
  EPD_CL_H;
  delay35ns(delay_time);

  EPD_CLK_H;
  delay35ns(delay_time);
}

static uint32_t t_delay;

void GxIO_DESTM32L::delay35ns(uint32_t nCount)
{
  for (; nCount != 0; nCount--)
  {
    t_delay += nCount;
  }
}


