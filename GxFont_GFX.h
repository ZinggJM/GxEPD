// class GxFont_GFX : Font Rendering Graphics Switch and Bridge Class
//
// This class allows to connect GxEPD to additional font rendering classes.
//
// Adafruit_ftGFX: a Adafruit_GFX variant with different fonts.
// need to use modified clone from: https://github.com/ZinggJM/Adafruit_ftGFX
//
// GxFont_GFX_TFT_eSPI: fonts and font rendering of TFT_eSPI library (Bodmer)
// available here: https://github.com/ZinggJM/GxFont_GFX_TFT_eSPI
//
// Author : J-M Zingg
//
// Version : see library.properties
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// Library: https://github.com/ZinggJM/GxEPD

#ifndef _GxFont_GFX_H_
#define _GxFont_GFX_H_

#include <Adafruit_GFX.h>

// select the library/libraries to add, none to preserve code space
//#include <Adafruit_ftGFX.h>
//#include <GxFont_GFX_TFT_eSPI.h>

class GxFont_GFX : public Adafruit_GFX
{
  public:
    GxFont_GFX(int16_t w, int16_t h);
    void setFont(const GFXfont *f = NULL);
#if defined(_ADAFRUIT_TF_GFX_H_)
    void setFont(uint8_t f);
#endif
#if defined(_GxFont_GFX_TFT_eSPI_H_)
#ifdef LOAD_GFXFF
    void setFreeFont(const GFXfont *f = NULL);
    void setTextFont(uint8_t font);
#else
    void setFreeFont(uint8_t font);
    void setTextFont(uint8_t font);
#endif
#endif
#if defined(_ADAFRUIT_TF_GFX_H_) || defined(_GxFont_GFX_TFT_eSPI_H_)
    void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size);
    void setCursor(int16_t x, int16_t y);
    void setTextColor(uint16_t c);
    void setTextColor(uint16_t c, uint16_t bg);
    void setTextSize(uint8_t s);
    void setTextWrap(boolean w);
    int16_t getCursorX(void) const;
    int16_t getCursorY(void) const;
    size_t write(uint8_t);
#endif
#if defined(_GxFont_GFX_TFT_eSPI_H_)
    void setTextDatum(uint8_t datum);
    void setTextPadding(uint16_t x_width);
    uint8_t getTextDatum(void);
    uint8_t color16to8(uint16_t color565); // Convert 16 bit colour to 8 bits
    uint16_t fontsLoaded(void);
    uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
    uint16_t color8to16(uint8_t color332);  // Convert 8 bit colour to 16 bits
    int16_t drawNumber(long long_num, int poX, int poY, int font);
    int16_t drawNumber(long long_num, int poX, int poY);
    int16_t drawFloat(float floatNumber, int decimal, int poX, int poY, int font);
    int16_t drawFloat(float floatNumber, int decimal, int poX, int poY);
    // Handle char arrays
    int16_t drawString(const char *string, int poX, int poY, int font);
    int16_t drawString(const char *string, int poX, int poY);
    // Handle String type
    int16_t drawString(const String& string, int poX, int poY, int font);
    int16_t drawString(const String& string, int poX, int poY);
    int16_t textWidth(const char *string, int font);
    int16_t textWidth(const char *string);
    int16_t textWidth(const String& string, int font);
    int16_t textWidth(const String& string);
    int16_t fontHeight(int16_t font);
#endif
  private:
#if defined(_ADAFRUIT_TF_GFX_H_)
    class GxF_Adafruit_tfGFX : public Adafruit_tfGFX
    {
      public:
        GxF_Adafruit_tfGFX(GxFont_GFX& container, int16_t w, int16_t h) : Adafruit_tfGFX(w, h), _container(container) {};
        void drawPixel(int16_t x, int16_t y, uint16_t color);
      private:
        GxFont_GFX& _container;
    };
    GxF_Adafruit_tfGFX _GxF_Adafruit_tfGFX;
#endif
#if defined(_GxFont_GFX_TFT_eSPI_H_)
    class GxF_GxFont_GFX_TFT_eSPI : public GxFont_GFX_TFT_eSPI
    {
      public:
        GxF_GxFont_GFX_TFT_eSPI(GxFont_GFX& container, int16_t w, int16_t h) : GxFont_GFX_TFT_eSPI(w, h), _container(container) {};
        void drawPixel(uint32_t x, uint32_t y, uint32_t color);
        void drawFastHLine(int32_t x, int32_t y, int32_t w, uint32_t color);
        void fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color);
      private:
        GxFont_GFX& _container;
    };
    GxF_GxFont_GFX_TFT_eSPI _GxF_GxFont_GFX_TFT_eSPI;
#endif
    uint16_t _font_gfx;
};

#endif
