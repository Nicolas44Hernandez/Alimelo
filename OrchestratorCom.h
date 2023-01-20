#ifndef ORCHESTRATOR_COM_H
#define ORCHESTRATOR_COM_H

#include <Arduino.h>
#include "ArduinoJson.h" 
#include "Adafruit_INA219.h"
#include "Object.h"

  typedef struct wifiStatus {
    bool wifiStatus;
    bool wifi_6GHz_status;
    bool wifi_5GHz_status;
    bool wifi_2GHz_status;
  }sWifiStatus;
 
class OrchestratorCom : public Object
{
protected: 
  bool generalWifiStatus;
  bool wifi_2GHz_statuz;
  bool wifi_5GHz_statuz;
  bool wifi_6GHz_statuz;
  String electrical_panel_status;
  String current_use_situation;
  long lastExecution;  
  sWifiStatus wifiStatus;
  Adafruit_INA219 ina219;
  Object* parent;

  void notifyStatusToOrchestrator();
  void readSerial();
  sWifiStatus getWifiStatus(); 
  void getJsonAlimeloStatus(JsonObject& value);
  void getJsonOrchestratorStatus(JsonObject& value);
  void publishOrchestratorStatusToLiveObjects();
  void sendCommandToOrchestrator(JsonObject& command);
  
public:
  OrchestratorCom();
  virtual ~OrchestratorCom();
  void begin(Object* parent);
  void loop();  

  void getJsonWifiStatus(JsonObject& value);
  void getJsonElectricPanelStatus(JsonObject& value);
  void getJsonUseSituation(JsonObject& value);  
  void getJsonUseSituationsList(JsonObject& value);
  void getJsonGeneralStatus(JsonObject& value);

  void manageLiveObjectsCommand(bool command, JsonObject& args);

  bool setUseSituation(String new_situation);
  bool setWifiStatus(bool wifi_status);
  bool setWifiBandStatus(String band, bool status);  
};

extern OrchestratorCom orchestratorCom;

#endif