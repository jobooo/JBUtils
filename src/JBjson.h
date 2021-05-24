#ifndef JBJSON_H
#define JBJSON_H

#include <Arduino.h>
#include <Streaming.h>


typedef enum  {
    jst_STRING,
    jst_NUMBER,
    jst_BOOL
}JSON_DATATYPE;

typedef enum  {
    jsp_FIRST,
    jsp_MIDDLE,
    jsp_LAST,
    jsp_SINGLE
}JSON_DATA_POSITION;
 

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
  myjs=CreateJSONString("Cle","Value",jsp_SINGLE);
  Serial lln "JSON A:" sp myjs;
  myjs=CreateJSONString("Key1","StringKEy",jsp_FIRST, jst_STRING);
  myjs = myjs + CreateJSONString("Key 2","666",jsp_MIDDLE,jst_NUMBER);
  myjs = myjs + CreateJSONString("Key 3","true",jsp_LAST,jst_BOOL);
  Serial lln "JSON B:" sp myjs;
  String myTmps=String();
  myTmps= "Key1=" + GetJSONData("Key1",myjs.c_str(),jst_STRING) + "\n\r";
  myTmps = myTmps + "Key 2=" + GetJSONData("Key 2",myjs.c_str(),jst_NUMBER) + "\n\r";
  myTmps = myTmps + "Key 3=" + GetJSONData("Key 3",myjs.c_str(),jst_BOOL) + "\n\r";
  Serial lln myTmps;

**************************************************************/
String GetJSONData(const char *pKey, const char * InThis_JSON_Data, JSON_DATATYPE dataType=jst_STRING){
    String s=InThis_JSON_Data;
    String r=String();
    int p1=0; int p2=0;

    p1=s.indexOf(pKey);         if(p1==-1) return r;
    p2=s.indexOf(":",p1);       if(p2==-1) return r;
    if(dataType==jst_STRING){
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

// Create a JSON string with Key, Value. 
// dataPosition is used to determine if {} and "," are needed, 
// dataType determine if "" are needed
String CreateJSONString(const char *pKeyName, 
                        const char *pValue, 
                        JSON_DATA_POSITION dataPosition=jsp_MIDDLE, 
                        JSON_DATATYPE dataType=jst_STRING   
                        )
{
    String r=String();
    switch(dataPosition){
        case jsp_FIRST:
            r = r + "{\"" + pKeyName + "\":";
            if(dataType==jst_STRING){
                r= r+ "\"" + pValue + "\"";
            } else {
                r = r + pValue;
            }
            r = r + ",";
            break;

        case jsp_LAST:
            r = r + "\"" + pKeyName + "\":";
            if(dataType==jst_STRING){
                r= r + "\"" + pValue + "\"";
            } else {
                r= r + pValue;
            }
            r = r + "}";
            break; 

        case jsp_MIDDLE:
            r = r + "\"" + pKeyName + "\":";
            if(dataType==jst_STRING){
                r= r+ "\"" + pValue + "\"";
            } else {
                r = r + pValue;
            }
            r = r + ",";
            break;
        
        case jsp_SINGLE:
            r = r + "{\"" + pKeyName + "\":";
            if(dataType==jst_STRING){
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