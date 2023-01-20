#include "LiveObjects.h"
#include "LO_config.h"
#include "SerialHeracles.h"
#include "ApplicationManager.h"

LiveObjects liveObjects;

void mqttCallback(char* topic, uint8_t * payload, unsigned int len) 
{
  liveObjects.onMqttCallback(topic, payload, len);
}

LiveObjects::LiveObjects() : livebooster(), mqtt(livebooster)
{  
  parent = NULL;
  lastReconnectAttempt = 0;  
  lastExecution = 0;
  resetAttempt = 0;
}

LiveObjects::~LiveObjects()
{
}

void LiveObjects::begin(Object* parent)
{
  this->parent = parent;
  SerialHeracles.start(9600);
  livebooster.begin(SerialHeracles);

  livebooster.powerOn();
  
  if(!livebooster.start()) 
  { 
    Serial.println(F(">> Couldn't find Heracles <<"));
    Serial.println(F("*** Application finished ***"));
    while(true);
  }   
}

void LiveObjects::powerOff()
{
  livebooster.powerOn(false);  
}

bool LiveObjects::connect()
{
  bool foundHeraclesV2 = livebooster.isHeracles();
  
  if(foundHeraclesV2) Serial.println("Heracles V2 Found");
  else Serial.println("Heracles V2 not Found");

  char imsi[21];
  
  if(livebooster.getSIMIMSI(imsi))
  {
    Serial.print(">> IMSI = ");Serial.println(imsi);
  }
  else
  {
    Serial.print(">> IMSI not found");
  }

 // avant 38 
  livebooster.setPreferredSelection(51);
  uint8_t n = 0;
  
  uint8_t scanTimeout = 0;

  while((n != 1) && (scanTimeout < SCAN_TIMEOUT_DELAY))
  {
    n = livebooster.getNetworkStatus();
    Serial.print(F("Network status "));
    Serial.print(n);
    Serial.print(F(": "));
    if (n == 0) Serial.println(F("Not registered"));
    if (n == 1) Serial.println(F("Registered (home)"));
    if (n == 2) Serial.println(F("Not registered (searching)"));
    if (n == 3) Serial.println(F("Denied"));
    if (n == 4) Serial.println(F("Unknown"));
    if (n == 5) Serial.println(F("Registered roaming"));
    
    scanTimeout++;

    delay(2000);
  }    

  if(n != 1) 
  {
      return false;
  }
  // nouvelle ligne ci dessous
  livebooster.getPreferredSelection();  

  int8_t state = 0;
  char buffer[256];
    
  //Serial.print(F(">> Mode: ")); Serial.print(fona.getMode());

  livebooster.setGPRSNetworkSettings(apn, user, pass);
  if(!livebooster.enableGPRS(true))
  {
      Serial.println(F(">> GPRS failed"));      
  }
  else Serial.println(F(">> GPRS connected"));

  // init mqtt   
  Serial.println("MQTT setServer");
  mqtt.setServer(domain, PORT_MQTT);
  mqtt.setCallback(mqttCallback);
  
  mqttConnect();
  return true;
}

bool LiveObjects::mqttConnect() {
  Serial.print("Connecting to ");
  Serial.println(domain);

  if (!mqtt.connect(deviceid, LO_USER, LO_MDP)) {
    Serial.println(">> MQTT connect failed");
    return false;
  }
  
  Serial.println(">> MQTT connect success");
  
  int connected = mqtt.connected();
  
  if (connected)
  {     
    subscribeToTopics();
  }
  else
  {
    Serial.println("Not connected");
  }
  return connected;
}

void LiveObjects::loop()
{
  unsigned long currentTime = millis();
  
  if (mqtt.connected()) 
  {
    mqtt.loop();    

    bool stateChanged = ((ApplicationManager*)parent)->deviceStateChanged();
    
    if((currentTime - lastExecution > SEND_DELAY) || (lastExecution == 0) || (stateChanged))
    {      
      // LIGNE EN DESSOUS A ENLEVER SI ON VEUT ZERO DONNEES A LO ET A REMETTRE APRES LE TEST 
      sendData(stateChanged);
      lastExecution = currentTime;
      Serial.println(">> send success");
    }
  }
  else 
  {    
    if( ((currentTime - lastReconnectAttempt) > DELAY_ATTEMPT_CONNECTION) || (lastReconnectAttempt == 0)) // Reconnect every 10 seconds
    {
      if(lastReconnectAttempt == 0)
      {
        resetAttempt = currentTime;  
      }

      lastReconnectAttempt = currentTime;
      
      if (mqttConnect()) 
      {
        lastReconnectAttempt = 0;
        lastExecution = 0;
        resetAttempt = 0;
      }

      if((currentTime - resetAttempt) > RESET_DELAY_AFTER_LOST_CONNECTION)
      {
        NVIC_SystemReset();
      }
    }
  }
} 

 void LiveObjects::alertSending(void)
{
  Serial.println("Sending data");
}

void LiveObjects::subscribeToTopics()
{
  Serial.println(">> subscribe to Topics <<");
  mqtt.subscribe(cmdTopic);
}

bool LiveObjects::publish(const char * topicArg, String output, bool alertUser, void(*callback)(void))
{
  Serial.println("publish");
  boolean result = true;
  result = mqtt.publish(topicArg, output.c_str());
  if (result)
  {
    Serial.println();
    Serial.print("publish : ");
    Serial.print(topicArg);
    Serial.println(output);
    if (alertUser)
      callback();
  }
  else
  {
    Serial.println();
    Serial.print("publish : Error during publishing");
  }
  return result;
}

void LiveObjects::sendData(bool stateChanged)
{
  if(parent == NULL) return;
  StaticJsonDocument<255> doc;
  doc["s"] = String("eff");
  JsonObject value = doc.createNestedObject("v");

  ((ApplicationManager*)parent)->getJsonConsumption(value);
  ((ApplicationManager*)parent)->getJsonBatteryLevel(value);
  ((ApplicationManager*)parent)->getJsonVinState(value);
  ((ApplicationManager*)parent)->getJsonPoweredBattery(value);
  ((ApplicationManager*)parent)->getJsonChragingBattery(value);

  value["rssi"] = stateChanged ? 99 : livebooster.getRSSI() ;

  Serial.print(">> Send Data <<");

  String output;
  serializeJson(doc, output);

  Serial.println(output);

  publish(postDataTopic, output, false, NULL);     

}


void LiveObjects::sendCustomData(DynamicJsonDocument  data_json, String key)
{
  JsonObject root = data_json.as<JsonObject>();
  StaticJsonDocument<512> doc;
  doc["s"] = String("eff");
  JsonObject value = doc.createNestedObject("v");
  JsonObject orchestrator_data = value.createNestedObject(key);

  for (JsonPair kv : root) {
    String key = kv.key().c_str();
    orchestrator_data[key] = kv.value();
  } 

  String output;
  serializeJson(doc, output);
  Serial.print(">> Send Custom Data <<");Serial.println(output);
  publish(postDataTopic, output, false, NULL);
}


void LiveObjects::onMqttCallback(char* topic, uint8_t * payload, unsigned int len) 
{
	Serial.print("Message arrived :");
	Serial.println((char*)payload);
  
  DynamicJsonDocument doc(255);
  deserializeJson(doc, (char*)payload);

  // Request: {"req":"eff","arg":{"state": true}}
  // Request: {"req":"reset","arg":{"state": true}}
  // Request: {"req":"power_off","arg":{"state": true}}

  //Orchestrator commands
  // Request Orchestrator status: {"req":"orchestrator","arg":{"command": false}}

  int cid = doc["cid"];
  String reqType = doc["req"];
  JsonObject arg = doc["arg"];

  if(reqType.equals("orchestrator")){
    Serial.println("***** ORCHESTRATOR COMMAND ****");

    bool command = arg["command"].as<bool>();
    orchestratorCom.manageLiveObjectsCommand(command, arg);    
    String response = "{\"res\":{\"done\":true},\"cid\":" + String(cid) + "}";
    publish(cmdTopicRes, response, false, NULL);
    return;
  }
    
  bool state = arg["state"];

  String response = "{\"res\":{\"done\":true},\"cid\":" + String(cid) + "}";
  publish(cmdTopicRes, response, false, NULL);
  if(parent != NULL)
  {
    if((reqType == "reset") && (state)) 
      NVIC_SystemReset();
    else if(reqType == "eff")
      ((ApplicationManager*)parent)->onChangeStateBattery(state);
    else if(reqType == "power_off")
    {
      ((ApplicationManager*)parent)->onChangePowerVOut(state);   
      delay(2000);
      sendData(true);   
    }
  }
  else
  {
    Serial.print("***** Parent is NULL ****");
  }
  
}
