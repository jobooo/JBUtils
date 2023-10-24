/************************************************************************************
 * JBESPFiles est basé sur le FS d'arduino et le SPIFFS pour les ESP (32 et 2866)
 ************************************************************************************/

#include <Arduino.h>
#include <Streaming.h>
#include "JBUtils.h"
#include <FS.h>
#include "SPIFFS.h"
#ifndef JBESPFiles_VERSION

#define JBESPFiles_VERSION "2021-04-01"

String ReadLineToString(File f);
String ReadFileToString(String sFile2Read);
uint WriteStringToFile(String sFileName, String sContent, bool bAppend);
int ReadFileUntil(File f, char * dest, char terminator);

/* Starts the SPIFFS file system. Should be called once at the begining of runtime.*/
bool initSPIFFS() {
  if (!SPIFFS.begin()) {
    Serial.println("Cannot start SPIFFS file system...");
    return false;
  }
  return true;
}

// Read a complete file to a string; Always returns a valid String object that may be empty but not null
String ReadFileToString(String sFile2Read){
  String s="";
  File f = SPIFFS.open(sFile2Read, "r");
  if(!f){Serial << "ReadFileToString:" << sFile2Read sp " open file" ln; return s;}
  
  while(f.available()){
    s+=(char)f.read();
  }
  f.close();
  return s;
}

uint WriteStringToFile(String sFileName, String sContent, bool bAppend=false){
  File f = SPIFFS.open(sFileName, bAppend ? "w+":"w");
  if(!f){Serial << "ReadFileToString:" << sFileName sp " open file" ln; return 0;}
  
  unsigned int buffLen;
  buffLen=sContent.length();

  for(uint i = 0; i<buffLen;i++){
    f.write(sContent.charAt(i));
  }
  f.close();
  return buffLen;
}

/*int ReadFileUntil(File f, char * dest, char terminator)
  Lit le fichier dans le buffer dest jusqu'à ce qu.il rencontre le terminator.
  Les caractères terminators sont flushés
   un NULL STRING TERMINATOR ('\0') es ajouté à la fin du buffer si au moins 1 caractere fut lu. 
   RETURN le nb de char placé dans le buffer sauf le '\0' de la fin
******************************************************************/
int ReadFileUntil(File f, char * dest, char terminator){
  int i=0;
  char c=0;
  //Serial << "IN ReadFileUntil()" << nl;
  while(f.available() && c!=terminator){
    c=f.read();
    //Serial << "c=" << c << "(" << (byte)c << ") " <<  "i=" << i<< nl;
    if(c!=terminator){
      if(dest!=NULL){  // éliminer le "cariage return"
        dest[i++]=c;
      } 
    }
  }
  if(i>0) dest[i]='\0';
  return i;
}


String ReadLineToString(File f){
  String s="";
  byte c=0;
  bool EndLineReached;
  EndLineReached=false;

  while(f.available() && !EndLineReached){
    c=(byte)f.read();
    switch (c)    {
    case 10:
    case 13:
      while ((c==10 || c==13) && f.available()){
        c=(byte)f.read();  
      }
      EndLineReached=true;
      if(f.available())
        f.seek(f.position()-1);
      break;
    case 0:
      
      break;  
    
    default:
      s+=(char)c;
      break;
    }
  }
  return s;
}

class JBINIFile {
private:
  bool is_open=false;
public:
  File f;
  String INIFileName;
  JBINIFile(String filename){INIFileName=filename;}
  JBINIFile(const char *filename){INIFileName=filename;}
  
  bool Open(const char *mode = "r");
  void Close(){ 
    if(is_open)
      f.close();
    is_open=false;
  }

  String GetINIFileKey(String sKey, bool KeepOpen =false);
  bool  SetINIFileKey(String sKey, String sValue);

};

bool JBINIFile::Open(const char *mode){
    
    if(is_open) 
      return true;
    
    f = SPIFFS.open(INIFileName, mode);
    if(!f){Serial << "Can't open " <<INIFileName sp " SPIFFS.open()" ln; return false;}
    is_open=true;
    return true;
}

String JBINIFile::GetINIFileKey(String sKey, bool KeepOpen){
  String s1,s2;

  
  if(!is_open)
    if(!Open()) 
      return s1;

  f.seek(0);
  
  while(f.available()){
    s1=ReadLineToString(f);
    
    if(s1.indexOf(sKey)!=-1){
      s2=s1.substring(s1.indexOf("=")+1);
      if(!KeepOpen) Close();
      return s2;
    }
  }
  if(!KeepOpen) 
    Close();
  return s2;  
}

bool  JBINIFile::SetINIFileKey(String sKey, String sValue){

  String s1;
  String s2;

  f.close();
  if(!Open()) 
    return false;

  //f.seek(0);
  
  while(f.available())  { 
    s1=ReadLineToString(f);
    if(s1.length()>0) {
      if(s1.indexOf(sKey)!=-1){   // Son trouve, on remplace la ligne par une nouvelle avec la key et la value
        s2+=sKey+"="+sValue+'\n'+'\r';
      }
      else{                       // Sinon on garde la même ligne
        s2+=s1+'\n'+'\r';
      }
    }
  }

  Close();
  
  if(s2.length()==0)
    s2+=sKey+"="+sValue+'\n'+'\r';
  
  WriteStringToFile(INIFileName,s2);

  return true;  
}
#endif