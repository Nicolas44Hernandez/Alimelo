/***************************************************
  This is a library for our Adafruit FONA Cellular Module

  Designed specifically to work with the Adafruit FONA
  ----> http://www.adafruit.com/products/1946
  ----> http://www.adafruit.com/products/1963

  These displays use TTL Serial to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/
#ifndef SERIAL_HERACLES_H
#define SERIAL_HERACLES_H

#include <Arduino.h>

class SerialHeraclesClass : public Uart
{
public:
  SerialHeraclesClass();
  virtual ~SerialHeraclesClass();
  
  virtual void start(long baudrate);
};

extern SerialHeraclesClass SerialHeracles;
#endif
