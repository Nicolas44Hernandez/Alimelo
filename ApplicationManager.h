#ifndef APPLICATION_MANAGER_H
#define APPLICATION_MANAGER_H

#include <Arduino.h>
#include "config.h"
#include "LiveObjects.h"
#include "Adafruit_INA219.h"
/* #include "display.h" */
#include "RTCZero.h"
#include "Object.h"

  typedef struct deviceState {
    bool vinState;
    bool powredByBattery;
    bool chargingBattery;
  }sDeviceState;

class ApplicationManager : public Object
{
protected: 
  volatile bool interruptAlarmTime;
  volatile bool interruptVinState;
  volatile bool interruptButton;
protected:
  Adafruit_INA219 ina219;
  RTCZero rtc;

  sDeviceState deviceState;

  void init();
  void run();


  sDeviceState getDeviceState();
  bool deviceStateIsEquals(sDeviceState deviceState, sDeviceState currentDeviceState);

  void switchToBattery(bool enable);
  void disableVOut(bool disable);

  bool electricSocketIsPowerSupplied();
  bool isPowredByBattery();
  bool isButtonPressed();  
  bool isChargingBattery();

  void checkPower();
  void checkAlarm();

  void processActionButton();

public:
  ApplicationManager();
  virtual ~ApplicationManager();

  void execute();  

  bool deviceStateChanged();

  void getJsonConsumption(JsonObject& value);
  void getJsonBatteryLevel(JsonObject& value);
  void getJsonVinState(JsonObject& value);
  void getJsonPoweredBattery(JsonObject& value);
  void getJsonChragingBattery(JsonObject& value);

  void onAlarmTime();
  void onInterruptCallbackVinState();
  void onInterruptCallbackButton();

  void onChangeStateBattery(bool state);
  void onChangePowerVOut(bool state);

  void logConsumption();
  void logBatteryLevel();
};

extern ApplicationManager applicationManager;

#endif
