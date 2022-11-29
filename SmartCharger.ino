#include <Arduino.h>
#include "ApplicationManager.h"

void setup() 
{
	  Serial.begin(9600); 
	  //while ((!Serial) && (millis() < 10000));  
    
	  Serial.println("*** Start application version 1.0.1 ***");  
}

void loop()
{
  applicationManager.execute();
  while(true);
}

