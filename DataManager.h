#ifndef DATAMANAGER_H
#define DATAMANAGER_H

/*
 	 - what we want :
 	 	 - a sensor that send readings,
 	 	 - a logger that can save them,
 	 	 - a server that can serve the data
 	 	 - a client that can show data, setting, and log
 */

#include <TimeUtil.h>

#define SERVER_DATAGET_URL "/json","x-Date"
#define FREESPACE_KEYTEXT "free-space"



class DataManager: public EventListener {
	DataMap *datamap;
	bool timesynced=false;

public:
	DataManager (DataMap *datamap0): datamap(datamap0){}

	bool isTimesynced(){return timesynced;}
	DataMap *getDataMap(){return datamap;}


	std::vector<GenString> getUrlNames(){
		std::vector<GenString>vect={SERVER_DATAGET_URL};
		return vect;
	}

	bool notify(GenString ename, Event *e){return notify(String(ename.c_str()),e);}
	bool notify(String ename, Event *e){
		if(ename[ename.length()-1]=='/') ename=ename.substring(0,ename.length()-1);
		StringMapEvent *se=0;
		if(e->isClassType("StringMapEvent")) se=(StringMapEvent *)e;
		if(se==0) return false;

		if(ename==String("/json")) {
			//beepjsonnotify();
			updateFreeSpace();
			se->insertValue("response",makeJson().c_str());
			return true;
		}

		if(ename=="x-Date"){	//should be named newtime or timesync and not boottime
	//		beepnotify();
			String str=se->get("x-Date").c_str();
			String tz=se->get("x-TimeZone").c_str();
			uint32_t ts=str.toInt();
			setNowZerotimeNow32(ts);//now32_boottime=str.toInt();
			datamap->update("/config/boottimestamp", String(now32BootTime()));
			Serial.println(String()+"Datamanager updated now32_boottime to "+String(now32BootTime())/*now32BootTimeLocal()*/+" / "+getTimeString(now32BootTime(),true,true,true).c_str());
			datamap->update("/config/boottime",getTimeString(now32BootTime(),true,true,true));
			if(tz.length()>0) {
				setTimeZoneNow32(tz.toInt());
				datamap->update("/config/timezone", tostring(now32TimeZone()).c_str());	// this is not working
			}

			timesynced=true;	//		String time=getTimeString(true,false,false);	//		Serial.println(String()+"DataManager:updated time :"+time.c_str());

			se->insertValue("response","ok");
			datamap->printMap();
			return true;
		}

		return false;
	}

	void updateFreeSpace(){

		datamap->update(String()+"/config/"+FREESPACE_KEYTEXT,String(SIMPLEFS.getFreeSpace())+" bytes ("+(uint32_t)((uint64_t)100*SIMPLEFS.getFreeSpace()/SIMPLEFS.getTotalSpace())+"%)");}

	String makeJson(){
		return datamap->getJson({"/config/password"});
	}
};

#endif
