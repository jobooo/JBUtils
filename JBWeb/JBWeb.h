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
#ifndef JBWEB_H
#define JBWEB_H

#include "SPIFFS.h"
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "JBUtils.h"
#include "JBESPFiles.h"
#include "JBjson.h"
// V0.1
// Create AsyncWebServer object on port 80
AsyncWebServer MyAsyncWebServer(80);
// Create WebSocket on another port
WebSocketsServer MySocket(81);

#define WiFiLogInTimeOut 20000  // when user gives new WiFi SSID & PW, how long do we try to log before failling. (in milliseconds)

/**********************************************
 Stuff Used show avail networks 
 and to sort them by their strenght
***********************************************/
typedef struct {
  String NetName;     
  int dbPower;
} NetworkItem;
// a pointer to an array of network Items, 
NetworkItem *NetList=nullptr; 
int numberOfNetworks=0;
void ScanNetworks(void);
int CompareNetworksPower (const void * a, const void * b);

String populateWebVars_WiFiList(const String&  var);

// HTML request handlers
void htmlReq_TryThisWifi(AsyncWebServerRequest *req);
void htmlReq_SelectWiFi(AsyncWebServerRequest *request);

// web socket handler(s)
void TryThisWiFi_html_wsHandler(char *JSON_data, uint8_t clientID=-1);


/******************************************************
 * WiFi connection stuff
 * ****************************************************/
String sTryThisSSID, sTryThisPW;
bool flag_TryNewWiFi =false;

void StartSoftAP(const char * SSIDName, char * PW=nullptr); // Used to start ESP32's local private WiFi provider
bool flag_UserCanceled = false;  // If user cancels while tryin to log to a wifi router 
bool ConnectToWiFi(const char *pSSID=nullptr, const char *pPW=nullptr, bool updateSocket=false);



/****************************************
 * Connect to WiFi. 
 * All parameters are optional. 
 * Will Connect to SSID and PW found in CONFIG.INI if params are empty
 * updateSocket is false by default. If true, will bradcast to WebSocketClients
 * about SSID and time left before fail
 * *****************************************/
bool ConnectToWiFi(const char *pSSID, const char *pPW, bool updateSocket) {
  // Timers to wait for conection 
  long Waiting =0;
  long WaitStart=millis();
  long LastCountownSent=0;
  
  String sSSID, sPW;

  // Will use data from CONFIG.ini if pSSID is empty
  if(!pSSID || strlen(pSSID)<=0){
    JBINIFile INIFile("/config.ini");
    sSSID=INIFile.GetINIFileKey("SSID").c_str();
    sPW=INIFile.GetINIFileKey("PW").c_str();
    INIFile.Close();
  }else{
    sSSID=pSSID;
    sPW=pPW;
  }
  
  // if we must update WebSocket clients
  if(updateSocket){
    // Populate JSON string with ssid & time left
    String s=CreateJSONString("WIFI_NAME",sSSID.c_str(),jsp_FIRST,jst_STRING);
    s+=CreateJSONString("TIME_LEFT",String((WiFiLogInTimeOut/1000)-LastCountownSent).c_str(),jsp_LAST,jst_NUMBER);
    MySocket.broadcastTXT(s);
    MySocket.loop();
    yield();
  }

  if(sSSID.length()>=0){
    Serial lln "ConnectToWiFi() connecting to: " sp sSSID sp sPW;
    WiFi.disconnect();  // disconect first
    while(WiFi.status()==WL_CONNECTED){   // on attend que le status change sinon ça passe trop vite vers le "success!"
      yield(); 
    } 

    WiFi.begin(sSSID.c_str(),sPW.c_str());
    Serial lln "Post wifi.begin()";

  } else {
    Serial lln "Error: Trying to log to empty sSSID";
    flag_UserCanceled=false;
    return false;
  }
    
  
  // Tant que non connecté ( et que le timeout n'est pas dépassé)
  Waiting=millis();
  while (WiFi.status() != WL_CONNECTED && Waiting-WaitStart<WiFiLogInTimeOut) {
    static int laststatus=0;
    
    // Check si le status a changé
    if(laststatus!=WiFi.status()){ laststatus=WiFi.status(); Serial sp "new status=" << laststatus;}
    
    if(updateSocket) {  // if we must update WebSocket clients
      MySocket.loop();

      if(flag_UserCanceled){  // The user canceled the trial (see main.cpp: TryThisWiFi_html_wsHandler())
        Serial lln "Socket 2";
        return false; 
      }
    }

    yield(); // céder le processeur

    // Donner signe de vie environ 1 fois par seconde au client et sur le port serie.   
    if((long)((Waiting-WaitStart)/1000) != LastCountownSent){
      LastCountownSent=(long)((Waiting-WaitStart)/1000);
      Serial.print(".");
      
      if(updateSocket){ // if we must update WebSocket clients
        String s =CreateJSONString("TIME_LEFT",String((WiFiLogInTimeOut/1000)-LastCountownSent).c_str(),jsp_SINGLE,jst_NUMBER);
        MySocket.broadcastTXT(s);
        MySocket.loop();
        yield();
        
      }
    }
    Waiting=millis();
  }

  flag_UserCanceled=false; // reset for future use
  
  // Succes!! broadcast the good news
  if(WiFi.status() == WL_CONNECTED){
    String s=CreateJSONString("WIFI_NAME",sSSID.c_str(),jsp_FIRST,jst_STRING);
    s+=CreateJSONString("OPERATION","Success",jsp_LAST,jst_STRING);
    MySocket.broadcastTXT(s);
    Serial lln s lln  WiFi.localIP();
    return true;
  }
  // Failed: broadcast the good news
  String s=CreateJSONString("OPERATION","Failed",jsp_SINGLE,jst_STRING);
  MySocket.broadcastTXT(s);
  Serial lln "ConnectToWiFi() = Failed to log to: " sp sSSID sp sPW;
  return false;
}

// Start Software Access Point, ESP32's own Wifi Provider
void StartSoftAP(const char * SSIDName, char * PW){
  WiFi.softAPConfig(IPAddress(10,10,10,10),IPAddress(10,10,10,1),IPAddress(255,255,255,0));
  WiFi.softAP(SSIDName,PW);
  Serial lln "IP * address: " sp  WiFi.softAPIP();
}

// qSort function to sort avail neworks by power, descending
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

// HAndler for TryThisWifi.html file request
void htmlReq_TryThisWifi(AsyncWebServerRequest *req) {
  Serial lln "htmlReq_TryThisWifi A";
  
  AsyncWebParameter *awParam;
  awParam = req->getParam("WiFiNets",true,false);
  sTryThisSSID=awParam->value();
  Serial lln "htmlReq_TryThisWifi says: " sp sTryThisSSID;
  awParam = req->getParam("pwd",true,false);
  sTryThisPW=awParam->value();
  Serial << "PW: " sp sTryThisPW;

  // Envoie une réponse HTML au caller
  req->send(SPIFFS, "/TryThisWiFi.html", "text/html",false);
}

// HAndler for SelectWiFi.html file request
void htmlReq_SelectWiFi(AsyncWebServerRequest *request) {
  request->send(SPIFFS, "/SelectWiFi.html", "text/html", false, populateWebVars_WiFiList);
}

void TryThisWiFi_html_wsHandler(char *JSON_data, uint8_t clientID) {
  if(GetJSONData("action",(char *)JSON_data)=="ReadyToMonitor"){
      // Cette réponse attend des updates de status de tentative de login via le websocket. On envoi ici les valeurs de départ
    String tmpJSON;
    tmpJSON = CreateJSONString("WIFI_NAME",sTryThisSSID.c_str(),jsp_FIRST,jst_STRING);
    tmpJSON += CreateJSONString("TIME_LEFT", String(WiFiLogInTimeOut/1000).c_str(),jsp_LAST,jst_NUMBER);
    MySocket.sendTXT(clientID,tmpJSON);
    
    flag_TryNewWiFi=true;   //Je crois (pas vérifié) qu'on ne peut pas caller ConnectToWiFi() durant une gestion d'un evenement socket, on passe par un flag qui sera vu dans le loop() 
  }
  
  if(GetJSONData("action",(char *)JSON_data)=="BT_CANCEL"){
      flag_UserCanceled=true;
  }
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

// Function called by the WebServer in main.cpp to populate the avail able WiFi list
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