#include "SerialHeracles.h"

#include <Wire.h>
#include "wiring_private.h"

SerialHeraclesClass SerialHeracles;

void SERCOM2_Handler()
{
  SerialHeracles.IrqHandler();
}

SerialHeraclesClass::SerialHeraclesClass() : Uart(&sercom2, 3, 4, SERCOM_RX_PAD_1, UART_TX_PAD_0)
{
}

SerialHeraclesClass::~SerialHeraclesClass()
{
}

void SerialHeraclesClass::start(long baudrate)
{
	Serial.println(F("SerialHeraclesClass begin"));
	Uart::begin(baudrate);
	pinPeripheral ( 3 , PIO_SERCOM_ALT );
	pinPeripheral ( 4 , PIO_SERCOM_ALT );	
}
