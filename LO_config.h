#ifndef CONFIG_LIVEOBJECTS_H
#define CONFIG_LIVEOBJECTS_H

int PORT_MQTT = 1883;

const long SCAN_TIMEOUT_DELAY    = 60; // scan_delay in second = delay in second / 2
const long SEND_DELAY            = 900000; // cycle send delay in ms 
const long DELAY_ATTEMPT_CONNECTION = 10000; // in ms
const long RESET_DELAY_AFTER_LOST_CONNECTION = 3600000; // in ms

const char* domain    = "liveobjects.orange-business.com";

const char LO_MDP[]   = "86667833d73c4172acc570ad98d39b39";   // Compte Gregory
//const char LO_MDP[]   = "b38d17399f8c4230918d45f65597b08f";   // Compte Halim
// const char LO_MDP[]  = "3157c56f308d42edbaf26925f553dcc2";

const char LO_USER[]  = "json+device";
const char * deviceid = "urn:lo:nsid:mqtt:SmartCharger-0003";

char apn[] = "wsiot";
char user[] = "";
char pass[] = "";


/*********************************************
*             SUBSCRIBE TOPIC ON LO
*********************************************/
const char* cmdTopicRes           = "dev/cmd/res";
const char* cmdTopic              = "dev/cmd";          // SUBSCRIPTION CMD TOPIC
const char* receiveCfgTopic       = "dev/cfg/upd";      // SUBSCRIPTION CONFIGURATION UPDATE TOPIC
const char* updtRscTopic          = "dev/rsc/upd";      // SUBSCRIPTION OF DEVICE UPDATE RESOURCE TOPIC

/*********************************************
*             PUBLISH TOPIC ON LO
********************************************/

const char* postDataTopic         = "dev/data";         // PUBLICATION DEVICE METERING DATA TOPIC
const char* postCurrentCfgTopic   = "dev/cfg";          // PUBLICATION DEVICE CONFIGURATION TOPIC

const char* cmdResTopic           = "dev/cmd/res";      // PUBLICATION CMD TOPIC ACKNOWLEDGE OF RECEIPT
const char* statusTopic           = "dev/info";         // PUBLICATION DEVICE INFO TOPIC 
const char* sendRscTopic          = "dev/rsc";          // PUBLICATION DEVICE RESOURCE TOPIC
const char* updtResponseRscTopic  = "dev/rsc/upd/res";  // PUBLICATION DEVICE UPDATE RESOURCE TOPIC

#endif
