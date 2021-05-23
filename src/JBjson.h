#ifndef JBJSON_H
#define JBJSON_H

#include <Arduino.h>
#include <Streaming.h>


#define JSON_DATATYPE_STRING 0
#define JSON_DATATYPE_NUMBER 1
#define JSON_DATATYPE_BOOL 2

#define JSON_DATA_POS_FIRST     0
#define JSON_DATA_POS_MIDDLE    1
#define JSON_DATA_POS_LAST      2
#define JSON_DATA_POS_SINGLE    3

/* GetJSONData(const char *pKey,  char * InThis_JSON_Data)

JSON data format: 
https://www.w3schools.com/js/js_json_datatypes.asp
Dans cette fonction GetJSONData(clé, BlocDeDonnées) , je me limite au cas le plus simple: 
Je cherche un clé (key) et fournis le data qui suit (après le ':')  
Donc pour que ça marche, le data doit etre dans le format suivant:
"nom de la clé":"Valeure"
les {} et [] et les virgules sont ignoré 
La chaine de char fournie en paramêtre doit être bien formattée. 
En cas d'échec, une string vide est retournée.
Exemple (ici avec des newline pour faire plus clair):
{ 
    "page":"welcome_AP.html",
    "Seconds Left": 20,
    "refresh" : false,
    "optional string": null
}

exemple d'utilisation: 

  String myjs = String();
  myjs=CreateJSONString("Cle","Value",JSON_DATA_POS_SINGLE);
  Serial lln "JSON A:" sp myjs;
  myjs=CreateJSONString("Key1","StringKEy",JSON_DATA_POS_FIRST, JSON_DATATYPE_STRING);
  myjs = myjs + CreateJSONString("Key 2","666",JSON_DATA_POS_MIDDLE,JSON_DATATYPE_NUMBER);
  myjs = myjs + CreateJSONString("Key 3","true",JSON_DATA_POS_LAST,JSON_DATATYPE_BOOL);
  Serial lln "JSON B:" sp myjs;
  String myTmps=String();
  myTmps= "Key1=" + GetJSONData("Key1",myjs.c_str(),JSON_DATATYPE_STRING) + "\n\r";
  myTmps = myTmps + "Key 2=" + GetJSONData("Key 2",myjs.c_str(),JSON_DATATYPE_NUMBER) + "\n\r";
  myTmps = myTmps + "Key 3=" + GetJSONData("Key 3",myjs.c_str(),JSON_DATATYPE_BOOL) + "\n\r";
  Serial lln myTmps;

**************************************************************/
String GetJSONData(const char *pKey, const char * InThis_JSON_Data, uint8_t dataType=JSON_DATATYPE_STRING){
    String s=InThis_JSON_Data;
    String r=String();
    int p1=0; int p2=0;

    p1=s.indexOf(pKey);         if(p1==-1) return r;
    p2=s.indexOf(":",p1);       if(p2==-1) return r;
    if(dataType==JSON_DATATYPE_STRING){
        p1=s.indexOf("\"",p2); if(p1==-1) return r; p1++;
        p2=s.indexOf("\"",p1); if(p2==-1) return r;
        r=s.substring(p1,p2);
    }else{
        p1=p2+1;

        p2=s.indexOf(",",p1);
        if(p2==-1){
            p2=s.indexOf("}",p1);
            if(p2==-1){
                return r;
            }
        }
        r=s.substring(p1,p2);
        r.trim();
    }
    return r;
}


String CreateJSONString(const char *pKeyName, const char *pValue, int dataPosition=JSON_DATA_POS_MIDDLE, int dataType=JSON_DATATYPE_STRING){
    String r=String();
    switch(dataPosition){
        case JSON_DATA_POS_FIRST:
            r = r + "{\"" + pKeyName + "\":";
            if(dataType==JSON_DATATYPE_STRING){
                r= r+ "\"" + pValue + "\"";
            } else {
                r = r + pValue;
            }
            r = r + ",";
            break;

        case JSON_DATA_POS_LAST:
            r = r + "\"" + pKeyName + "\":";
            if(dataType==JSON_DATATYPE_STRING){
                r= r + "\"" + pValue + "\"";
            } else {
                r= r + pValue;
            }
            r = r + "}";
            break; 

        case JSON_DATA_POS_MIDDLE:
            r = r + "\"" + pKeyName + "\":";
            if(dataType==JSON_DATATYPE_STRING){
                r= r+ "\"" + pValue + "\"";
            } else {
                r = r + pValue;
            }
            r = r + ",";
            break;
        
        case JSON_DATA_POS_SINGLE:
            r = r + "{\"" + pKeyName + "\":";
            if(dataType==JSON_DATATYPE_STRING){
                r = r + "\"" + pValue + "\"";
            } else {
                r = r + pValue;
            }
            r = r + "}";
            break;
        default:
            break;
    }
    return r;
}
#endif