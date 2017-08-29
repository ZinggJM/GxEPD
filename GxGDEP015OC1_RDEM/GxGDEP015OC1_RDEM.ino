#include "GDEP015OC1.h"
#include "BitmapExamples.h"

#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
#include <GxIO/GxIO.cpp>

GxIO_Class io(SPI, SS, 8, 9);
//  GxGDEP015OC1(GxIO& io, uint8_t rst = 9, uint8_t busy = 7);
GxEPD_Class display(io, 9, 3);


void setup(void)
{
  display.init();
  display.showDemoExample();
}

void loop()
{
  
}

