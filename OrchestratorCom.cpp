//#include "LiveObjects.h"
#include "ApplicationManager.h"
#include "Adafruit_INA219.h"
#include "OrchestratorCom.h"
#include "OrchestratorCom_Config.h"
#include <ArduinoJson.h>

OrchestratorCom orchestratorCom;

OrchestratorCom::OrchestratorCom() {
  generalWifiStatus = true;
  wifi_2GHz_statuz = true;
  wifi_5GHz_statuz = true;
  wifi_6GHz_statuz = true;
  lastExecution = 0;
  electrical_panel_status = "000000";
  current_use_situation = "Unknown";
}

OrchestratorCom::~OrchestratorCom() {
}

void OrchestratorCom::begin(Object* parent) {
  //Launch start message for start serial communication with orchestrator
  Serial.println("OrchestratorCom begin");
  this->parent = parent;
}

void OrchestratorCom::loop() {
  unsigned long currentTime = millis();

  if ((currentTime - lastExecution > ORCHESTRATOR_NOTIFICATION_DELAY) || (lastExecution == 0)) {
    notifyStatusToOrchestrator();
    lastExecution = currentTime;
  }
  readSerial();
}

void OrchestratorCom::notifyStatusToOrchestrator() {
  if (parent == NULL) return;
  StaticJsonDocument<511> doc;
  JsonObject alimelo_values = doc.createNestedObject("alimelo");

  getJsonAlimeloStatus(alimelo_values);

  Serial.println("ORCHESTRATOR_SERIAL_NOTIFICATION_BEGINS");
  String output;
  serializeJson(doc, output);
  Serial.println(output);
  Serial.println("ORCHESTRATOR_SERIAL_NOTIFICATION_ENDS");
}


void OrchestratorCom::sendCommandToOrchestrator(JsonObject& command) {  
  Serial.println("ORCHESTRATOR_SERIAL_COMMAND_BEGINS");  
  String output;
  serializeJson(command, output);
  Serial.println(output);
  Serial.println("ORCHESTRATOR_SERIAL_COMMAND_ENDS");
}

void OrchestratorCom::readSerial() {
  if (Serial.available() > 0) {
    String input = Serial.readString();
    Serial.print("Received: ");Serial.println(input);

    StaticJsonDocument<1024> doc;
    deserializeJson(doc, input);
    JsonObject obj = doc.as<JsonObject>();

    generalWifiStatus = doc["wf"]["w"];    
    wifi_2GHz_statuz = doc["wf"]["w2"];
    wifi_5GHz_statuz = doc["wf"]["w5"];
    wifi_6GHz_statuz = doc["wf"]["w6"];
    electrical_panel_status = doc["ep"].as<String>();
    current_use_situation = doc["us"].as<String>();

    //Publish orchestrator data to LiveObjects
    publishOrchestratorStatusToLiveObjects();
  }
}

void OrchestratorCom::publishOrchestratorStatusToLiveObjects(){
  StaticJsonDocument<511> doc;
  JsonObject orchestrator_values = doc.createNestedObject("orch");
  getJsonOrchestratorStatus(orchestrator_values); 
  liveObjects.sendCustomData(orchestrator_values, "orch");  
}

void OrchestratorCom::manageLiveObjectsCommand(bool command, JsonObject& args){
  if(!command){
    publishOrchestratorStatusToLiveObjects();
  }
  else{
    sendCommandToOrchestrator(args);
  }
}

sWifiStatus OrchestratorCom::getWifiStatus() {
  Serial.println("OrchestratorCom getWifiStatus");
  sWifiStatus currentWifiStatus;
  return currentWifiStatus;
}

void OrchestratorCom::getJsonAlimeloStatus(JsonObject& value) {
  ((ApplicationManager*)parent)->getJsonConsumption(value);
  ((ApplicationManager*)parent)->getJsonBatteryLevel(value);
  ((ApplicationManager*)parent)->getJsonVinState(value);
  ((ApplicationManager*)parent)->getJsonPoweredBattery(value);
  ((ApplicationManager*)parent)->getJsonChragingBattery(value);
}

void OrchestratorCom::getJsonOrchestratorStatus(JsonObject& value) {
  JsonObject wifi_values = value.createNestedObject("wf");
  getJsonWifiStatus(wifi_values);
  getJsonElectricPanelStatus(value);
  getJsonElectricPanelStatus(value);
  getJsonUseSituation(value);
}

void OrchestratorCom::getJsonWifiStatus(JsonObject& value) {
  value["w"] = generalWifiStatus;
  value["w2"] = wifi_2GHz_statuz;
  value["w5"] = wifi_5GHz_statuz;
  value["w6"] = wifi_6GHz_statuz;
}

void OrchestratorCom::getJsonElectricPanelStatus(JsonObject& value) {
  value["ep"] = electrical_panel_status;
}

void OrchestratorCom::getJsonUseSituation(JsonObject& value) {
  value["us"] = current_use_situation;
}

void OrchestratorCom::getJsonUseSituationsList(JsonObject& value) {
  Serial.println("OrchestratorCom getJsonUseSituationsList");
}

void OrchestratorCom::getJsonGeneralStatus(JsonObject& value) {
  Serial.println("OrchestratorCom getJsonGeneralStatus");
}

bool OrchestratorCom::setUseSituation(String new_situation) {
  Serial.println("OrchestratorCom setUseSituation");
}

bool OrchestratorCom::setWifiStatus(bool wifi_status) {
  Serial.println("OrchestratorCom setWifiStatus");
}

bool OrchestratorCom::setWifiBandStatus(String band, bool status) {
  Serial.println("OrchestratorCom setWifiBandStatus");
}
