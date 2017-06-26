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

#include "GxIO_GreenSTM32F103V.h"

#define GPIO_Pin_0 (0x1<<0)
#define GPIO_Pin_1 (0x1<<1)
#define GPIO_Pin_2 (0x1<<2)
#define GPIO_Pin_3 (0x1<<3)
#define GPIO_Pin_4 (0x1<<4)
#define GPIO_Pin_5 (0x1<<5)
#define GPIO_Pin_6 (0x1<<6)
#define GPIO_Pin_7 (0x1<<7)
#define GPIO_Pin_8 (0x1<<8)
#define GPIO_Pin_9 (0x1<<9)
#define GPIO_Pin_10 (0x1<<10)
#define GPIO_Pin_11 (0x1<<11)
#define GPIO_Pin_12 (0x1<<12)
#define GPIO_Pin_13 (0x1<<13)
#define GPIO_Pin_14 (0x1<<14)
#define GPIO_Pin_15 (0x1<<15)
#define GPIOA GPIOA_BASE
#define GPIOB GPIOB_BASE
#define GPIOC GPIOC_BASE

#define EPD_XCE1_PORT       GPIOA
#define EPD_XCE1            GPIO_Pin_8    //PA8     U1CE1  p31
#define EPD_XCE1_PIN        PA8

#define EPD_CL_PORT         GPIOB
#define EPD_CL              GPIO_Pin_8    //PB8       CL    P5    WR
#define EPD_CL_PIN          PB8
#define EPD_OE_PORT         GPIOB
#define EPD_OE              GPIO_Pin_9    //PB9     OE    P7    OE
#define EPD_OE_PIN          PB9
#define EPD_LE_PORT         GPIOB
#define EPD_LE              GPIO_Pin_10   //PB10    LE    P6    LE
#define EPD_LE_PIN          PB10
#define EPD_SHR_PORT        GPIOB
#define EPD_SHR             GPIO_Pin_11   //PB11    SHR   P8
#define EPD_SHR_PIN         PB11
#define EPD_GMODE1_PORT     GPIOB
#define EPD_GMODE1          GPIO_Pin_12   //PB12    MODE1
#define EPD_GMODE1_PIN      PB12
#define EPD_XCE2_PORT       GPIOB
#define EPD_XCE2            GPIO_Pin_13   //PB13    U1CE2
#define EPD_XCE2_PIN        PB13
#define EPD_GMODE2_PORT     GPIOB
#define EPD_GMODE2          GPIO_Pin_14   //PB14    MODE2
#define EPD_GMODE2_PIN      PB14
#define EPD_XRL_PORT        GPIOB
#define EPD_XRL             GPIO_Pin_15   //PB15    RL
#define EPD_XRL_PIN         PB15

#define EPD_DB_PORT         GPIOC

#define EPD_XSPV_PORT       GPIOC
#define EPD_XSPV            GPIO_Pin_8    //PC8   SPV
#define EPD_XSPV_PIN        PC8
#define EPD_CLK_PORT        GPIOC
#define EPD_CLK             GPIO_Pin_9    //PC9   CPV
#define EPD_CLK_PIN         PC9
#define EPD_SPH_PORT        GPIOC
#define EPD_SPH             GPIO_Pin_13   //PC13  SPH
#define EPD_SPH_PIN         PC13

#define VCOM_CTR_PORT       GPIOB
#define VCOM_CTR            GPIO_Pin_1    //PB1     4053 S2
#define VCOM_CTR_PIN        PB1

#define VNEGGVEE_CTR_PORT   GPIOA
#define VNEGGVEE_CTR        GPIO_Pin_1    //PA1     AME5142
#define VNEGGVEE_CTR_PIN    PA1
#define VPOS15_CTR_PORT     GPIOA
#define VPOS15_CTR          GPIO_Pin_2    //PA2     Q4
#define VPOS15_CTR_PIN      PA2
#define GVDD22_CTR_PORT     GPIOA
#define GVDD22_CTR          GPIO_Pin_3    //PA3     Q5
#define GVDD22_CTR_PIN      PA3

#define CL_DLY 4
#define CLK_DLY 1

#define EPD_CL_H            EPD_CL_PORT->BSRR = EPD_CL; delay140ns(CL_DLY);
#define EPD_CL_L            EPD_CL_PORT->BRR = EPD_CL; delay140ns(CL_DLY);

#define EPD_LE_H            EPD_LE_PORT->BSRR = EPD_LE
#define EPD_LE_L            EPD_LE_PORT->BRR = EPD_LE

#define EPD_OE_H            EPD_OE_PORT->BSRR = EPD_OE
#define EPD_OE_L            EPD_OE_PORT->BRR = EPD_OE

#define EPD_SPH_H           EPD_SPH_PORT->BSRR = EPD_SPH
#define EPD_SPH_L           EPD_SPH_PORT->BRR = EPD_SPH

#define EPD_XSPV_H          EPD_XSPV_PORT->BSRR = EPD_XSPV
#define EPD_XSPV_L          EPD_XSPV_PORT->BRR = EPD_XSPV

#define EPD_CLK_H           EPD_CLK_PORT->BSRR = EPD_CLK; delay140ns(CLK_DLY);
#define EPD_CLK_L           EPD_CLK_PORT->BRR = EPD_CLK; delay140ns(CLK_DLY);

GxIO_GreenSTM32F103V::GxIO_GreenSTM32F103V()
{
}

void GxIO_GreenSTM32F103V::init(void)
{
  // PORT_A_PINS
  pinMode(EPD_XCE1_PIN, OUTPUT);
  pinMode(VNEGGVEE_CTR_PIN, OUTPUT);
  pinMode(VPOS15_CTR_PIN, OUTPUT);
  pinMode(GVDD22_CTR_PIN, OUTPUT);
  // PORT_B_PINS
  pinMode(EPD_CL_PIN, OUTPUT);
  pinMode(EPD_OE_PIN, OUTPUT);
  pinMode(EPD_LE_PIN, OUTPUT);
  pinMode(EPD_SHR_PIN, OUTPUT);
  pinMode(EPD_GMODE1_PIN, OUTPUT);
  pinMode(EPD_XCE2_PIN, OUTPUT);
  pinMode(EPD_GMODE2_PIN, OUTPUT);
  pinMode(EPD_XRL_PIN, OUTPUT);
  pinMode(VCOM_CTR_PIN, OUTPUT);
  // PORT_C_PINS
  pinMode(EPD_XSPV_PIN, OUTPUT);
  pinMode(EPD_CLK_PIN, OUTPUT);
  pinMode(EPD_SPH_PIN, OUTPUT);
  for (uint8_t p = PC0; p <= PC7; p++)
  {
    pinMode(p, OUTPUT);
  }
  // EPD_Init(void)
  digitalWrite(EPD_SHR_PIN, LOW);     //Shift direction source driver
  digitalWrite(EPD_GMODE1_PIN, HIGH); //one pulse mode
  digitalWrite(EPD_GMODE2_PIN, HIGH); //one pulse mode
  //digitalWrite(EPD_XRL_PIN, HIGH);    //Shift direction gate driver
  digitalWrite(EPD_XRL_PIN, LOW);    //Shift direction gate driver
  digitalWrite(EPD_XCE1_PIN, LOW);
  digitalWrite(EPD_XCE2_PIN, LOW);
  powerOff();
  EPD_LE_L;
  EPD_CL_L;
  EPD_OE_L;
  EPD_SPH_H;
  EPD_XSPV_H;
  EPD_CLK_L;
}

void GxIO_GreenSTM32F103V::powerOn(void)
{
  digitalWrite(VNEGGVEE_CTR_PIN, HIGH);
  delay140ns(0xf);
  digitalWrite(VPOS15_CTR_PIN, HIGH);
  delay140ns(0xf);
  digitalWrite(GVDD22_CTR_PIN, HIGH);
  delay140ns(0xf);
  digitalWrite(VCOM_CTR_PIN, HIGH);
  delay140ns(0xff);
}

void GxIO_GreenSTM32F103V::powerOff(void)
{
  digitalWrite(VCOM_CTR_PIN, LOW);
  delay140ns(0x2ff);
  digitalWrite(GVDD22_CTR_PIN, LOW);
  delay140ns(0x2f);
  digitalWrite(VPOS15_CTR_PIN, LOW);
  delay140ns(0x2f);
  digitalWrite(VNEGGVEE_CTR_PIN, LOW);
  delay140ns(0x2ff);
}

void GxIO_GreenSTM32F103V::start_scan(void)
{
  EPD_XSPV_H;

  uint8_t repeat = 2;
  while (repeat--)
  {
    EPD_CLK_L;
    EPD_CLK_H;
  }

  EPD_XSPV_L;

  repeat = 2;
  while (repeat--)
  {
    EPD_CLK_L;
    EPD_CLK_H;
  }

  EPD_XSPV_H;
  repeat = 2;
  while (repeat--)
  {
    EPD_CLK_L;
    EPD_CLK_H;
  }
}

void GxIO_GreenSTM32F103V::send_row(uint8_t row_data[], uint16_t row_size)
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
    EPD_DB_PORT->BRR = 0xFF; // reset data bits
    EPD_DB_PORT->BSRR = (uint16_t)row_data[column]; // set data bits

    EPD_CL_L;
    EPD_CL_H;
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

  EPD_CLK_H;
}

static uint32_t t_delay;

void GxIO_GreenSTM32F103V::delay140ns(uint32_t nCount)
{
  for (; nCount != 0; nCount--)
  {
    t_delay += nCount;
  }
}


