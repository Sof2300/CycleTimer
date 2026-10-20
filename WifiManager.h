#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <net/WifiMan.h>
#include <datastruct/DataMap.h>

#define MDNSNAMEPATH "/config/mdns"
#define SSIDNAMEPATH "/config/ssid"
#define PWDNAMEPATH "/config/password"
#define APSSIDNAMEPATH "/config/apssid"
#define APPWDNAMEPATH "/config/appassword"
#define MODENAMEPATH "/config/mode"
#define IPNAMEPATH "/config/ip"


class WifiSubMan: public EventListener
{
	DataMap *sconfig;
public:
	WifiSubMan(DataMap *config0){sconfig=config0;}

	bool hasAP(){
		if(WifiMan::getMode()=="AP") return true;
		return false;
	}
	String getMode(){return WifiMan::getMode();}
	void run(){WifiMan::reconnectIfNeeded();}
	bool notify(GenString ename, Event *e){return notify(String(ename.c_str()),e);}
	bool notify(String ename, Event *e){
		StringMapEvent *se=0;
		if(e->isClassType("StringMapEvent")) se=(StringMapEvent *)e;
		if(ename=="/switchToStation" || ename=="/switchToAP") {
			String ssid=se->values["ssid"].c_str(),password=se->values["password"].c_str(),nmdnsname=se->values["mdns"].c_str();
			if(!ssid.length()) return false;
			if(ename=="/switchToStation"){
				sconfig->update(SSIDNAMEPATH, ssid);// save it to config
				sconfig->update(PWDNAMEPATH, password);// save it to config
				bool b=WifiMan::connectStation(ssid.c_str(),password.c_str());	// contains delay rather should be called from the main loop
				if(!b) WifiMan::connectAP(sconfig->get(APSSIDNAMEPATH).c_str(), sconfig->get(APPWDNAMEPATH).c_str());
			} else if(ename=="/switchToAP") {
				sconfig->update(APSSIDNAMEPATH, ssid);// save it to config
				sconfig->update(APPWDNAMEPATH, password);// save it to config
				WifiMan::connectAP(ssid.c_str(),password.c_str());	// contains delay rather should be called from the main loop
			}
			if(nmdnsname!=sconfig->get(MDNSNAMEPATH)) {sconfig->update(MDNSNAMEPATH,nmdnsname);}
			if(getMode()!=sconfig->get(MODENAMEPATH)) {sconfig->update(MODENAMEPATH,getMode());}			// ssid/password got from server are not saved in the data -> save to sconfig			// if ssid not provided, retreive from sconfig			//saveSettings();
			saveDataMap(sconfig);
			return false;
		};
		return false;
	}

	bool connect(){// connect wifi//
		//		Serial.println("WifiSubMan::connect "+sconfig->get("mode"));
		bool connected=false;
		if(sconfig->get(MODENAMEPATH)=="ST" && sconfig->get(SSIDNAMEPATH).length()>0){			//			Serial.println(String()+"WifiSubMan::connect Search config station ssid: "+(*sconfig)["ssid"]);
			if((*sconfig)[SSIDNAMEPATH].length()>0) {				//			Serial.println(String()+"WifiSubMan::connect Found config station ssid: "+(*sconfig)["ssid"]);
				connected=WifiMan::connectStation((*sconfig)[SSIDNAMEPATH].c_str(), (*sconfig)[PWDNAMEPATH].c_str());	// try saved ssid
			} else connected=WifiMan::connectStation(DEFSSID,DEFPASSWORD);	// try my ssid
		}
		if(!connected) {	// try ap
			String APname=DEFAULTAPNAME,APpass="";
			if(sconfig->get(MODENAMEPATH)=="AP" && sconfig->get(APSSIDNAMEPATH).length()>0) {
				APname=sconfig->get(APSSIDNAMEPATH);
				APpass=sconfig->get(APPWDNAMEPATH);
			}
			//			Serial.println(String()+"WifiSubMan::connect AP connect config:"+(*sconfig)["apssid"]+" final apname:"+APname);
			/*			if(sconfig->exists("apssid")){
				Serial.println(String()+"Found saved apssid:"+sconfig->get("apssid")+" / appassword:"+sconfig->get("appassword"));
				APname=sconfig->get("apssid");APpass=sconfig->get("appassword");}	// try saved AP
			 */
			WifiMan::connectAP(APname.c_str(),APpass.c_str());
			if(getMode()!=sconfig->get(MODENAMEPATH)) sconfig->update(MODENAMEPATH,"AP");
		}
		sconfig->update(IPNAMEPATH,WifiMan::getIP());

		return connected;
	}
};

#endif
