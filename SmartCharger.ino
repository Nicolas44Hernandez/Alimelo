#include <Arduino.h>
#include "ApplicationManager.h"

void setup() 
{
	  Serial.begin(9600); 
	  // while ((!Serial) && (millis() < 10000));  
    delay(10000);
	  Serial.println("*** Start application version 2.0.0 ***");  
}

void loop()
{
  applicationManager.execute();
  while(true);
}
