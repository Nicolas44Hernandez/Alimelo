#ifndef ADAFRUIT_FONA_H
#define ADAFRUIT_FONA_H

#include "includes/FONAConfig.h"
#include "includes/FONAExtIncludes.h"
#include "includes/platform/FONAPlatform.h"

#define UNKNOWN		0
#define HERACLES_V1	1
#define HERACLES_V2  10
#define FONA800L 	2
#define FONA808_V1 	3
#define FONA808_V2 	4
#define FONA3G_A 	5
#define FONA3G_E 	6
#define FONA800H 	7

// Set the preferred SMS storage.
//   Use "SM" for storage on the SIM.
//   Use "ME" for internal storage on the FONA chip
#define FONA_PREF_SMS_STORAGE "\"SM\""
//#define FONA_PREF_SMS_STORAGE "\"ME\""


#define FONA_DEFAULT_TIMEOUT_MS 500

#define FONA_HTTP_GET   0
#define FONA_HTTP_POST  1
#define FONA_HTTP_HEAD  2

#define FONA_CALL_READY 0
#define FONA_CALL_FAILED 1
#define FONA_CALL_UNKNOWN 2
#define FONA_CALL_RINGING 3
#define FONA_CALL_INPROGRESS 4

#define DEBUG_PRINT(...)    Serial.print(__VA_ARGS__)
#define DEBUG_PRINTLN(...)  Serial.println(__VA_ARGS__)

class Adafruit_FONA : public FONAStreamType 
{
protected:
  FONAStreamType *mySerial;
  uint8_t type;
  
 public:
  Adafruit_FONA();
  
  boolean begin(FONAStreamType &port);
  boolean start();
  
  bool isHeracles();

  void flushMessage();
  
  uint8_t sendAtCommand(char *cmd, uint16_t timeout = FONA_DEFAULT_TIMEOUT_MS);
  bool sendAtCommand(char *send, char *reply, uint16_t timeout = FONA_DEFAULT_TIMEOUT_MS);
  bool sendAtCommand(char *cmd, int32_t* param, char *reply, uint16_t timeout = FONA_DEFAULT_TIMEOUT_MS);
  bool sendAtCommand(char *cmd, char* param, char *reply, uint16_t timeout = FONA_DEFAULT_TIMEOUT_MS);

  uint8_t getResponse(uint16_t timeout  = FONA_DEFAULT_TIMEOUT_MS, bool multiline = false);
 
  void powerOn(bool on = true);
  void reset();
  uint8_t getType();
  // Stream
  int available(void);
  size_t write(uint8_t x);
  int read(void);
  int peek(void);
  void flush();

  // FONA 3G requirements
  boolean setBaudrate(uint16_t baud);
  uint16_t getMode(void);

  // RTC
  boolean enableRTC(uint8_t i);
  boolean readRTC(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *hr, uint8_t *min, uint8_t *sec);

  // Battery and ADC
  boolean getADCVoltage(uint16_t *v);
  boolean getBattPercent(uint16_t *p);
  boolean getBattVoltage(uint16_t *v);

  // SIM query
  uint8_t unlockSIM(char *pin);
  uint8_t getSIMCCID(char *ccid);
  uint8_t getSIMIMSI(char *imsi); 
  uint8_t getNetworkStatus(void);

  // Network
  uint8_t getRSSI(void);
  uint16_t getPreferredSelection();
  boolean setPreferredSelection(int32_t a);

  // IMEI
  uint8_t getIMEI(char *imei);

  // SMS handling
  boolean setSMSInterrupt(uint8_t i);
  uint8_t getSMSInterrupt(void);
  int8_t getNumSMS(void);
  boolean readSMS(uint8_t i, char *smsbuff, uint16_t max, uint16_t *readsize);
  boolean sendSMS(char *smsaddr, char *smsmsg);
  boolean deleteSMS(uint8_t i);
  boolean getSMSSender(uint8_t i, char *sender, int senderlen);
  boolean sendUSSD(char *ussdmsg, char *ussdbuff, uint16_t maxlen, uint16_t *readlen);

  // Time
  boolean enableNetworkTimeSync(boolean onoff);
  boolean enableNTPTimeSync(boolean onoff, FONAFlashStringPtr ntpserver=0);
  boolean getTime(char *buff, uint16_t maxlen);

  // GPRS handling
  boolean enableGPRS(boolean onoff);
  uint8_t GPRSstate(void);
  boolean getGSMLoc(uint16_t *replycode, char *buff, uint16_t maxlen);
  boolean getGSMLoc(float *lat, float *lon);
  void setGPRSNetworkSettings(char* apn, char* username = NULL, char* password = NULL);

  // GPS handling
  void getGpsWorkSet();
  boolean enableGPS(boolean onoff);
  int8_t GPSstatus(char* buffer);
  int8_t GPSstatus();
  uint8_t getGPS(uint8_t arg, char *buffer, uint8_t maxbuff);
  boolean getGPS(float *lat, float *lon, float *speed_kph=0, float *heading=0, float *altitude=0);
  boolean enableGPSNMEA(uint8_t nmea);

  // TCP raw connections
  boolean TCPconnect(char *server, uint16_t port);
  boolean TCPclose(void);
  boolean TCPconnected(void);
  uint16_t TCPsend(char *packet, uint8_t len);
  uint16_t TCPavailable(void);
  uint16_t TCPread(uint8_t *buff, uint8_t len);
  uint16_t TCPread();

  // HTTP low level interface (maps directly to SIM800 commands).
  boolean HTTP_init();
  boolean HTTP_term();
  void HTTP_para_start(FONAFlashStringPtr parameter, boolean quoted = true);
  boolean HTTP_para_end(boolean quoted = true);
  boolean HTTP_para(FONAFlashStringPtr parameter, const char *value);
  boolean HTTP_para(FONAFlashStringPtr parameter, FONAFlashStringPtr value);
  boolean HTTP_para(FONAFlashStringPtr parameter, int32_t value);
  boolean HTTP_data(uint32_t size, uint32_t maxTime=10000);
  boolean HTTP_action(uint8_t method, uint16_t *status, uint16_t *datalen, int32_t timeout = 10000);
  boolean HTTP_readall(uint16_t *datalen);
  boolean HTTP_ssl(boolean onoff);

  // HTTP high level interface (easier to use, less flexible).
  boolean HTTP_GET_start(char *url, uint16_t *status, uint16_t *datalen);
  void HTTP_GET_end(void);
  boolean HTTP_POST_start(char *url, FONAFlashStringPtr contenttype, const uint8_t *postdata, uint16_t postdatalen,  uint16_t *status, uint16_t *datalen);
  void HTTP_POST_end(void);
  void setUserAgent(char* useragent);

  // HTTPS
  void setHTTPSRedirect(boolean onoff);

  // Helper functions to verify responses.
  boolean expectReply(char* reply, uint16_t timeout = 10000);
  
 protected:  
 
  char replybuffer[255];
  char apn[50];
  char apnusername[50];
  char apnpassword[50];
  boolean httpsredirect;
  char useragent[100];
 
  // HTTP helpers
  boolean HTTP_setup(char *url);

  void flushInput();
  uint16_t readRaw(uint16_t b); 

  boolean parseReply(char* toreply, uint16_t *v, char divider  = ',', uint8_t index=0);
  boolean parseReply(char* toreply, char *v, char divider  = ',', uint8_t index=0);
  boolean parseReplyQuoted(char* toreply, char *v, int maxlen, char divider, uint8_t index);
  boolean sendParseReply(char* tosend, char* toreply, uint16_t *v, char divider = ',', uint8_t index=0);
};

#endif
