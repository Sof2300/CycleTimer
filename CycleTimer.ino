/*
 	 Work in progress
		 - compile and test for esp

	Improve
		- more elegant way to save map to file from both wifiman and cycleman

 	 Future dev
 	 	 - change durations according to sensor : different for different intervals or interpolation
 	 	 	 	 	 - use names like sensor-max-2:50, on-duration-2:10, off-duration-2:90
 	 	 	 	 	 - have something intermediary between cyclemanager and cycleactivator that take care of the switch
 	 	 	 	 	 - UI show all the values and which is activated, and the delta of change (if not interpolated)
 	 	 	 	 	 - interpolation creates a problem that current program is not among the list -> then use only -2 -3 -4 etc the default being the current

 	 	 - improved format for duration >60sec

	Hardware
		- esp
		- relay + valve
		- buzzer ?

	Test procedure
		-

	TODO:
		- pbs of time sync
		- relay need reversing
		- physical box to put items inside
 */


#if defined(ARDUINO_ARCH_ESP8266) || defined(ESP8266)
#define ESP8266BUILD
#include <Ticker.h>
#endif

// Supported platforms ESP32BUILD, x86BUILD
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
#define ESP32BUILD
#include <Ticker.h>
#endif

//#include <ArduinoJson.h>	// too complicated to build under x86



#include <mylib.h>    // Arduino IDE don't want to take subfolders without including a file at the root
//#include <sensors/SensorFactory.H>
#include <datastruct/GenString.h>

#include <net/BasicServer.h>

#include <datastruct/DataMap.h>


#define DEFSSID "Bamboo"
#define DEFPASSWORD "gingembre"
#define DEFAULTAPNAME "CycleTimer"

#define DEFMDNS "EspServer"

#define CONFIGFILENAME "/config.json"

#define SWITCH_STATE_NAMEPATH "/data/switch-state"
#define CYCLE_STATE_NAMEPATH "/data/cycle-state"
#define TIMESTAMP_LAST_CYCLE_START_NAMEPATH "/data/last-start"
#define TIMESTAMP_NEXT_CYCLE_START_NAMEPATH "/data/next-start"
//#define TIMESTAMP_FIRST_START_NAMEPATH "/data/first-start"
#define ON_DURATION_NAMEPATH "/data/on-duration-sec"
#define CYCLE_DURATION_NAMEPATH "/data/cycle-duration-sec"
#define RELAY_PIN_NAMEPATH "/data/relay-pin"
#define RELAY_REVERSE_NAMEPATH "/data/reverse"

#define DONT_SAVE_DATA SWITCH_STATE_NAMEPATH, CYCLE_STATE_NAMEPATH, TIMESTAMP_LAST_CYCLE_START_NAMEPATH, TIMESTAMP_NEXT_CYCLE_START_NAMEPATH//, TIMESTAMP_FIRST_START_NAMEPATH
#define DONT_SAVE_CONFIG "/config/boottime", "/config/boottimestamp", "/config/free-space", "/config/ip", "/config/timezone"


void saveDataMap(DataMap *datamap){
	SIMPLEFS.rewriteFile(CONFIGFILENAME, datamap->getJson({DONT_SAVE_DATA,DONT_SAVE_CONFIG},true,true));
	Serial.println(String()+"save to file:"+ datamap->getJson({DONT_SAVE_DATA,DONT_SAVE_CONFIG},true,true));
}

#include "ConfigManager.h"
#include "WifiManager.h"


#include "CycleManager.h"
#include "DataManager.h"

DataMap datamap;
DataManager dataman=DataManager(&datamap);
WifiSubMan wifisubman = WifiSubMan(&datamap);
BasicServer *server;

ConfigManager configman=ConfigManager(&datamap);
bool loginmemory=false;

CycleManager cycleman(&datamap);

// save datamap on user modification


bool usedeepsleep=false;

void setup(){
	Serial.begin(115200);
	Serial.println("CycleTimer v0.2 Setup started");
	//Serial.println(to_string(123456789));
	SIMPLEFS.begin();
	SIMPLEFS.printInfo();

	configman.load();
//	logger= new Logger(String()+LOGGERPATH+LOGGERFILENAME,&datamap, loginmemory);
	Serial.println("");
	wifisubman.connect();
	Serial.println("Wifi connected");

	cycleman.init();

	// create server
	if(datamap.get(MDNSNAMEPATH).length()==0) {datamap.update(MDNSNAMEPATH,DEFMDNS);}
 	server = new BasicServer(datamap.get(MDNSNAMEPATH));
	server->on(std::vector<GenString>({"/switchToStation", "/switchToAP"}), &wifisubman);
	server->on(dataman.getUrlNames(), &dataman);

	server->on(cycleman.getUrlNames(), &cycleman);

	server->begin();
	Serial.println("CycleTimer HTTP server started");
/*
	std::vector<unsigned int> pins={22,21};
	htu=SensorFactory.createSensor("HTU21D", "GreenHouse", pins);
	//htu->setCalibration("Humidity",1/1.25,0);

	// load rules into datamap "/rules"
*/
	datamap.printMap();
//	mappingman.init(&datamap);
//	server->on("/set", &mappingman);
//	server->on("/create", &mappingman);
}

void serverloop();

void loop(){
 	serverloop();

	delay(5);	// best delay for server response
}

void serverloop() {
	long serverts = millis();
	server->yield();// run the server
	if (millis() - serverts > 10) Serial.println(String() + "server too long " + (millis() - serverts));
	wifisubman.run();
}
