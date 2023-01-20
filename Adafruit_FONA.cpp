/***************************************************
  This is a library for our Adafruit FONA Cellular Module

  Designed specifically to work with the Adafruit FONA
  ----> http://www.adafruit.com/products/1946
  ----> http://www.adafruit.com/products/1963

  These displays use TTL Serial to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/
    // next line per http://postwarrior.com/arduino-ethershield-error-prog_char-does-not-name-a-type/

#include "Adafruit_FONA.h"
#include "config.h"

Adafruit_FONA::Adafruit_FONA()
{
  strcpy(apn, "\"wsiot\"");
  mySerial = 0;
  httpsredirect = false;
  
  strcpy(useragent, "FONA");
}


void Adafruit_FONA::powerOn(bool on)
{
  if(on)
  {
    digitalWrite(TURN_ON_OFF, LOW);
    delay(200);
    digitalWrite(TURN_ON_OFF, HIGH);
    delay(100);
    digitalWrite(TURN_ON_OFF, LOW);
    delay(64);
  }
  else
  {
    digitalWrite(TURN_ON_OFF, HIGH);
    delay(2000);
    digitalWrite(TURN_ON_OFF, LOW);
  }
}

void Adafruit_FONA::reset()
{	
/*	digitalWrite(RESET_PIN, LOW);
	delay(10);
	digitalWrite(RESET_PIN, HIGH);
	delay(500);*/
}

bool Adafruit_FONA::isHeracles(void) {
  return (type == HERACLES_V2);
}


boolean Adafruit_FONA::begin(Stream &port) {
  pinMode(POWER_GSM, OUTPUT);
  digitalWrite(POWER_GSM, HIGH);
  
  pinMode(TURN_ON_OFF, OUTPUT);  
  digitalWrite(TURN_ON_OFF, LOW);
	
  mySerial = &port;
}

boolean Adafruit_FONA::start() {
	DEBUG_PRINTLN(F("Attempting to open comm with ATs"));

	// give 7 seconds to reboot
	int16_t timeout = 7000;
	bool finish = false;
  
	while ((!finish) && (timeout > 0)) 
	{
		while (mySerial->available()) mySerial->read();
		   
		if (sendAtCommand("AT", "OK"))
		{
		  finish = true;
		}

		while (mySerial->available()) mySerial->read();

		if (sendAtCommand("AT", "AT"))
		{
		  finish = true;
		}

		delay(500);
		timeout-=500;
	}

	if (timeout <= 0) 
	{
		#ifdef ADAFRUIT_FONA_DEBUG
			DEBUG_PRINTLN(F("Timeout: No response to AT... last ditch attempt."));
		#endif

		DEBUG_PRINTLN(F("No time out"));
		sendAtCommand("AT", "OK");
    
		delay(100);
		sendAtCommand("AT", "OK");
		delay(100);
		sendAtCommand("AT", "OK");
		delay(100);
	}

	// turn off Echo!
	DEBUG_PRINTLN(F("turn off Echo!"));
	sendAtCommand("ATE0", "OK");
	delay(100);

	if (! sendAtCommand("ATE0", "OK")) {
	//return false;
	}

	// turn on hangupitude
	sendAtCommand("AT+CVHU=0", "OK");

	delay(100);	
  flushMessage();
  
	DEBUG_PRINT(F("\t---> ")); DEBUG_PRINTLN("ATI");

	mySerial->println("ATI");
	getResponse(500, true);

	DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);

	type = UNKNOWN;
	if(strstr(replybuffer, "Heracles 224G R1529") != 0) 
	{
		type = HERACLES_V2;
	} 	
  
	return true;
}


/********* Serial port ********************************************/
boolean Adafruit_FONA::setBaudrate(uint16_t baud) {
  return sendAtCommand("AT+IPREX=", (int32_t*)&baud, "OK");
}

/********* Real Time Clock ********************************************/

boolean Adafruit_FONA::readRTC(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *hr, uint8_t *min, uint8_t *sec) {
  uint16_t v;
  sendParseReply("AT+CCLK?", "+CCLK: ", &v, '/', 0);
  *year = v;

  DEBUG_PRINTLN(*year);
}

boolean Adafruit_FONA::enableRTC(uint8_t i) {
  if (! sendAtCommand("AT+CLTS=", (int32_t*)&i, "OK"))
    return false;
  return sendAtCommand("AT&W", "OK");
}


/********* BATTERY & ADC ********************************************/

/* returns value in mV (uint16_t) */
boolean Adafruit_FONA::getBattVoltage(uint16_t *v) {
  return sendParseReply("AT+CBC", "+CBC: ", v, ',', 2);
}

/* returns the percentage charge of battery as reported by sim800 */
boolean Adafruit_FONA::getBattPercent(uint16_t *p) {
  return sendParseReply("AT+CBC", "+CBC: ", p, ',', 1);
}

boolean Adafruit_FONA::getADCVoltage(uint16_t *v) {
  return sendParseReply("AT+CADC?", "+CADC: 1,", v);
}

/********* SIM ***********************************************************/
uint8_t Adafruit_FONA::unlockSIM(char *pin)
{
  char sendbuff[14] = "AT+CPIN=";
  sendbuff[8] = pin[0];
  sendbuff[9] = pin[1];
  sendbuff[10] = pin[2];
  sendbuff[11] = pin[3];
  sendbuff[12] = '\0';

  return sendAtCommand(sendbuff, "OK");
}

uint8_t Adafruit_FONA::getSIMCCID(char *ccid) 
{
  sendAtCommand((char*)"AT+CCID");
  // up to 28 chars for reply, 20 char total ccid
  if (replybuffer[0] == '+') {
    // fona 3g?
    strncpy(ccid, replybuffer+8, 20);
  } else {
    // fona 800 or 800
    strncpy(ccid, replybuffer, 20);
  }
  ccid[20] = 0;

  getResponse(); // eat 'OK'
  return strlen(ccid);
}

uint8_t Adafruit_FONA::getSIMIMSI(char *imsi) 
{
  sendAtCommand("AT+CIMI");
  // up to 28 chars for reply, 20 char total ccid
  /*if (replybuffer[0] == '+') {
    // fona 3g?
    strncpy(imsi, replybuffer+8, 20);
  } else {
    // fona 800 or 800
    strncpy(imsi, replybuffer, 20);
  }*/
  strncpy(imsi, replybuffer, 20);
  imsi[20] = 0;

  getResponse(); // eat 'OK'

  return strlen(imsi);
}

/********* IMEI **********************************************************/

uint8_t Adafruit_FONA::getIMEI(char *imei) 
{
  sendAtCommand("AT+GSN");

  // up to 15 chars
  strncpy(imei, replybuffer, 15);
  imei[15] = 0;

  getResponse(); // eat 'OK'

  return strlen(imei);
}


/********* NETWORK *******************************************************/

uint8_t Adafruit_FONA::getNetworkStatus(void) {
  uint16_t status;

  if (! sendParseReply("AT+CREG?", "+CREG: ", &status, ',', 1)) return 0;

  return status;
}


uint8_t Adafruit_FONA::getRSSI(void) {
  uint16_t reply;

  if (! sendParseReply("AT+CSQ", "+CSQ: ", &reply) ) return 0;

  return reply;
}

uint16_t Adafruit_FONA::getPreferredSelection()
{
    uint16_t reply;
    if (! sendParseReply("AT+CNMP?", "+CNMP: ", &reply) ) return 0;
    
    return reply;
}

boolean Adafruit_FONA::setPreferredSelection(int32_t a) {
    // 2 Automatic, 13 GSM Only, 38 LTE Only, 51 GSM and LTE Only
   if((a != 2) && (a != 13) && (a != 38) && (a != 51)) return false;    
   return sendAtCommand("AT+CNMP=", (int32_t*)&a, "OK");
}


/********* SMS **********************************************************/

uint8_t Adafruit_FONA::getSMSInterrupt(void) {
  uint16_t reply;

  if (! sendParseReply("AT+CFGRI?", "+CFGRI: ", &reply) ) return 0;

  return reply;
}

boolean Adafruit_FONA::setSMSInterrupt(uint8_t i) {
  return sendAtCommand("AT+CFGRI=", (int32_t*)&i, "OK");
}

int8_t Adafruit_FONA::getNumSMS(void) {
  uint16_t numsms;

  // get into text mode
  if (! sendAtCommand("AT+CMGF=1", "OK")) return -1;

  // ask how many sms are stored
  if (sendParseReply("AT+CPMS?", FONA_PREF_SMS_STORAGE ",", &numsms))
    return numsms;
  if (sendParseReply("AT+CPMS?", "\"SM\",", &numsms))
    return numsms;
  if (sendParseReply("AT+CPMS?", "\"SM_P\",", &numsms))
    return numsms;
  return -1;
}

// Reading SMS's is a bit involved so we don't use helpers that may cause delays or debug
// printouts!
boolean Adafruit_FONA::readSMS(uint8_t i, char *smsbuff,
			       uint16_t maxlen, uint16_t *readlen) {
  // text mode
  if (! sendAtCommand("AT+CMGF=1", "OK")) return false;

  // show all text mode parameters
  if (! sendAtCommand("AT+CSDH=1", "OK")) return false;

  // parse out the SMS len
  uint16_t thesmslen = 0;


  DEBUG_PRINT(F("AT+CMGR="));
  DEBUG_PRINTLN(i);


  //sendAtCommand("AT+CMGR="), i, 1000);  //  do not print debug!
  mySerial->print(F("AT+CMGR="));
  mySerial->println(i);
  getResponse(1000); // timeout

  //DEBUG_PRINT(F("Reply: ")); DEBUG_PRINTLN(replybuffer);
  // parse it out...

  DEBUG_PRINTLN(replybuffer);

  if (! parseReply("+CMGR:", &thesmslen, ',', 11)) {
    *readlen = 0;
    return false;
  }

  readRaw(thesmslen);

  flushMessage();

  uint16_t thelen = min(maxlen, (uint16_t)strlen(replybuffer));
  strncpy(smsbuff, replybuffer, thelen);
  smsbuff[thelen] = 0; // end the string


  DEBUG_PRINTLN(replybuffer);

  *readlen = thelen;
  return true;
}

// Retrieve the sender of the specified SMS message and copy it as a string to
// the sender buffer.  Up to senderlen characters of the sender will be copied
// and a null terminator will be added if less than senderlen charactesr are
// copied to the result.  Returns true if a result was successfully retrieved,
// otherwise false.

boolean Adafruit_FONA::getSMSSender(uint8_t i, char *sender, int senderlen) {
  // Ensure text mode and all text mode parameters are sent.
  if (! sendAtCommand("AT+CMGF=1", "OK")) return false;
  if (! sendAtCommand("AT+CSDH=1", "OK")) return false;


  DEBUG_PRINT(F("AT+CMGR="));
  DEBUG_PRINTLN(i);


  // Send command to retrieve SMS message and parse a line of response.
  mySerial->print(F("AT+CMGR="));
  mySerial->println(i);
  getResponse(1000);


  DEBUG_PRINTLN(replybuffer);


  // Parse the second field in the response.
 // -> to do  boolean result = parseReplyQuoted("+CMGR:", sender, senderlen, ',', 1);
  boolean result;
  // Drop any remaining data from the response.
  flushMessage();
  return result;
}

boolean Adafruit_FONA::sendSMS(char *smsaddr, char *smsmsg) {
  if (! sendAtCommand("AT+CMGF=1", "OK")) return false;

  char sendcmd[30] = "AT+CMGS=\"";
  strncpy(sendcmd+9, smsaddr, 30-9-2);  // 9 bytes beginning, 2 bytes for close quote + null
  sendcmd[strlen(sendcmd)] = '\"';

  if (! sendAtCommand(sendcmd, "> ")) return false;

  DEBUG_PRINT(F("> ")); DEBUG_PRINTLN(smsmsg);

  mySerial->println(smsmsg);
  mySerial->println();
  mySerial->write(0x1A);

  DEBUG_PRINTLN("^Z");

  if ( (type == FONA3G_A) || (type == FONA3G_E) ) {
    // Eat two sets of CRLF
    getResponse(200);
    //DEBUG_PRINT("Line 1: "); DEBUG_PRINTLN(strlen(replybuffer));
    getResponse(200);
    //DEBUG_PRINT("Line 2: "); DEBUG_PRINTLN(strlen(replybuffer));
  }
  getResponse(10000); // read the +CMGS reply, wait up to 10 seconds!!!
  //DEBUG_PRINT("Line 3: "); DEBUG_PRINTLN(strlen(replybuffer));
  if (strstr(replybuffer, "+CMGS") == 0) {
    return false;
  }
  getResponse(1000); // read OK
  //DEBUG_PRINT("* "); DEBUG_PRINTLN(replybuffer);

  if (strcmp(replybuffer, "OK") != 0) {
    return false;
  }

  return true;
}

boolean Adafruit_FONA::deleteSMS(uint8_t i) {
    if (! sendAtCommand("AT+CMGF=1", "OK")) return false;
  // read an sms
  char sendbuff[12] = "AT+CMGD=000";
  sendbuff[8] = (i / 100) + '0';
  i %= 100;
  sendbuff[9] = (i / 10) + '0';
  i %= 10;
  sendbuff[10] = i + '0';

  return sendAtCommand(sendbuff, "OK", 2000);
}

/********* USSD *********************************************************/

boolean Adafruit_FONA::sendUSSD(char *ussdmsg, char *ussdbuff, uint16_t maxlen, uint16_t *readlen) {
  if (! sendAtCommand("AT+CUSD=1", "OK")) return false;

  char sendcmd[30] = "AT+CUSD=1,\"";
  strncpy(sendcmd+11, ussdmsg, 30-11-2);  // 11 bytes beginning, 2 bytes for close quote + null
  sendcmd[strlen(sendcmd)] = '\"';

  if (! sendAtCommand(sendcmd, "OK")) {
    *readlen = 0;
    return false;
  } else {
      getResponse(10000); // read the +CUSD reply, wait up to 10 seconds!!!
      //DEBUG_PRINT("* "); DEBUG_PRINTLN(replybuffer);
      char *p = prog_char_strstr(replybuffer, PSTR("+CUSD: "));
      if (p == 0) {
        *readlen = 0;
        return false;
      }
      p+=7; //+CUSD
      // Find " to get start of ussd message.
      p = strchr(p, '\"');
      if (p == 0) {
        *readlen = 0;
        return false;
      }
      p+=1; //"
      // Find " to get end of ussd message.
      char *strend = strchr(p, '\"');

      uint16_t lentocopy = min(maxlen-1, strend - p);
      strncpy(ussdbuff, p, lentocopy+1);
      ussdbuff[lentocopy] = 0;
      *readlen = lentocopy;
  }
  return true;
}


/********* TIME **********************************************************/

boolean Adafruit_FONA::enableNetworkTimeSync(boolean onoff) {
  if (onoff) {
    if (! sendAtCommand("AT+CLTS=1", "OK"))
      return false;
  } else {
    if (! sendAtCommand("AT+CLTS=0", "OK"))
      return false;
  }

  flushMessage(); // eat any 'Unsolicted Result Code'

  return true;
}

boolean Adafruit_FONA::enableNTPTimeSync(boolean onoff, FONAFlashStringPtr ntpserver) {
  if (onoff) {
    if (! sendAtCommand("AT+CNTPCID=1", "OK"))
      return false;

    mySerial->print(F("AT+CNTP=\""));
    if (ntpserver != 0) {
      mySerial->print(ntpserver);
    } else {
      mySerial->print(F("pool.ntp.org"));
    }
    mySerial->println(F("\",0"));
    getResponse(FONA_DEFAULT_TIMEOUT_MS);
    if (strcmp(replybuffer, "OK") != 0)
      return false;

    if (! sendAtCommand("AT+CNTP", "OK", 10000))
      return false;

    uint16_t status;
    getResponse(10000);
    if (! parseReply("+CNTP:", &status))
      return false;
  } else {
    if (! sendAtCommand("AT+CNTPCID=0", "OK"))
      return false;
  }

  return true;
}

boolean Adafruit_FONA::getTime(char *buff, uint16_t maxlen) {
  sendAtCommand("AT+CCLK?", (uint16_t) 10000);
  if (strncmp(replybuffer, "+CCLK: ", 7) != 0)
    return false;

  char *p = replybuffer+7;
  uint16_t lentocopy = min(maxlen-1, (int)strlen(p));
  strncpy(buff, p, lentocopy+1);
  buff[lentocopy] = 0;

  getResponse(); // eat OK

  return true;
}

/********* GPS **********************************************************/


void Adafruit_FONA::getGpsWorkSet() {
  sendAtCommand("AT+CGNSMOD?");

  getResponse(); // eat 'OK'

  //return strlen(imei);
}

boolean Adafruit_FONA::enableGPS(boolean onoff) {
  uint16_t state;

  // first check if its already on or off

  //if (type == FONA808_V2) {
    if (! sendParseReply("AT+CGNSPWR?", "+CGNSPWR: ", &state) )
      return false;
  /*} else {
    if (! sendParseReply(F("AT+CGPSPWR?"), F("+CGPSPWR: "), &state))
      return false;
  }*/

  if (onoff && !state) {
   // if (type == FONA808_V2) {
      if (! sendAtCommand("AT+CGNSPWR=1", "OK"))  // try GNS command
	    return false;
    /*} else {
      if (! sendCheckReply(F("AT+CGPSPWR=1"), "OK"))
	return false;
    }*/
  } else if (!onoff && state) {
    //if (type == FONA808_V2) {
      if (! sendAtCommand("AT+CGNSPWR=0", "OK")) // try GNS command
	return false;
    /*} else {
      if (! sendCheckReply(F("AT+CGPSPWR=0"), "OK"))
	return false;
    }*/
  }
  return true;
}

int8_t Adafruit_FONA::GPSstatus(char* buffer) 
{
  //if (type == FONA808_V2) {
    // 808 V2 uses GNS commands and doesn't have an explicit 2D/3D fix status.
    // Instead just look for a fix and if found assume it's a 3D fix.
    sendAtCommand("AT+CGNSINF");
    char *p = prog_char_strstr(replybuffer, (prog_char*)F("+CGNSINF: "));
    strcpy(buffer, replybuffer);
    if (p == 0) return -1;
    p+=10;
    getResponse(); // eat 'OK'
    if (p[0] == '0') return 0; // GPS is not even on!

    p+=2; // Skip to second value, fix status.
    //DEBUG_PRINTLN(p);
    // Assume if the fix status is '1' then we have a 3D fix, otherwise no fix.
    if (p[0] == '1') return 3;
    else return 1;
  //}
 /* if (type == FONA3G_A || type == FONA3G_E) {
    // FONA 3G doesn't have an explicit 2D/3D fix status.
    // Instead just look for a fix and if found assume it's a 3D fix.
    sendAtCommand("AT+CGPSINFO"));
    char *p = prog_char_strstr(replybuffer, (prog_char*)F("+CGPSINFO:"));
    if (p == 0) return -1;
    if (p[10] != ',') return 3; // if you get anything, its 3D fix
    return 0;
  }
  else {*/
    // 808 V1 looks for specific 2D or 3D fix state.
 /*   sendAtCommand("AT+CGPSSTATUS?"));
    char *p = prog_char_strstr(replybuffer, (prog_char*)F("SSTATUS: Location "));
    if (p == 0) return -1;
    p+=18;
    getResponse(); // eat 'OK'
    //DEBUG_PRINTLN(p);
    if (p[0] == 'U') return 0;
    if (p[0] == 'N') return 1;
    if (p[0] == '2') return 2;
    if (p[0] == '3') return 3;*/
  //}
  // else
  return 0;
}

uint8_t Adafruit_FONA::getType()
{
  return type;
}

int8_t Adafruit_FONA::GPSstatus() 
{
  //if (type == FONA808_V2) {
    // 808 V2 uses GNS commands and doesn't have an explicit 2D/3D fix status.
    // Instead just look for a fix and if found assume it's a 3D fix.
    sendAtCommand("AT+CGNSINF");
    char *p = prog_char_strstr(replybuffer, (prog_char*)F("+CGNSINF: "));
   
    if (p == 0) return -1;
    p+=10;
    getResponse(); // eat 'OK'
    if (p[0] == '0') return 0; // GPS is not even on!

    p+=2; // Skip to second value, fix status.
    //DEBUG_PRINTLN(p);
    // Assume if the fix status is '1' then we have a 3D fix, otherwise no fix.
    if (p[0] == '1') return 3;
    else return 1;
  //}
 /* if (type == FONA3G_A || type == FONA3G_E) {
    // FONA 3G doesn't have an explicit 2D/3D fix status.
    // Instead just look for a fix and if found assume it's a 3D fix.
    sendAtCommand("AT+CGPSINFO"));
    char *p = prog_char_strstr(replybuffer, (prog_char*)F("+CGPSINFO:"));
    if (p == 0) return -1;
    if (p[10] != ',') return 3; // if you get anything, its 3D fix
    return 0;
  }
  else {*/
    // 808 V1 looks for specific 2D or 3D fix state.
 /*   sendAtCommand("AT+CGPSSTATUS?"));
    char *p = prog_char_strstr(replybuffer, (prog_char*)F("SSTATUS: Location "));
    if (p == 0) return -1;
    p+=18;
    getResponse(); // eat 'OK'
    //DEBUG_PRINTLN(p);
    if (p[0] == 'U') return 0;
    if (p[0] == 'N') return 1;
    if (p[0] == '2') return 2;
    if (p[0] == '3') return 3;*/
  //}
  // else
  return 0;
}


uint8_t Adafruit_FONA::getGPS(uint8_t arg, char *buffer, uint8_t maxbuff) {
  int32_t x = arg;

  if ( (type == FONA3G_A) || (type == FONA3G_E) ) {
    sendAtCommand("AT+CGPSINFO");
  } else if (type == FONA808_V1) {
    sendAtCommand("AT+CGPSINF=", x);
  } else {
    sendAtCommand("AT+CGNSINF");
  }

  char *p = prog_char_strstr(replybuffer, (prog_char*)F("SINF"));
  if (p == 0) {
    buffer[0] = 0;
    return 0;
  }

  p+=6;

  uint8_t len = max(maxbuff-1, (int)strlen(p));
  strncpy(buffer, p, len);
  buffer[len] = 0;

  getResponse(); // eat 'OK'
  return len;
}

boolean Adafruit_FONA::getGPS(float *lat, float *lon, float *speed_kph, float *heading, float *altitude) {

  char gpsbuffer[120];

  // we need at least a 2D fix
  if (GPSstatus() < 2)
    return false;

  // grab the mode 2^5 gps csv from the sim808
  uint8_t res_len = getGPS(32, gpsbuffer, 120);

  // make sure we have a response
  if (res_len == 0)
    return false;

  if (type == FONA3G_A || type == FONA3G_E) {
    // Parse 3G respose
    // +CGPSINFO:4043.000000,N,07400.000000,W,151015,203802.1,-12.0,0.0,0
    // skip beginning
    char *tok;

   // grab the latitude
    char *latp = strtok(gpsbuffer, ",");
    if (! latp) return false;

    // grab latitude direction
    char *latdir = strtok(NULL, ",");
    if (! latdir) return false;

    // grab longitude
    char *longp = strtok(NULL, ",");
    if (! longp) return false;

    // grab longitude direction
    char *longdir = strtok(NULL, ",");
    if (! longdir) return false;

    // skip date & time
    tok = strtok(NULL, ",");
    tok = strtok(NULL, ",");

   // only grab altitude if needed
    if (altitude != NULL) {
      // grab altitude
      char *altp = strtok(NULL, ",");
      if (! altp) return false;
      *altitude = atof(altp);
    }

    // only grab speed if needed
    if (speed_kph != NULL) {
      // grab the speed in km/h
      char *speedp = strtok(NULL, ",");
      if (! speedp) return false;

      *speed_kph = atof(speedp);
    }

    // only grab heading if needed
    if (heading != NULL) {

      // grab the speed in knots
      char *coursep = strtok(NULL, ",");
      if (! coursep) return false;

      *heading = atof(coursep);
    }

    double latitude = atof(latp);
    double longitude = atof(longp);

    // convert latitude from minutes to decimal
    float degrees = floor(latitude / 100);
    double minutes = latitude - (100 * degrees);
    minutes /= 60;
    degrees += minutes;

    // turn direction into + or -
    if (latdir[0] == 'S') degrees *= -1;

    *lat = degrees;

    // convert longitude from minutes to decimal
    degrees = floor(longitude / 100);
    minutes = longitude - (100 * degrees);
    minutes /= 60;
    degrees += minutes;

    // turn direction into + or -
    if (longdir[0] == 'W') degrees *= -1;

    *lon = degrees;

  } else if (type == FONA808_V2) {
    // Parse 808 V2 response.  See table 2-3 from here for format:
    // http://www.adafruit.com/datasheets/SIM800%20Series_GNSS_Application%20Note%20V1.00.pdf

    // skip GPS run status
    char *tok = strtok(gpsbuffer, ",");
    if (! tok) return false;

    // skip fix status
    tok = strtok(NULL, ",");
    if (! tok) return false;

    // skip date
    tok = strtok(NULL, ",");
    if (! tok) return false;

    // grab the latitude
    char *latp = strtok(NULL, ",");
    if (! latp) return false;

    // grab longitude
    char *longp = strtok(NULL, ",");
    if (! longp) return false;

    *lat = atof(latp);
    *lon = atof(longp);

    // only grab altitude if needed
    if (altitude != NULL) {
      // grab altitude
      char *altp = strtok(NULL, ",");
      if (! altp) return false;

      *altitude = atof(altp);
    }

    // only grab speed if needed
    if (speed_kph != NULL) {
      // grab the speed in km/h
      char *speedp = strtok(NULL, ",");
      if (! speedp) return false;

      *speed_kph = atof(speedp);
    }

    // only grab heading if needed
    if (heading != NULL) {

      // grab the speed in knots
      char *coursep = strtok(NULL, ",");
      if (! coursep) return false;

      *heading = atof(coursep);
    }
  }
  else {
    // Parse 808 V1 response.

    // skip mode
    char *tok = strtok(gpsbuffer, ",");
    if (! tok) return false;

    // skip date
    tok = strtok(NULL, ",");
    if (! tok) return false;

    // skip fix
    tok = strtok(NULL, ",");
    if (! tok) return false;

    // grab the latitude
    char *latp = strtok(NULL, ",");
    if (! latp) return false;

    // grab latitude direction
    char *latdir = strtok(NULL, ",");
    if (! latdir) return false;

    // grab longitude
    char *longp = strtok(NULL, ",");
    if (! longp) return false;

    // grab longitude direction
    char *longdir = strtok(NULL, ",");
    if (! longdir) return false;

    double latitude = atof(latp);
    double longitude = atof(longp);

    // convert latitude from minutes to decimal
    float degrees = floor(latitude / 100);
    double minutes = latitude - (100 * degrees);
    minutes /= 60;
    degrees += minutes;

    // turn direction into + or -
    if (latdir[0] == 'S') degrees *= -1;

    *lat = degrees;

    // convert longitude from minutes to decimal
    degrees = floor(longitude / 100);
    minutes = longitude - (100 * degrees);
    minutes /= 60;
    degrees += minutes;

    // turn direction into + or -
    if (longdir[0] == 'W') degrees *= -1;

    *lon = degrees;

    // only grab speed if needed
    if (speed_kph != NULL) {

      // grab the speed in knots
      char *speedp = strtok(NULL, ",");
      if (! speedp) return false;

      // convert to kph
      *speed_kph = atof(speedp) * 1.852;

    }

    // only grab heading if needed
    if (heading != NULL) {

      // grab the speed in knots
      char *coursep = strtok(NULL, ",");
      if (! coursep) return false;

      *heading = atof(coursep);

    }

    // no need to continue
    if (altitude == NULL)
      return true;

    // we need at least a 3D fix for altitude
    if (GPSstatus() < 3)
      return false;

    // grab the mode 0 gps csv from the sim808
    res_len = getGPS(0, gpsbuffer, 120);

    // make sure we have a response
    if (res_len == 0)
      return false;

    // skip mode
    tok = strtok(gpsbuffer, ",");
    if (! tok) return false;

    // skip lat
    tok = strtok(NULL, ",");
    if (! tok) return false;

    // skip long
    tok = strtok(NULL, ",");
    if (! tok) return false;

    // grab altitude
    char *altp = strtok(NULL, ",");
    if (! altp) return false;

    *altitude = atof(altp);
  }

  return true;

}

boolean Adafruit_FONA::enableGPSNMEA(uint8_t i) {

  char sendbuff[15] = "AT+CGPSOUT=000";
  sendbuff[11] = (i / 100) + '0';
  i %= 100;
  sendbuff[12] = (i / 10) + '0';
  i %= 10;
  sendbuff[13] = i + '0';

  if (type == FONA808_V2) {
    if (i)
      return sendAtCommand("AT+CGNSTST=1", "OK");
    else
      return sendAtCommand("AT+CGNSTST=0", "OK");
  } else {
    return sendAtCommand(sendbuff, "OK", 2000);
  }
}

boolean Adafruit_FONA::enableGPRS(boolean onoff) {

  if (onoff) {
    // disconnect all sockets
    sendAtCommand("AT+CIPSHUT", "SHUT OK", 20000);

	// Deactivate the bearer context
    if (! sendAtCommand("AT+CGATT=0", "OK", 20000))
      return false;
  
    if (! sendAtCommand("AT+CGATT=1", "OK", 20000))
      return false;	


    // set bearer profile! connection type GPRS
    if (! sendAtCommand("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"", "OK", 10000))
      return false;

    // set bearer profile access point name
    if (apn) 
    {
      // Send command AT+SAPBR=3,1,"APN","<apn value>" where <apn value> is the configured APN value.
      if (!sendAtCommand("AT+SAPBR=3,1,\"APN\",", apn, "OK", 10000))
        return false;

      // send AT+CSTT,"apn","user","pass"
      flushMessage();

      mySerial->print(F("AT+CSTT="));
      mySerial->print(apn);
      if (apnusername) {
  mySerial->print(",");
  mySerial->print(apnusername);
      }
      if (apnpassword) {
  mySerial->print(",");
  mySerial->print(apnpassword);
      }
      mySerial->print("\r\n");
      DEBUG_PRINT(F("\t---> ")); DEBUG_PRINT(F("AT+CSTT="));
      DEBUG_PRINT(apn);

      if (apnusername) {
  DEBUG_PRINT(",\"");
  DEBUG_PRINT(apnusername);
      }
      if (apnpassword) {
  DEBUG_PRINT(",\"");
  DEBUG_PRINT(apnpassword);
      }

  /*      mySerial->print(F("AT+CSTT=\""));
      mySerial->print(apn);
      if (apnusername) {
	mySerial->print("\",\"");
	mySerial->print(apnusername);
      }
      if (apnpassword) {
	mySerial->print("\",\"");
	mySerial->print(apnpassword);
      }
      mySerial->println("\"");

      DEBUG_PRINT(F("\t---> ")); DEBUG_PRINT(F("AT+CSTT=\""));
      DEBUG_PRINT(apn);

      if (apnusername) {
	DEBUG_PRINT("\",\"");
	DEBUG_PRINT(apnusername);
      }
      if (apnpassword) {
	DEBUG_PRINT("\",\"");
	DEBUG_PRINT(apnpassword);
      }
      DEBUG_PRINTLN("\"");*/

      if (! expectReply("OK")) return false;

      // set username/password
      if (apnusername) {
        // Send command AT+SAPBR=3,1,"USER","<user>" where <user> is the configured APN username.
       if (! sendAtCommand("AT+SAPBR=3,1,\"USER\",", apnusername, "OK", 10000))
        return false;
       
      }
      if (apnpassword) {
        // Send command AT+SAPBR=3,1,"PWD","<password>" where <password> is the configured APN password.
        if (!sendAtCommand("AT+SAPBR=3,1,\"PWD\",", apnpassword, "OK", 10000))
          return false;          
      }
    }

    // open GPRS context
    if (! sendAtCommand("AT+SAPBR=1,1", "OK", 30000))
      return false;

    // bring up wireless connection
    if (! sendAtCommand("AT+CIICR", "OK", 10000))
      return false;

  } else {
    // disconnect all sockets
    if (! sendAtCommand("AT+CIPSHUT", "SHUT OK", 20000))
      return false;

    // close GPRS context
    if (! sendAtCommand("AT+SAPBR=0,1", "OK", 10000))
      return false;

    if (! sendAtCommand("AT+CGATT=0", "OK", 10000))
      return false;

  }
  return true;
}

uint8_t Adafruit_FONA::GPRSstate(void) {
  uint16_t state;

  if (! sendParseReply("AT+CGATT?", "+CGATT: ", &state) )
    return -1;

  return state;
}

uint16_t Adafruit_FONA::getMode(void)
{
  uint16_t state;
  if (! sendParseReply("AT+CNSMOD?", "+CNSMOD: ", &state) )
  return -1;

  return state;
  
}

/*void Adafruit_FONA::copyAndAddQuote(char* dest, char* src)
{
  sprintf(
  dest[0] = '"';
  dest++;
  strcpy(dest, src);
  uint16_t len = strlen(this->apn);  
  this->apn[len] = '"';
  this->apn[len + 1]   
}*/

void Adafruit_FONA::setGPRSNetworkSettings(char* apn, char* username, char* password) 
{
  sprintf(this->apn, "%c%s%c", '"', apn, '"');
  sprintf(this->apnusername, "%c%s%c", '"', username, '"');
  sprintf(this->apnpassword, "%c%s%c", '"', password, '"');  
//  this->apn[0] = '"';
//  strcpy(this->apn + 1, apn);
//  uint16_t len = strlen(this->apn);  
//  this->apn[len] = '"';
//  this->apn[len + 1]
  
 // strcpy(this->apnusername, username);
  //strcpy(this->apnpassword, password);
}

boolean Adafruit_FONA::getGSMLoc(uint16_t *errorcode, char *buff, uint16_t maxlen) {

  sendAtCommand("AT+CIPGSMLOC=1,1", (uint16_t)10000);

  if (! parseReply("+CIPGSMLOC: ", errorcode))
    return false;

  char *p = replybuffer+14;
  uint16_t lentocopy = min(maxlen-1, (int)strlen(p));
  strncpy(buff, p, lentocopy+1);

  getResponse(); // eat OK

  return true;
}

boolean Adafruit_FONA::getGSMLoc(float *lat, float *lon) {

  uint16_t returncode;
  char gpsbuffer[120];

  // make sure we could get a response
  if (! getGSMLoc(&returncode, gpsbuffer, 120))
    return false;

  // make sure we have a valid return code
  if (returncode != 0)
    return false;

  // +CIPGSMLOC: 0,-74.007729,40.730160,2015/10/15,19:24:55
  // tokenize the gps buffer to locate the lat & long
  char *longp = strtok(gpsbuffer, ",");
  if (! longp) return false;

  char *latp = strtok(NULL, ",");
  if (! latp) return false;

  *lat = atof(latp);
  *lon = atof(longp);

  return true;

}
/********* TCP FUNCTIONS  ************************************/


boolean Adafruit_FONA::TCPconnect(char *server, uint16_t port) {
  flushMessage();

  // close all old connections
  if (! sendAtCommand("AT+CIPSHUT", "SHUT OK", 20000) ) return false;

  // single connection at a time
  if (! sendAtCommand("AT+CIPMUX=0", "OK") ) return false;

  // manually read data
  if (! sendAtCommand("AT+CIPRXGET=1", "OK") ) return false;


  DEBUG_PRINT(F("AT+CIPSTART=\"TCP\",\""));
  DEBUG_PRINT(server);
  DEBUG_PRINT(F("\",\""));
  DEBUG_PRINT(port);
  DEBUG_PRINTLN(F("\""));


  mySerial->print("AT+CIPSTART=\"TCP\",\"");
  mySerial->print(server);
  mySerial->print("\",\"");
  mySerial->print(port);
  mySerial->println("\"");

  if (! expectReply("OK")) return false;
  if (! expectReply("CONNECT OK")) return false;

  // looks like it was a success (?)
  return true;
}

boolean Adafruit_FONA::TCPclose(void) {
  return sendAtCommand("AT+CIPCLOSE", "OK");
}

boolean Adafruit_FONA::TCPconnected(void) {
  if (! sendAtCommand("AT+CIPSTATUS", "OK", 100) ) return false;
  getResponse(100);
  // DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);

  return (strcmp(replybuffer, "STATE: CONNECT OK") == 0);
}

uint16_t Adafruit_FONA::TCPsend(char *packet, uint8_t len) {

  DEBUG_PRINT(F("AT+CIPSEND="));
  DEBUG_PRINTLN(len);
#ifdef ADAFRUIT_FONA_DEBUG
  //for (uint16_t i=0; i<len; i++) {
  //DEBUG_PRINT(F(" 0x"));
  //DEBUG_PRINT(packet[i], HEX);
  //}
#endif
  DEBUG_PRINTLN();


  mySerial->print("AT+CIPSEND=");
  mySerial->println(len);
  getResponse();

  DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);

  if (replybuffer[0] != '>') return false;

  mySerial->write(packet, len);
  getResponse(3000); // wait up to 3 seconds to send the data

  DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);

  return (strcmp(replybuffer, "SEND OK") == 0) ? len : 0;
  //return (strcmp(replybuffer, "SEND OK") == 0);
}

uint16_t Adafruit_FONA::TCPavailable(void) {
  uint16_t avail;

  if (! sendParseReply("AT+CIPRXGET=4", "+CIPRXGET: 4,", &avail, ',', 0) ) return false;


  //DEBUG_PRINT (avail); DEBUG_PRINTLN(F(" bytes available"));
  return avail;
}


uint16_t Adafruit_FONA::TCPread(uint8_t *buff, uint8_t len) {
  uint16_t avail;

  mySerial->print(F("AT+CIPRXGET=2,"));
  mySerial->println(len);
  getResponse();
  if (! parseReply("+CIPRXGET: 2,", &avail, ',', 0)) return false;

  readRaw(avail);

/*#ifdef ADAFRUIT_FONA_DEBUG
  DEBUG_PRINT (avail); DEBUG_PRINTLN(F(" bytes read"));
  for (uint8_t i=0;i<avail;i++) {
  DEBUG_PRINT(F(" 0x")); DEBUG_PRINT(replybuffer[i], HEX);
  }
  DEBUG_PRINTLN();
#endif*/

  memcpy(buff, replybuffer, avail);

  return avail;
}

uint16_t Adafruit_FONA::TCPread() {
    uint8_t c;
    if (TCPread(&c, 1) == 1) {
      return c;
    }
    return -1;
  }



/********* HTTP LOW LEVEL FUNCTIONS  ************************************/

boolean Adafruit_FONA::HTTP_init() {
  return sendAtCommand("AT+HTTPINIT", "OK");
}

boolean Adafruit_FONA::HTTP_term() {
  return sendAtCommand("AT+HTTPTERM", "OK");
}

void Adafruit_FONA::HTTP_para_start(FONAFlashStringPtr parameter,
                                    boolean quoted) {
  flushMessage();


  DEBUG_PRINT(F("\t---> "));
  DEBUG_PRINT(F("AT+HTTPPARA=\""));
  DEBUG_PRINT(parameter);
  DEBUG_PRINTLN('"');


  mySerial->print("AT+HTTPPARA=\"");
  mySerial->print(parameter);
  if (quoted)
    mySerial->print("\",\"");
  else
    mySerial->print("\",");
}

boolean Adafruit_FONA::HTTP_para_end(boolean quoted) {
  if (quoted)
    mySerial->println('"');
  else
    mySerial->println();

  return expectReply("OK");
}

boolean Adafruit_FONA::HTTP_para(FONAFlashStringPtr parameter,
                                 const char *value) {
  HTTP_para_start(parameter, true);
  mySerial->print(value);
  return HTTP_para_end(true);
}

boolean Adafruit_FONA::HTTP_para(FONAFlashStringPtr parameter,
                                 FONAFlashStringPtr value) {
  HTTP_para_start(parameter, true);
  mySerial->print(value);
  return HTTP_para_end(true);
}

boolean Adafruit_FONA::HTTP_para(FONAFlashStringPtr parameter,
                                 int32_t value) {
  HTTP_para_start(parameter, false);
  mySerial->print(value);
  return HTTP_para_end(false);
}

boolean Adafruit_FONA::HTTP_data(uint32_t size, uint32_t maxTime) {
  flushMessage();


  DEBUG_PRINT(F("\t---> "));
  DEBUG_PRINT(F("AT+HTTPDATA="));
  DEBUG_PRINT(size);
  DEBUG_PRINT(',');
  DEBUG_PRINTLN(maxTime);


  mySerial->print("AT+HTTPDATA=");
  mySerial->print(size);
  mySerial->print(",");
  mySerial->println(maxTime);

  return expectReply("DOWNLOAD");
}

boolean Adafruit_FONA::HTTP_action(uint8_t method, uint16_t *status,
                                   uint16_t *datalen, int32_t timeout) {
  // Send request.
  if (! sendAtCommand("AT+HTTPACTION=", (int32_t*)&method, "OK"))
    return false;

  // Parse response status and size.
  getResponse(timeout);
  if (!parseReply("+HTTPACTION:", status, ',', 1))
    return false;
  if (!parseReply("+HTTPACTION:", datalen, ',', 2))
    return false;

  return true;
}

boolean Adafruit_FONA::HTTP_readall(uint16_t *datalen) 
{
  sendAtCommand("AT+HTTPREAD");
  if (! parseReply("+HTTPREAD:", datalen, ',', 0))
    return false;

  return true;
}

boolean Adafruit_FONA::HTTP_ssl(boolean onoff) {
  uint8_t value = onoff ? 1 : 0;
  return sendAtCommand("AT+HTTPSSL=", (int32_t*)&value, "OK");
}

/********* HTTP HIGH LEVEL FUNCTIONS ***************************/

boolean Adafruit_FONA::HTTP_GET_start(char *url,
              uint16_t *status, uint16_t *datalen){
  if (! HTTP_setup(url))
    return false;

  // HTTP GET
  if (! HTTP_action(FONA_HTTP_GET, status, datalen, 30000))
    return false;

  DEBUG_PRINT(F("Status: ")); DEBUG_PRINTLN(*status);
  DEBUG_PRINT(F("Len: ")); DEBUG_PRINTLN(*datalen);

  // HTTP response data
  if (! HTTP_readall(datalen))
    return false;

  return true;
}

void Adafruit_FONA::HTTP_GET_end(void) {
  HTTP_term();
}

boolean Adafruit_FONA::HTTP_POST_start(char *url,
              FONAFlashStringPtr contenttype,
              const uint8_t *postdata, uint16_t postdatalen,
              uint16_t *status, uint16_t *datalen){
  if (! HTTP_setup(url))
    return false;

  if (! HTTP_para(F("CONTENT"), contenttype)) {
    return false;
  }

  // HTTP POST data
  if (! HTTP_data(postdatalen, 10000))
    return false;
  mySerial->write(postdata, postdatalen);
  if (! expectReply("OK"))
    return false;

  // HTTP POST
  if (! HTTP_action(FONA_HTTP_POST, status, datalen))
    return false;

  DEBUG_PRINT(F("Status: ")); DEBUG_PRINTLN(*status);
  DEBUG_PRINT(F("Len: ")); DEBUG_PRINTLN(*datalen);

  // HTTP response data
  if (! HTTP_readall(datalen))
    return false;

  return true;
}

void Adafruit_FONA::HTTP_POST_end(void) {
  HTTP_term();
}

void Adafruit_FONA::setUserAgent(char* useragent) {
  strcpy(this->useragent, useragent);
}

void Adafruit_FONA::setHTTPSRedirect(boolean onoff) {
  httpsredirect = onoff;
}

/********* HTTP HELPERS ****************************************/

boolean Adafruit_FONA::HTTP_setup(char *url) {
  // Handle any pending
  HTTP_term();

  // Initialize and set parameters
  if (! HTTP_init())
    return false;
  if (! HTTP_para(F("CID"), 1))
    return false;
  if (! HTTP_para(F("UA"), useragent))
    return false;
  if (! HTTP_para(F("URL"), url))
    return false;

  // HTTPS redirect
  if (httpsredirect) {
    if (! HTTP_para(F("REDIR"),1))
      return false;

    if (! HTTP_ssl(true))
      return false;
  }

  return true;
}

/********* HELPERS *********************************************/

boolean Adafruit_FONA::expectReply(char* reply,
                                   uint16_t timeout) {
  getResponse(timeout);

  DEBUG_PRINT(F("\t<--- ")); DEBUG_PRINTLN(replybuffer);

  return (prog_char_strcmp(replybuffer, (prog_char*)reply) == 0);
}

/********* LOW LEVEL *******************************************/

inline int Adafruit_FONA::available(void) {
  return mySerial->available();
}

inline size_t Adafruit_FONA::write(uint8_t x) {
  return mySerial->write(x);
}

inline int Adafruit_FONA::read(void) {
  return mySerial->read();
}

inline int Adafruit_FONA::peek(void) {
  return mySerial->peek();
}

inline void Adafruit_FONA::flush() {
  mySerial->flush();
}

void Adafruit_FONA::flushInput() {
    // Read all available serial input to flush pending data.
    uint16_t timeoutloop = 0;
    while (timeoutloop++ < 40) {
        while(mySerial->available()) {
            mySerial->read();
            timeoutloop = 0;  // If char was received reset the timer
        }
        delay(1);
    }
}

uint16_t Adafruit_FONA::readRaw(uint16_t b) {
  uint16_t idx = 0;

  while (b && (idx < sizeof(replybuffer)-1)) {
    if (mySerial->available()) {
      replybuffer[idx] = mySerial->read();
      idx++;
      b--;
    }
  }
  replybuffer[idx] = 0;

  return idx;
}

bool Adafruit_FONA::parseReply(char* toreply, uint16_t *v, char divider, uint8_t index) 
{
  char *p = strstr(replybuffer, toreply);  // get the pointer to the voltage
  if (p == 0) return false;
  
  p += strlen(toreply);
  //DEBUG_PRINTLN(p);
  for (uint8_t i=0; i<index;i++) 
  {
    // increment dividers
    p = strchr(p, divider);
    if (!p) return false;
    p++;
    //DEBUG_PRINTLN(p);
  }
  *v = atoi(p);

  return true;
}

boolean Adafruit_FONA::parseReply(char* toreply, char *v, char divider, uint8_t index) 
{
  uint8_t i=0;
  char *p = prog_char_strstr(replybuffer, (prog_char*)toreply);
  if (p == 0) return false;
  p+=prog_char_strlen((prog_char*)toreply);

  for (i=0; i<index;i++) {
    // increment dividers
    p = strchr(p, divider);
    if (!p) return false;
    p++;
  }

  for(i=0; i<strlen(p);i++) {
    if(p[i] == divider)
      break;
    v[i] = p[i];
  }

  v[i] = '\0';

  return true;
}

// Parse a quoted string in the response fields and copy its value (without quotes)
// to the specified character array (v).  Only up to maxlen characters are copied
// into the result buffer, so make sure to pass a large enough buffer to handle the
// response.
boolean Adafruit_FONA::parseReplyQuoted(char* toreply,  char *v, int maxlen, char divider, uint8_t index) {
  uint8_t i=0, j;
  // Verify response starts with toreply.
  char *p = prog_char_strstr(replybuffer, (prog_char*)toreply);
  if (p == 0) return false;
  p+=prog_char_strlen((prog_char*)toreply);

  // Find location of desired response field.
  for (i=0; i<index;i++) {
    // increment dividers
    p = strchr(p, divider);
    if (!p) return false;
    p++;
  }

  // Copy characters from response field into result string.
  for(i=0, j=0; j<maxlen && i<strlen(p); ++i) {
    // Stop if a divier is found.
    if(p[i] == divider)
      break;
    // Skip any quotation marks.
    else if(p[i] == '"')
      continue;
    v[j++] = p[i];
  }

  // Add a null terminator if result string buffer was not filled.
  if (j < maxlen)
    v[j] = '\0';

  return true;
}

boolean Adafruit_FONA::sendParseReply(char* tosend, char* toreply, uint16_t *v, char divider, uint8_t index) 
{
  sendAtCommand(tosend);

  if (! parseReply(toreply, v, divider, index)) return false;

  getResponse(); // eat 'OK'

  return true;
}


// Code Halim

uint8_t Adafruit_FONA::sendAtCommand(char *cmd, uint16_t timeout) 
{
  flushMessage();

  DEBUG_PRINT(F("\t---> ")); DEBUG_PRINTLN(cmd);
  mySerial->println(cmd);

  uint8_t len = getResponse(timeout);

  DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);

  return len;
}

bool Adafruit_FONA::sendAtCommand(char *cmd, int32_t* param, char *reply, uint16_t timeout) 
{
  flushMessage();

  DEBUG_PRINT(F("\t---> ")); DEBUG_PRINT(*param); DEBUG_PRINTLN(*param, DEC);

  mySerial->print(cmd);
  mySerial->println(*param, DEC);

  uint8_t len = getResponse(timeout);

  DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);

  return (strcmp(replybuffer, reply) == 0);  
}

bool Adafruit_FONA::sendAtCommand(char *cmd, char* param, char *reply, uint16_t timeout) 
{
  flushMessage();

  DEBUG_PRINT(F("\t---> ")); DEBUG_PRINT(param); 

  mySerial->print(cmd);
  mySerial->println(param);

  uint8_t len = getResponse(timeout);

  DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);

  return (strcmp(replybuffer, reply) == 0);  
}

void Adafruit_FONA::flushMessage() 
{
  // Read all available serial input to flush pending data.
  uint16_t timeoutloop = 0;
  while (timeoutloop++ < 20) 
  {
    while(mySerial->available()) 
    {
      mySerial->read();
      timeoutloop = 0;
    }
    delay(1);
  }
}

uint8_t Adafruit_FONA::getResponse(uint16_t timeout, bool multiline) 
{
  uint16_t index = 0;
 
  bool finish = false;
  bool first = true;
  
  while((!finish) && (timeout > 0) && (index < 255))
  {    
    while((!finish) && (mySerial->available()))    
    {
      char c = mySerial->read();      
      bool setValue = !((c == '\r') || ((c == '\n') && (first || !multiline)));
//      DEBUG_PRINT("valBak="); DEBUG_PRINTLN(c, HEX);
      if(setValue)
      {          
          first = false;
          replybuffer[index] = c;      
          index++;
      }
      
      finish = ((!first) && (c == '\n') && (!multiline));
    }
    delay(1);
    timeout--;
  }
  
  replybuffer[index] = '\0';  // null term

  //DEBUG_PRINT ("index="); DEBUG_PRINTLN (index);
  return index;
}

bool Adafruit_FONA::sendAtCommand(char *send, char *reply, uint16_t timeout) 
{ 
  if(!sendAtCommand(send, timeout))
  {
    DEBUG_PRINT ("TIME OUT !!");
    return false;
  }

  /*for (uint8_t i=0; i<strlen(replybuffer); i++) {
  DEBUG_PRINT(replybuffer[i], HEX); DEBUG_PRINT(" ");
  }
  DEBUG_PRINTLN();
  for (uint8_t i=0; i<strlen(reply); i++) {
    DEBUG_PRINT(reply[i], HEX); DEBUG_PRINT(" ");
  }
  DEBUG_PRINTLN();*/
  
  return (strcmp(replybuffer, reply) == 0);
}
