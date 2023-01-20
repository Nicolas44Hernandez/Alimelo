#include "ApplicationManager.h"
#include "Adafruit_INA219.h"
#include "ArduinoLowPower.h"

// TODO: review pinout variants

ApplicationManager applicationManager;

void resetCallback()
{
  NVIC_SystemReset();
}

void interruptAlarmTimeCallback()
{
  applicationManager.onAlarmTime();
}

void interruptCallbackVinState()
{
  applicationManager.onInterruptCallbackVinState();
}

void interruptCallbackButton()
{
  applicationManager.onInterruptCallbackButton();
}

ApplicationManager::ApplicationManager() : ina219(), rtc()
{    
  interruptAlarmTime = false;
  interruptVinState = false;
  interruptButton = false;

  deviceState.chargingBattery = false;
  deviceState.powredByBattery = false;
  deviceState.vinState = false;
}

ApplicationManager::~ApplicationManager()
{
}

void ApplicationManager::execute()
{
  init();
  run();
}

void ApplicationManager::init()
{
  digitalWrite(BAT_ON, HIGH);
  pinMode(BAT_ON, OUTPUT);
      
  pinMode(BUTTON, INPUT);  
  attachInterrupt(BUTTON, interruptCallbackButton, RISING);

  pinMode(VIN_STATE, INPUT);

  digitalWrite(VIN_OFF, LOW);
  pinMode(VIN_OFF, OUTPUT);

  pinMode(CHARGE_STATE, INPUT);

  liveObjects.begin(this);
  orchestratorCom.begin(this);
  ina219.begin();

  //  display.begin(); 
  //  display.setText("hello", 0, 0, 1, SSD1306_WHITE); 

  rtc.begin(true);
  rtc.attachInterrupt(interruptAlarmTimeCallback);
  rtc.setAlarmTime(23, 59, 59);  // alarm every day 
  rtc.enableAlarm(rtc.MATCH_HHMMSS);

  deviceState = getDeviceState();
}

  
void ApplicationManager::processActionButton()
{
  if(interruptButton)
  {
    interruptButton = false;
    Serial.print("Push button interrupt");
  }
}

void ApplicationManager::logConsumption()
{
  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  loadvoltage = busvoltage + (shuntvoltage / 1000);
  Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
  Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
  Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
  Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
  Serial.print("Power:         "); Serial.print(power_mW); Serial.println(" mW");    
}

void ApplicationManager::getJsonConsumption(JsonObject& value)
{
  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  loadvoltage = busvoltage + (shuntvoltage / 1000);

  //logConsumption(); 

  value["bv"] = busvoltage;
  value["sw"] = shuntvoltage;
  value["lv"] = loadvoltage;
  value["ma"] = current_mA;
  value["pw"] = power_mW;
}

void ApplicationManager::logBatteryLevel()
{
  int batLevel = analogRead(BAT_LEVEL);
  Serial.print("BatLevel:");Serial.println(batLevel);
}

void ApplicationManager::getJsonBatteryLevel(JsonObject& value)
{
  int batLevel = analogRead(BAT_LEVEL);
  //Serial.print("BatLevel:");Serial.println(batLevel); 
  value["bat"] = batLevel;
}

sDeviceState ApplicationManager::getDeviceState()
{
  sDeviceState currentDeviceState;

  currentDeviceState.vinState = electricSocketIsPowerSupplied();
  currentDeviceState.powredByBattery = isPowredByBattery();
  currentDeviceState.chargingBattery = isChargingBattery();

  return currentDeviceState;
}

bool ApplicationManager::deviceStateIsEquals(sDeviceState bakDeviceState, sDeviceState currentDeviceState)
{
  static uint8_t countCheckCharge = 0;
  static bool checkStateBatteryStarted = false;

  bool equal = (bakDeviceState.vinState == currentDeviceState.vinState);
  equal &= (bakDeviceState.powredByBattery == currentDeviceState.powredByBattery);

  if(!equal) return false;

  if(bakDeviceState.chargingBattery != currentDeviceState.chargingBattery)
  {
    if(!checkStateBatteryStarted) 
    {
      checkStateBatteryStarted = true;
      countCheckCharge = 0;
    } 
    else
    {
      countCheckCharge++;
      if(countCheckCharge > 20)
      {
        countCheckCharge = 0;
        checkStateBatteryStarted = false;
        return false;
      }
    }
  }

  return true;
}

bool ApplicationManager::deviceStateChanged()
{
    sDeviceState currentDeviceState = getDeviceState();

    bool res = deviceStateIsEquals(deviceState, currentDeviceState);

    if(!res)
    {      
      deviceState = currentDeviceState;
      return true;
    }
    return false;
}

void ApplicationManager::getJsonVinState(JsonObject& value)
{
  bool state = electricSocketIsPowerSupplied();
  //Serial.print("VIN State: "); Serial.println(state ? "Missing" : "Present");
  value["vs"] = state;
}

void ApplicationManager::getJsonPoweredBattery(JsonObject& value)
{
  bool state = isPowredByBattery();
  // Serial.print("Powered by battery: "); Serial.println(state ? "yes" : "no");
  value["pb"] = state;
}

void ApplicationManager::getJsonChragingBattery(JsonObject& value)
{
  bool state = isChargingBattery();
  //Serial.print("Battery in charge: "); Serial.println(state ? "yes" : "no");
  value["ch"] = state;

}

void ApplicationManager::run()
{
	if(liveObjects.connect())
  {
    while(true)
    {
      bool state = isChargingBattery();
      //Serial.print("Battery in charge: "); Serial.println(state ? "yes" : "no");
      //logBatteryLevel();
      //logConsumption();
      orchestratorCom.loop();      
      liveObjects.loop(); 
      processActionButton();
      checkPower();
      checkAlarm();
      delay(250);
    }
  }
}

void ApplicationManager::checkAlarm()
{
  //Serial.print("checkAlarm: "); Serial.println(interruptAlarmTime ? "yes" : "no");
  if((interruptAlarmTime) && (!isPowredByBattery()))
    NVIC_SystemReset();
}

void ApplicationManager::checkPower()
{
  int batLevel = analogRead(BAT_LEVEL);
  //Serial.print("BatLevel:");Serial.println(batLevel);
  //Serial.print("Powered By Battery:");Serial.println(isPowredByBattery());

  if((batLevel < BAT_THRESHOLD) && isPowredByBattery())
  {
    if(!electricSocketIsPowerSupplied())
    {
      //Serial.print("Low Power mode");
      //disableVOut(true);
      //liveObjects.powerOff();
      //LowPower.attachInterruptWakeup(VIN_STATE, resetCallback, INPUT_PULLDOWN);
      delay(1000);

      //LowPower.deepSleep();
    }
    else
    {
      Serial.print("Switch to power supply");
      switchToBattery(false);
    }    
  }
}

void ApplicationManager::disableVOut(bool disable)
{
  digitalWrite(BAT_ON, !disable);  
}

bool ApplicationManager::isChargingBattery()
{
  return (digitalRead(CHARGE_STATE) == LOW);
}

void ApplicationManager::switchToBattery(bool enable)
{
  int state = enable ? HIGH : LOW;
  if(electricSocketIsPowerSupplied() && (!enable)) digitalWrite(VIN_OFF, state);
  else digitalWrite(VIN_OFF, enable);
}

bool ApplicationManager::isPowredByBattery()
{
  if(!electricSocketIsPowerSupplied()) return true; 
  return (digitalRead(VIN_OFF) == HIGH);
}

bool ApplicationManager::electricSocketIsPowerSupplied()
{
  return (digitalRead(VIN_STATE) == LOW);
}

bool ApplicationManager::isButtonPressed()
{
  return (digitalRead(BUTTON) == HIGH);
}

void ApplicationManager::onAlarmTime()
{
  Serial.println("************* ON ALARM ************");
  interruptAlarmTime = true;
}

void ApplicationManager::onInterruptCallbackVinState()
{
  interruptVinState = true;
}

void ApplicationManager::onInterruptCallbackButton()
{
  interruptButton = true;
}

void ApplicationManager::onChangeStateBattery(bool state)
{
  Serial.println(state ? "Switch to battery" : "Switch to power supply");
  switchToBattery(state);
}

void ApplicationManager::onChangePowerVOut(bool state)
{
  Serial.println(state ? "Livebox power off" : "Livebox power on");
  disableVOut(state);
}
