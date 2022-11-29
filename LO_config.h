#ifndef CONFIG_LIVEOBJECTS_H
#define CONFIG_LIVEOBJECTS_H

int PORT_MQTT = 1883;

const long SCAN_TIMEOUT_DELAY    = 60; // scan_delay in second = delay in second / 2
const long SEND_DELAY            = 900000; // cycle send delay in ms 
const long DELAY_ATTEMPT_CONNECTION = 10000; // in ms
const long RESET_DELAY_AFTER_LOST_CONNECTION = 3600000; // in ms

const char* domain    = "liveobjects.orange-business.com";

const char LO_MDP[]   = "100c1646e3454e1db8c535ec47cc2643";   // Compte Nicolas


const char LO_USER[]  = "json+device";
const char * deviceid = "urn:lo:nsid:mqtt:GreenHomeLan-0001";



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
