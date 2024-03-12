/* 
	ModBusIP Effect
	Anderson Scenic
	20240227
*/

#include <IotWebConf.h>
#include <IotWebConfUsing.h> // This loads aliases for easier class names.
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#ifdef ESP8266
 #include <ESP8266WiFi.h>
#else //ESP32
 #include <WiFi.h>
#endif
#include <ModbusIP_ESP8266.h>

#define STRING_LEN 128
#define NUMBER_LEN 32

// -- Configuration specific key. The value should be modified if config structure was changed.
#define CONFIG_VERSION "deea"

// -- When CONFIG_PIN is pulled to ground on startup, the Thing will use the initial
//      password to buld an AP. (E.g. in case of lost password)
#define CONFIG_PIN 2

// -- Status indicator pin.
//      First it will light up (kept LOW), on Wifi connection it will blink,
//      when connected to the Wifi it will turn off (kept HIGH).
#define STATUS_PIN LED_BUILTIN

// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char thingName[] = "ModBusEffect1";

// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "12345678";

//Modbus Registers Offsets
int COIL1 = 100;
int COIL2 = 101;
int COIL3 = 102;
int COIL4 = 103;
int COIL5 = 104;
int COIL6 = 105;
int COIL7 = 106;
int COIL8 = 107;
//Used Pins
const int coil1Pin = 0; //GPIO0
const int coil2Pin = 1;
const int coil3Pin = 2;
const int coil4Pin = 3;
const int coil5Pin = 4;
const int coil6Pin = 5;
const int coil7Pin = 6;
const int coil8Pin = 7;

char intParamValue1[NUMBER_LEN];
char intParamValue2[NUMBER_LEN];
char intParamValue3[NUMBER_LEN];
char intParamValue4[NUMBER_LEN];
char intParamValue5[NUMBER_LEN];
char intParamValue6[NUMBER_LEN];
char intParamValue7[NUMBER_LEN];
char intParamValue8[NUMBER_LEN];

// -- Method declarations.
void handleRoot();
// -- Callback methods.
void configSaved();
bool formValidator(iotwebconf::WebRequestWrapper* webRequestWrapper);

//ModbusIP object
ModbusIP mb;

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

DNSServer dnsServer;
WebServer server(80);

IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);
// -- You can also use namespace formats e.g.: iotwebconf::TextParameter
IotWebConfNumberParameter intParam1 = IotWebConfNumberParameter("Coil #1", "intParam1", intParamValue1, NUMBER_LEN, "20", "1..100", "min='1' max='100' step='1'");
IotWebConfNumberParameter intParam2 = IotWebConfNumberParameter("Coil #2", "intParam2", intParamValue2, NUMBER_LEN, "20", "1..100", "min='1' max='100' step='1'");
IotWebConfNumberParameter intParam3 = IotWebConfNumberParameter("Coil #3", "intParam3", intParamValue3, NUMBER_LEN, "20", "1..100", "min='1' max='100' step='1'");
IotWebConfNumberParameter intParam4 = IotWebConfNumberParameter("Coil #4", "intParam4", intParamValue4, NUMBER_LEN, "20", "1..100", "min='1' max='100' step='1'");
IotWebConfNumberParameter intParam5 = IotWebConfNumberParameter("Coil #5", "intParam5", intParamValue5, NUMBER_LEN, "20", "1..100", "min='1' max='100' step='1'");
IotWebConfNumberParameter intParam6 = IotWebConfNumberParameter("Coil #6", "intParam6", intParamValue6, NUMBER_LEN, "20", "1..100", "min='1' max='100' step='1'");
IotWebConfNumberParameter intParam7 = IotWebConfNumberParameter("Coil #7", "intParam7", intParamValue7, NUMBER_LEN, "20", "1..100", "min='1' max='100' step='1'");
IotWebConfNumberParameter intParam8 = IotWebConfNumberParameter("Coil #8", "intParam8", intParamValue8, NUMBER_LEN, "20", "1..100", "min='1' max='100' step='1'");
IotWebConfParameterGroup group1 = IotWebConfParameterGroup("group1", "Coil Selection");

void setup()
{
	// IOTWebConf Initialization
	Serial.begin(115200);
  while( !Serial ){continue;}
	Serial.println();
	Serial.println("Starting up...");

  group1.addItem(&intParam1);
  group1.addItem(&intParam2);
  group1.addItem(&intParam3);
  group1.addItem(&intParam4);
  group1.addItem(&intParam5);
  group1.addItem(&intParam6);
  group1.addItem(&intParam7);
  group1.addItem(&intParam8);
  iotWebConf.addParameterGroup(&group1);

	iotWebConf.setStatusPin(STATUS_PIN);
	Serial.println( "setStatusPin completed" );
	iotWebConf.setConfigPin(CONFIG_PIN);
	Serial.println( "setConfigPin completed" );
	iotWebConf.setConfigSavedCallback(&configSaved);
	Serial.println( "setConfigSavedCallback completed" );
	iotWebConf.setFormValidator(&formValidator);
	Serial.println( "setFormValidator completed" );
	iotWebConf.getApTimeoutParameter()->visible = true;
	Serial.println( "getApTimeoutParameter complete" );

	// -- Initializing the configuration.
	iotWebConf.init();
	Serial.println( " IOT init complete" );

	// -- Set up required URL handlers on the web server.
	server.on("/", handleRoot);
	Serial.println( "server.on handleRoot completed" );
  server.on("/config", []{ iotWebConf.handleConfig(); });
  Serial.println( "server.on handleConfig completed" );
  server.onNotFound([](){ iotWebConf.handleNotFound(); });
  Serial.println( "server.on handleNotFound completed" );

	// LCD Initialization skipped
	lcd.init();
	Serial.println( "LCD init completed" );
	// ModBus Initialization
	mb.server();
  delay(5000);
	Serial.println( "mb.server completed" );
	delay(1000);

  COIL1 = intParam1;
  COIL2 = intParam2;
  COIL3 = intParam3;
  COIL4 = intParam4;
  COIL5 = intParam5;
  COIL6 = intParam6;
  COIL7 = intParam7;
  COIL8 = intParam8;

	mb.addCoil(COIL1-1);
  mb.addCoil(COIL2-1);
	mb.addCoil(COIL3-1);
	mb.addCoil(COIL4-1);
	mb.addCoil(COIL5-1);
	mb.addCoil(COIL6-1);
	mb.addCoil(COIL7-1);
	mb.addCoil(COIL8-1);
	Serial.println( "mb.addCoil completed" );

	Serial.println("Ready.");
	Serial.print( "Starting loop in 3." );
	delay(250);
	Serial.print( "." );
	delay(250);
	Serial.print( "." );
	delay(250);
	Serial.print( "." );
	delay(250);
	Serial.print( "2" );	
	delay(250);
	Serial.print( "." );
	delay(250);
	Serial.print( "." );
	delay(250);
	Serial.print( "." );
	delay(250);
	Serial.print( "1" );	
	delay(250);
	Serial.print( "." );
	delay(250);
	Serial.print( "." );
	delay(250);
	Serial.print( "." );
	delay(250);
	Serial.print( "Beginning loop" );
	
}

void loop() 
{
  // -- doLoop should be called as frequently as possible.
  iotWebConf.doLoop();
  ModBusLoop();
}

void ModBusLoop() 
{
   //Call once inside loop() - all magic here
   mb.task();

   //Attach ledPin to LED_COIL register
   digitalWrite(coil1Pin, mb.Coil(COIL1));
   digitalWrite(coil2Pin, mb.Coil(COIL2));
   digitalWrite(coil3Pin, mb.Coil(COIL3));
   digitalWrite(coil4Pin, mb.Coil(COIL4));
   digitalWrite(coil5Pin, mb.Coil(COIL5));
   digitalWrite(coil6Pin, mb.Coil(COIL6));
   digitalWrite(coil7Pin, mb.Coil(COIL7));
   digitalWrite(coil8Pin, mb.Coil(COIL8));
   delay(10);
}

/**
 * Handle web requests to "/" path.
 */
void handleRoot()
{
	// -- Let IotWebConf test and handle captive portal requests.
	if (iotWebConf.handleCaptivePortal())
	{
		// -- Captive portal request were already served.
		return;
	}
	String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
	s += "<title>ModBus Effect</title></head><body>Hello world!";
	s += "<ul>";
	  s += "<li>Relay #1 to Coil: ";
  s += atoi(intParamValue1);
  s += "<li>Relay #2 to Coil: ";
  s += atoi(intParamValue2);  s += "<li>Relay #3 to Coil: ";
  s += atoi(intParamValue3);  s += "<li>Relay #4 to Coil: ";
  s += atoi(intParamValue4);  s += "<li>Relay #5 to Coil: ";
  s += atoi(intParamValue5);  s += "<li>Relay #6 to Coil: ";
  s += atoi(intParamValue6);  s += "<li>Relay #7 to Coil: ";
  s += atoi(intParamValue7);  s += "<li>Relay #8 to Coil: ";
  s += atoi(intParamValue8);
s += "</ul>";
	s += "Go to <a href='config'>configure page</a> to change values.";
	s += "</body></html>\n";

	server.send(200, "text/html", s);
}

void configSaved()
{
  COIL1 = intParam1;
  COIL2 = intParam2;
  COIL3 = intParam3;
  COIL4 = intParam4;
  COIL5 = intParam5;
  COIL6 = intParam6;
  COIL7 = intParam7;
  COIL8 = intParam8;

  mb.addCoil(COIL1-1);
  mb.addCoil(COIL2-1);
	mb.addCoil(COIL3-1);
	mb.addCoil(COIL4-1);
	mb.addCoil(COIL5-1);
	mb.addCoil(COIL6-1);
	mb.addCoil(COIL7-1);
	mb.addCoil(COIL8-1);
	Serial.println("Configuration was updated.");
}

bool formValidator(iotwebconf::WebRequestWrapper* webRequestWrapper)
{
	Serial.println("Validating form.");
	bool valid = true;

	/*
	int l = webRequestWrapper->arg(stringParam.getId()).length();
	if (l < 3)
	{
	stringParam.errorMessage = "Please provide at least 3 characters for this test!";
	valid = false;
	}
	*/
	return valid;
}
