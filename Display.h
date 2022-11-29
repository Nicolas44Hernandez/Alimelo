#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

#include "SPI.h"
#include <Wire.h>
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"

class Display 
{
protected: 
  Adafruit_SSD1306 lcd;
  void showLogo();
public:
  Display();
  virtual ~Display();
  
  void begin();
  void setText(String text, uint16_t x, uint16_t y, uint16_t textSize, uint16_t color, uint16_t backgroundColor = SSD1306_BLACK);
};

extern Display display;

#endif
