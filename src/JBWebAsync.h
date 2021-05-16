/************************************************************
 * Sources: 
 *  https://randomnerdtutorials.com/esp32-websocket-server-arduino/
 *  https://m1cr0lab-esp32.github.io/remote-control-with-websocket/
 * 
 * 
 * Libdeps: 
 *  ottowinter/AsyncTCP-esphome@^1.1.1
	ottowinter/ESPAsyncWebServer-esphome@^1.2.7
 * *********************************************************/
#ifndef JBWEBASYNCH_H
#define JBWEBASYNCH_H

#include "SPIFFS.h"
#include <WiFi.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "JBUtils.h"
#include "JBjson.h"

// Create AsyncWebServer object on port 80
AsyncWebServer MyAsyncWebServer(80);
AsyncWebSocket MySocketPlugIn("/ws");       // je crois que le "/ws" sert au AsyncWebServer pour identifier le socket à utiliser lorsqu'il reçoit une requete d'un fichier.js (javascript) 
                                            // PAr exemple, dans le index.js, la première ligne servirait à établir ce lien: 
                                            // var gateway = `ws://${window.location.hostname}/ws`; Cela signifie peut-être qu'il pourrait y avoir
                                            // Plusieurs AsyncWebServer, genre 1 par page?... 

#define WiFiLogInTimeOut 10000

typedef struct {
  String NetName; 
  int dbPower;
} NetworkItem;

NetworkItem *NetList=nullptr;

int numberOfNetworks=0;

String sTryThisSSID, sTryThisPW;
bool flag_TryThisWiFi = false;

bool ConnectToWiFi(const char *pSSID=nullptr, const char *pPW=nullptr/*, bool updateSocket=false*/);
void ScanNetworks(void);
void StartSoftAP(const char * SSIDName, char * PW=nullptr);
int CompareNetworksPower (const void * a, const void * b);

String populateWebVars_WiFiList(const String& var);



bool ConnectToWiFi(const char *pSSID, const char *pPW/*, bool updateSocket*/) {
  long Waiting =0;
  long WaitStart=millis();
  long LastCountownSent=0;
  String sSSID, sPW;


  if(!pSSID || strlen(pSSID)<=0){
    JBINIFile INIFile("/config.ini");
    sSSID=INIFile.GetINIFileKey("SSID").c_str();
    sPW=INIFile.GetINIFileKey("PW").c_str();
    INIFile.Close();
  }else{
    sSSID=pSSID;
    sPW=pPW;
  }

  
  //return false;
  /*if(updateSocket){
    MySocketPlugIn.textAll(CreateJSONString("WIFI_NAME",sSSID,JSON_DATA_POS_SINGLE,JSON_DATATYPE_STRING));
  }*/

  if(sSSID.length()>=0){
    Serial lln "ConnectToWiFi() connecting to: " sp sSSID sp sPW;
    WiFi.begin(sSSID.c_str(),sPW.c_str());
  } else{
    Serial lln "Error: Trying to log to empty sSSID";
    return false;
  }
    
  
  // Tant que non connecté ( et que le timeou n'est pas dépassé)
  Waiting=millis();
  while (WiFi.status() != WL_CONNECTED && Waiting-WaitStart<WiFiLogInTimeOut) {
    static int laststatus=0;
    
    // Check si le status a changé
    if(laststatus!=WiFi.status()){ laststatus=WiFi.status(); Serial sp "new status=" << laststatus;}
    
    /*     
    // Check si le USER a CANCELé
    if(updateSocket && !flag_TryThisWiFi){
      return false; // The user canceled the trial (see main.cpp: TryThisWiFi_html_wsHandler())
    }*/

    // Donner signe de vie environ 1 fois par seconde au client et sur le port serie.   
    if((long)((Waiting-WaitStart)/500) != LastCountownSent){
      LastCountownSent=(long)((Waiting-WaitStart)/500);
      Serial.print(".");
      /*
      if(updateSocket){
        MySocketPlugIn.textAll(CreateJSONString("TIME_LEFT",String(WiFiLogInTimeOut-LastCountownSent).c_str(),JSON_DATA_POS_SINGLE,JSON_DATATYPE_NUMBER));
      }*/
    }
    Waiting=millis();
  }

  if(WiFi.status() == WL_CONNECTED){
    Serial lln  "Success: " sp WiFi.localIP().toString().c_str() sp "on " sp WiFi.SSID();
    return true;
  }
  Serial lln "ConnectToWiFi() = Failed to log to: " sp sSSID sp sPW;
  return false;
}


void StartSoftAP(const char * SSIDName, char * PW){
  WiFi.softAPConfig(IPAddress(10,10,10,10),IPAddress(10,10,10,1),IPAddress(255,255,255,0));
  WiFi.softAP(SSIDName,PW);
  Serial lln "IP * address: " sp  WiFi.softAPIP();
}

int CompareNetworksPower (const void * a, const void * b) {
   NetworkItem *NetA = (NetworkItem *)a;
  NetworkItem *NetB = (NetworkItem *)b;

  if (NetA->dbPower > NetB->dbPower)
      return -1;
   else if (NetA->dbPower < NetB->dbPower)
      return 1;
   else
      return 0;
}

void ScanNetworks() {
  numberOfNetworks = WiFi.scanNetworks();
 
  if(numberOfNetworks>0){   // if there are networks available

    if(NetList!=nullptr){   // if the array was already allocated in a previous attempt. Note that this can lead to memory fragmentation. 
      Serial ln <<"Freeing Netlist array pointer!";
      free(NetList);
    }
    NetList = (NetworkItem *) calloc(numberOfNetworks,sizeof(NetworkItem)); // Memory allocation for the NetList NetworkItem array. In some places, there can be too many networks. 
    if(NetList!=nullptr){
      for (int i = 0; i < numberOfNetworks; i++) {
        NetList[i].NetName=WiFi.SSID(i);
        NetList[i].dbPower=WiFi.RSSI(i);
      }
      //sort by power
      qsort (NetList, numberOfNetworks, sizeof(NetworkItem), CompareNetworksPower);

      for (int i = 0; i < numberOfNetworks; i++) {
        Serial ln << NetList[i].NetName << " " << NetList[i].dbPower;
      }
    }
  }
}

String populateWebVars_WiFiList(const String& var){
  String s;
  s="";
  if(var == "WIFI_LIST"){
    if(numberOfNetworks==0){
        s="<p> NO NETWORK FOUND!</p>";
    } else {
      for(int i=0; i<numberOfNetworks;i++){
        s+=nl; 
        s+="<option value=\"";
        s+=NetList[i].NetName;
        s+="\">"+NetList[i].NetName+"</option>";
      }
    }
    s+=nl;
    return s;
  }
  return s;
}
#endif