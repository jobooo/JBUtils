/*CONTENU:
 * JBSwitch(SerialOut *pSP, byte pPin=2, byte pPullType=PullType_INT_PULLUP
 * MomentarySwitch(SerialOut *pSP, byte pPin=2, byte pPullType=PullType_INT_PULLUP):JBSwitch(pSP, pPin,pPullType) {}  
 * RotaryEncoderWithPushButton(SerialOut *pSP, byte pClockWisePin,byte pCounterClockWisePin, byte pPushButtonPin,long pMinValue=-50, long pMaxValue=50, long pStartValue=0,byte pButtonPullType=PullType_INT_PULLUP){
 * 
 */

#ifndef JBPITONS_H
#define JBPITONS_H
#define JBPITONS_VERSION "2021-03-10"
#include <Arduino.h>
#include <Streaming.h>
#include <RotaryEncoder.h> // Fourni par une Arduino lib
#include "JBUtils.h"


#define PullType_GND 0          // pulled down to GND
#define PullType_INT_PULLUP 1   // Internally pulled up (INPUT_PULLUP) (par defaut)
#define PullType_EXT_PULLUP 2   // Externally pulled up (je dois mettre une pull-ip resistor sur mon PCB).

#define DEBOUNCE_TYPE_EXT 0     // externally (cap + res on PCB) debounced before arduino pin
#define DEBOUNCE_TYPE_INT 1     // Must be debounced by arduino



/************************************************ 
 * Electric Switch
 * Usage: 
 * Constructuer: JBSwitch(byte pPin=2, byte pPullType=PullType_INT_PULLUP) 
 * Gère tous type de contact. 
 * Gère le débouncing
 * Se souvient du passé  (PinHasChangedFlag)
 * Gère la logique inverse si en mode pullup pullup
 * une seule fonction:
 *  bool CheckPin(void)
 *************************************************/
class JBSwitch {  
  private:
   
  public:
    
    const char *Name = NULL; // nom de la switch 
    byte Pin; // par défaut: la pin 2, première dispo sans utiliser 0 et 1 et qui sont aussi RX, TX 
    bool PinState=false; // état de la pin: 0 = open (pas de contact), 1 = closed (contact)
    byte PullType;        // Pullu-up interne (défaut), externe et pull-down to GND
    bool PinHasChangedFlag = false;   // used in change
    
    bool InvertLogic = true; //  PAr défaut: true.  Détermine si on doit iverser la valeur. 
                              // les switch avec pull-up (interne ou externe et qui sont le plus utilisées) sont true par defaut. 
                              // Lorsque le contact est engagé, il est mis au ground, ce qui met l'entrée du micro controler à false. 
                              // Cette valeur est automatiquement ajusté dans le constructeur selon la varible PullType alors il faut la changer au besoin.
    
   
    unsigned long lastDebounceTime = 0;
    unsigned long debounceDelay = 50;   // Default debounceDelay. Set to 0 to prevent debouncing
        
    // constructeur
    
    JBSwitch(byte pPin=2, byte pPullType=PullType_INT_PULLUP) {
      Pin=pPin;
      PullType=pPullType;
      
      
      switch (PullType) {
        case PullType_GND: // external GND pull-dwn
          pinMode(Pin,INPUT);
          InvertLogic=false;
          break;
          
        case PullType_INT_PULLUP: // Internally pulled up eutilisant la res 20k interne au Arduino. Donc pas besoin de resistance externe.
          pinMode(Pin,INPUT_PULLUP);
          InvertLogic=true;
          break;
          
        case PullType_EXT_PULLUP: // external Positive pull-uo
          pinMode(Pin,INPUT);
          InvertLogic=true;
          break; 
                    
        default:  // anything else, should not happen
          pinMode(Pin,INPUT);
          break;
      }
      lastDebounceTime = millis();  
    }

      
    // This function checks if value has changed.
    // Return TRUE if changed, false if not
    bool CheckPin(void){
        int val;

        // debouncing:
        if ((millis() - lastDebounceTime) < debounceDelay) return false;
        
        val = digitalRead(Pin);
        if(InvertLogic) val= !val;
 
        // return if no new value
        if(val==PinState){return false;}
        
        lastDebounceTime = millis();
        
        PinState=val;
        PinHasChangedFlag=true; // this is the only place  PianHasChangedFlag can be set to TRUE
       
        return true;
    }   
}; 

/****************************
 * class MomentarySwitch : public JBSwitch
 * 
 * Constructor: 
 * MomentarySwitch(byte pPin=2, byte pPullType=PullType_INT_PULLUP)
 * 
 * Gère les switch momentanée, telle que celle que l'on trouve sur les Encodeur Rotatifs 
 * ou une limit switch. 
 * Descend de JBSwitch
 * Fonctions: 
 *    bool Pushed() 
 *    bool PushedAndReleased()
 * 
 * **************************/
class MomentarySwitch : public JBSwitch {
    
    private:
    public: 
    //constructeur call son parent:
    MomentarySwitch(byte pPin=2, byte pPullType=PullType_INT_PULLUP):JBSwitch(pPin,pPullType) {}  

    
    
    // return true if the button was pushed and is still pressed. 
    bool Pushed(){ 
      CheckPin();
      if (PinHasChangedFlag && (PinState==true)) {
        PinHasChangedFlag=false;
        return true;  
      }else
        return false;
    }
    // return true if the button was pushed and released. 
    bool PushedAndReleased(){  
      CheckPin();
      if (PinHasChangedFlag && (PinState==false)) {
        PinHasChangedFlag=false;
        return true;  
      }else
        return false; 
    }
};


/************************************************
 * class RotaryEncoderWithPushButton
 * RotaryEncoderWithPushButton(
 *    byte pClockWisePin, byte pCounterClockWisePin, 
 *    byte pPushButtonPin,
 *    long pMinValue=-50, long pMaxValue=50, long pStartValue=0,
 *    byte pButtonPullType=PullType_INT_PULLUP)
 * 
 * Comprend une MomentarySwitch et un RotaryEncoder de la lib Arduino RotaryEncoder.h
 * 
 * Ne comprend qu'une seule fonction: 
 *  int CheckRotary() Returns table:
 *         1 if rotary encoder went clockwise 
 *        -1 if CCW
 *         0 if did not move  
 * Expose la variable Value pour connaître la valeure actuelle
 ***********************************************/
class RotaryEncoderWithPushButton {
private:
    
    byte CWPin, CCWPin;
    byte PushButtonPin;
            
public: 
    MomentarySwitch *PushButton; 
    long MinValue=-50, MaxValue=50, Value=0;
    
    char *Name = NULL;
    RotaryEncoder *Rotary;
     
    //constructeur call son parent:
    RotaryEncoderWithPushButton(byte pClockWisePin,byte pCounterClockWisePin, byte pPushButtonPin,long pMinValue=-50, long pMaxValue=50, long pStartValue=0,byte pButtonPullType=PullType_INT_PULLUP){
          
        CWPin = pClockWisePin;
        CCWPin = pCounterClockWisePin;
        PushButtonPin=pPushButtonPin;

        MinValue=pMinValue;
        MaxValue=pMaxValue; 
        Value=pStartValue;
        
        
        PushButton = new MomentarySwitch(pPushButtonPin,pButtonPullType);
        Rotary = new RotaryEncoder(CWPin, CCWPin, RotaryEncoder::LatchMode::TWO03); //Il se pourrait qu'il faille ajuster le dernier paramètre (TWO03 ) au besoin si l'encodeur est d'un autre type en particulier la séquence des ON/OFF
    }

   // CheckRotary() Returns table:
   //       1 if rotary encoder went clockwise 
   //       -1 if CCW
   //       0 if did not move  
   int CheckRotary(void){
        
        Rotary->tick();
        Value=Rotary->getPosition();

        switch ((int)(Rotary->getDirection())){
          case 0: // no movement
            
          break;
          case 1: // CW
            if (Value<=MaxValue){
                return 1;
            }else{
              Rotary->setPosition(MaxValue);
              Value=MaxValue;
              return 0;
            }

          break;
          case -1: // CCW
            if (Value>=MinValue){
                return -1;                
             }else{
                Rotary->setPosition(MinValue);
                Value=MinValue;
                return 0;
            }
          break;
          
          default:
          break;
        }
        return 0;
    }
};

#endif
