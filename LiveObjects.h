#ifndef LIVE_OBJECTS_H
#define LIVE_OBJECTS_H

#include <Arduino.h>
#include "ArduinoJson.h" 
#include "PubSubClient.h"
#include "Adafruit_FONA.h"
#include "Object.h"

class LiveObjects : public Object
{
protected: 
  //Timestamp of last reconnection
  long lastReconnectAttempt;
  //Timestamp for last execution
  long lastExecution;
  //Timestamp for reset
  long resetAttempt;
  
  Adafruit_FONA livebooster;  
  PubSubClient mqtt;
  Object* parent;

  bool mqttConnect();
  
public:  
  LiveObjects();
  virtual ~LiveObjects(); 
  void begin(Object* parent);
  bool connect();
  void loop();  
  void alertSending(void);
  void subscribeToTopics();
  bool publish(const char * topicArg, String output, bool alertUser, void(*callback)(void));
  void sendData(bool stateChanged);
  void sendCustomData(DynamicJsonDocument  data, String key);
  void onMqttCallback(char* topic, uint8_t * payload, unsigned int len);
  void powerOff();
};

extern LiveObjects liveObjects;
#endif
