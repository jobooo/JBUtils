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
#include "C:\Users\jbore\Documents\Electronique\Projets\Vieux Debris\Vieux Debris Async\.pio\libdeps\esp32dev\WebSockets\src\WebSocketsServer.h"
#include "JBUtils.h"
#include "JBESPFiles.h"
#include "JBjson.h"
// V0.1
// Create AsyncWebServer object on port 80
AsyncWebServer MyAsyncWebServer(80);
// Create WebSocket on another port
WebSocketsServer MySocket(81);

#define WiFiLogInTimeOut 20000

typedef struct {
  String NetName;     
  int dbPower;
} NetworkItem;

NetworkItem *NetList=nullptr;

int numberOfNetworks=0;

String sTryThisSSID, sTryThisPW;
bool flag_UserCanceled = false;  

bool ConnectToWiFi(const char *pSSID=nullptr, const char *pPW=nullptr, bool updateSocket=false);
void ScanNetworks(void);
void StartSoftAP(const char * SSIDName, char * PW=nullptr);
int CompareNetworksPower (const void * a, const void * b);

String populateWebVars_WiFiList(const String&  var);


bool ConnectToWiFi(const char *pSSID, const char *pPW, bool updateSocket) {
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

  if(updateSocket){
    
    Serial lln "Socket 1";
    String s=CreateJSONString("WIFI_NAME",sSSID.c_str(),jsp_SINGLE,jst_STRING);
    MySocket.broadcastTXT(s);
    Serial sp s ln;
    s =CreateJSONString("TIME_LEFT",String((WiFiLogInTimeOut/1000)-LastCountownSent).c_str(),jsp_SINGLE,jst_NUMBER);
    MySocket.broadcastTXT(s);
    MySocket.loop();
    Serial sp s ln;
    yield();
  }

  if(sSSID.length()>=0){
    Serial lln "ConnectToWiFi() connecting to: " sp sSSID sp sPW;
    WiFi.disconnect();
    while(WiFi.status()==WL_CONNECTED){// on attend que le status change sinon ça passe trop vite vers le "success!"
      yield(); 
    } 
      
    WiFi.begin(sSSID.c_str(),sPW.c_str());
    Serial lln "Post wifi.begin()";
  } else{
    Serial lln "Error: Trying to log to empty sSSID";
    flag_UserCanceled=false;
    return false;
  }
    
  
  // Tant que non connecté ( et que le timeou n'est pas dépassé)
  Waiting=millis();
  while (WiFi.status() != WL_CONNECTED && Waiting-WaitStart<WiFiLogInTimeOut) {
    static int laststatus=0;
    
    // Check si le status a changé
    if(laststatus!=WiFi.status()){ laststatus=WiFi.status(); Serial sp "new status=" << laststatus;}
    
         
    // Check si le USER a CANCELé
    if(updateSocket) {
      MySocket.loop();

      if(flag_UserCanceled){  // The user canceled the trial (see main.cpp: TryThisWiFi_html_wsHandler())
        Serial lln "Socket 2";
        return false; 
      }
    }

    yield(); // céder le processeur

    // Donner signe de vie environ 2 fois par seconde au client et sur le port serie.   
    if((long)((Waiting-WaitStart)/1000) != LastCountownSent){
      LastCountownSent=(long)((Waiting-WaitStart)/1000);
      Serial.print(".");
      
      if(updateSocket){
        
        String s =CreateJSONString("TIME_LEFT",String((WiFiLogInTimeOut/1000)-LastCountownSent).c_str(),jsp_SINGLE,jst_NUMBER);
        MySocket.broadcastTXT(s);
        MySocket.loop();
        Serial lln "Socket 3:" sp LastCountownSent sp s;
        yield();
        
      }
    }
    Waiting=millis();
  }

  flag_UserCanceled=false;
  
  if(WiFi.status() == WL_CONNECTED){
    String ss = "Success: " + WiFi.localIP().toString() + " on " + sSSID;
    Serial lln  "Success: " sp WiFi.localIP().toString().c_str() sp "on " sp sSSID;

    String s=CreateJSONString("WIFI_NAME",ss.c_str(),jsp_FIRST,jst_STRING);
    s+=CreateJSONString("OPERATION","Success",jsp_LAST,jst_STRING);
    MySocket.broadcastTXT(s);

    return true;
  }
  String s=CreateJSONString("OPERATION","Failed",jsp_SINGLE,jst_STRING);
  MySocket.broadcastTXT(s);

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