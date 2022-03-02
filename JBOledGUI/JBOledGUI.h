/*    CONTENU
 * JBOLED()
 * JBOledHorizDataBar(uint8_t pX=1,uint8_t pY=1,uint8_t pH=10)
 * JBValueSelector(.... see lower)
 * JBMenu(uint8_t pNbItems, const char **pTitles, FunctionPointer *pFunctions, JBMenu **pSubMenus)
 *
 */


/*Lire la doc dans la librairie arduino SS1306 de Alexey Dynda
 *
 * La fonction ssd1306_128x64_i2c_init() utilise les PIN suivantes par defaut:
 * Sur le ARDUINO:
 *    SDA (DATA) to pin A4
 *    SCL (clock) to pin A5
 *
 * Sur le ESP8266: (pas testé)
 *    SDA (DATA) to pin GPIO_04
 *    SCL (clock) to pin GPIO_05
 *
 * Sur le ESP32s:
 *    SDA (DATA) to pin GPIO_21
 *    SCL (clock) to pin GPIO_22
 *
 * il y a d'autre fonction telle que ssd1306_128x64_i2c_initEx(uint_8 scl,sda,sa (adress - defaut = 0)5)
 * qui permettent de choisir les pins. pour l'instant je laisse ça tel quel.
 *
 * inclure les ssd1306.h et nano_gfx.h et utiliser les fonction graphique directement.
 * Je ne vois pas l'utilité de les wrapper dans mon objet JBOLED pour l'instant.
 * Voir la fonction init() ci-bas pour un exemple de texte. voir MiCrane pour d'autres exemples.
 */

#ifndef JBOLED_H
#define JBOLED_H
#define JBOLED_VERSION "2021-03-10"
#include <arduino.h>
#include "Streaming.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_GFX.h>
#include <ssd1306.h>        // chercher la lib "ssd1306" par Alexey Dynda (ici la dernière version - 1.8.2) Attention: ne pas confondre avec lcdgfx du même auteur qui sort avec la requete "ssd1306"
#include <nano_gfx.h>
#include <JBUtils.h>
#include <JBPitons.h>


// Note le OLED 1306 a une resolution de 128(horiz)  * 64(vert) pixels
class JBOLED {
    private:
    int8_t DataPin, ClockPin, i2c_address;

    public:
      FunctionPointer AfterClear=NULL;
    // constructeur
    /* À date, la librairie 1306 marche avec les paramètres par défaut avec tous les écrans essayés,
     * même si l'adresse change, adresse qui est en général indiquée et sélectioné avec un RES 0ohm
     * à l'arriere du PCB de l'OLED.
     * USAGE:
     */
    JBOLED(int8_t pDataPin=-1, int8_t pClockPin=-1, int8_t pi2c_address = 0){
      DataPin=pDataPin;
      ClockPin=pClockPin;
      i2c_address=pi2c_address;
    }
    void Init(bool ShowInit=false){

      ssd1306_setFixedFont(ssd1306xled_font8x16); // 4 lignes  (64 pixels / 16 = 4)
      ssd1306_128x64_i2c_initEx(DataPin, ClockPin, i2c_address); // je n'ai jamais essayé d'autres pins. Pas certain que ça marche: peut-etre certaines contraintes i2c.
      ssd1306_clearScreen();
      if(ShowInit)
        ssd1306_printFixed(3.5*8, 16, "OLED INIT", STYLE_BOLD); //X (3.5ième Caractère sur 16 espaces dispo.), Y (ligne) dont les position possibles ne sont que: 0, 16, 32, 48
    }
    void Clear(){
      ssd1306_clearScreen();
      if(AfterClear!=NULL)
        (*AfterClear)();
    }
};

/*
 * Affiche une barre Horizontale de 0 à 127 de large
 * Note: OLED 1306 = 128 large * 64 haut
 * 0,0 est en haut à gauche
 */

class JBOledHorizDataBar{
  private:
    uint8_t LastVal=0;
  public:
    uint8_t x; uint8_t y; uint8_t Hauteur;

  JBOledHorizDataBar(uint8_t pX=1,uint8_t pY=1,uint8_t pH=10){
    x=pX; y=pY;Hauteur=pH;
  }

  void Draw(uint8_t Val){
    if (LastVal!=Val){
      ssd1306_fillRect(x,y,Val,y+Hauteur);

      if(Val<LastVal){
        ssd1306_negativeMode();
        ssd1306_fillRect(Val+1,y, LastVal,y+Hauteur);
        ssd1306_positiveMode();
      }
      LastVal=Val;
    }
  }
};

/*
 * Concu pour être utilisé pas plusieurs fonction,
 * Il faut donner des valeurs au Run() à chaque fois.
 */
class JBValueSelector : public RotaryEncoderWithPushButton{
  public:
    char sValue[14]; // La valeur à afficher;
    JBOLED *Ecran=NULL;

    // Constructor calls parent
    JBValueSelector(
                    JBOLED *pEcran,

                    byte pClockWisePin,
                    byte pCounterClockWisePin,
                    byte pPushButtonPin,
                    long pMinValue=-50, long pMaxValue=50, long pStartValue=0,
                    byte pButtonPullType=PullType_INT_PULLUP):RotaryEncoderWithPushButton(pClockWisePin, pCounterClockWisePin, pPushButtonPin, pMinValue, pMaxValue, pStartValue,pButtonPullType){

      Ecran=pEcran;
    }

    long Run(const char *pLabel=NULL, long pMinValue=0,long pMaxValue=127,long pValue=0){


          MinValue=pMinValue; MaxValue=pMaxValue; Value=pValue;
          Rotary->setPosition(pValue);
          Ecran->Clear();

          if(pLabel) ssd1306_printFixed(8,0, pLabel, STYLE_BOLD);
          ltoa(Value,sValue,10);
          ssd1306_printFixed(8,32,sValue, STYLE_BOLD);

          // tant que l'usager ne presse pas le pushbutton
          while(!PushButton->PushedAndReleased()){
           if(CheckRotary()!=0){
              ltoa(Value,sValue,10);
              ssd1306_printFixed(8,32, "             ", STYLE_BOLD);
              ssd1306_printFixed(8,32, sValue, STYLE_BOLD);
            }
          }
          return Value;
    }
};

/*
 * Selecteur de Timeur en Heures:Minutes avec option d'incrément dans le Run() (par défaut 5 min)
 * Concu pour être utilisé pas plusieurs fonction,
 * Il faut donner des valeurs au Run() à chaque fois.
 */
class JBMinHrsSelector : public RotaryEncoderWithPushButton{
  public:

    char sMinutes[14];
    char sValue[14]; // La valeur à afficher;
    JBOLED *Ecran=NULL;

    // Constructor calls parent
    JBMinHrsSelector(
                    JBOLED *pEcran,

                    byte pClockWisePin,
                    byte pCounterClockWisePin,
                    byte pPushButtonPin,
                    long pMinValue=-50, long pMaxValue=50, long pStartValue=0,
                    byte pButtonPullType=PullType_INT_PULLUP):RotaryEncoderWithPushButton(pClockWisePin, pCounterClockWisePin, pPushButtonPin, pMinValue, pMaxValue, pStartValue,pButtonPullType){

      Ecran=pEcran;
    }

    /*************************************************
      pLabel: label affiché au dessus de la valeur
      pMinValue, pMaxValue: vals min & max
      pValue, valeure par défaut
      pIncrement: multiplicateur de MAxValue et MinValue (6*5=30, 96*5=8 heures)
    ******************************/
    long Run(const char *pLabel=NULL, long pMinValue=0,long pMaxValue=96,long pValue=6, byte pIncrement=5){

          long Hours, Minutes;
          MinValue=pMinValue; MaxValue=pMaxValue; Value=pValue;
          Rotary->setPosition(pValue);
          Ecran->Clear();

          if(pLabel) ssd1306_printFixed(8,0, pLabel, STYLE_BOLD);
          Hours=(long)((Value*pIncrement)/60);
          Minutes=(long)((Value*pIncrement)%60);

          ltoa(Hours,sValue,10);
          ltoa(Minutes,sMinutes,10);
          strcat(sValue,":");
          if(Minutes<10)
            strcat(sValue,"0");  
          strcat(sValue,sMinutes);

          ssd1306_printFixed(40,32,sValue, STYLE_BOLD);

          // tant que l'usager ne presse pas le pushbutton
          while(!PushButton->PushedAndReleased()){
           if(CheckRotary()!=0){
              Hours=(long)((Value*pIncrement)/60);
              Minutes=(long)((Value*pIncrement)%60);
              ltoa(Hours,sValue,10);
              ltoa(Minutes,sMinutes,10);
              strcat(sValue,":");
              if(Minutes<10)
                strcat(sValue,"0");  
              strcat(sValue,sMinutes);

              ssd1306_printFixed(40,32, "             ", STYLE_BOLD);
              ssd1306_printFixed(40,32, sValue, STYLE_BOLD);
            }
          }
          return Value*pIncrement;
    }
};


class JBMenu{
  private:
    bool inited=false;
  public:
    RotaryEncoderWithPushButton *Rot=NULL;
    SAppMenu AppMenu;

    bool MenuCirculaire=false; // Mettre à TRUE pour que le menu saute au début s'il arrive à la fin et vice versa

    FunctionPointer *Functions; // pointeur sur un tableau de pointeur sur des fonctions
    JBMenu **SubMenus=NULL;     // pointeur sur un tableau de JBMenu constituant les sous menu éventuels.
                                // Ils DOIVENT pointer sur NULL si ils ne pointent pas sur un sous menu

    // Constructeur
    JBMenu(uint8_t pNbItems, const char **pTitles, FunctionPointer *pFunctions, JBMenu **pSubMenus) {
        AppMenu.count=pNbItems;
        AppMenu.items=pTitles;
        Functions=pFunctions;
        SubMenus=pSubMenus;
        for(int i=0;i<pNbItems;i++){
          //Par défaut mettre tous les pointeur de sous-menus à NULL puisque le traitement cherche des pointeurs non-null pour exécuter un sous-menu
          SubMenus[i]=NULL;
    }
};

    void Init(){
      if(!inited){
        ssd1306_createMenu(&AppMenu, AppMenu.items, AppMenu.count);
        inited=true;
      }
    };
    void Show(){
      Init();
      ssd1306_clearScreen();
      ssd1306_showMenu(&AppMenu);
    }


    int Run(){
      Show();

      while(true){      // roule à l'infini (sorties avec return ci-bas)

        // Check Rotary Encoder
        switch (Rot->CheckRotary()){      // Si rotary a tourné
          case -1:  // menu up
            if(MenuCirculaire || AppMenu.selection > 0){
              ssd1306_menuUp(&AppMenu);
              ssd1306_updateMenu(&AppMenu);
            }
          break;
          case 1:  // menu down
            if(MenuCirculaire || AppMenu.selection < AppMenu.count-1){
              ssd1306_menuDown(&AppMenu);
              ssd1306_updateMenu(&AppMenu);
            }
          break;
          default:
          break;
        }

        // Check if PuchButton pressed
        if(Rot->PushButton->PushedAndReleased()){
          int retval=0;

          /*
           * C'est ici que tout se passe.
           * Si l'item est un sous menu (son pointeur n'est pas NULL), on call le sous menu
           *
           * Sinon, c'est une fonction, et si ce pointeur n'Est pas null, la fonction est appelée
           * et selon la valeur que celle-ci retourne,
           * une action ou une autre sera prise, soit:
           * 0 : le menu continue d'attendre un évènement up, dwn ou push
           * -1: le menu retrounera en cascade jusqu'à l'origine (probablement le loop())
           * Toute autre valeure sera décrémentée (--) puis retourneé vers le haut.
           * ça permet de remonter d'un nombre de niveau spécifique
           */

          if(SubMenus[AppMenu.selection]!=NULL){
            int smretval=0;

            smretval=SubMenus[AppMenu.selection]->Run(); // Run le sous-menu -- ici on devient récursif! /

            switch (smretval){
                case -1:      // return all the way to originating caller
                  return -1;
                break;

                case 0:   // Stay here, redraw menu
                  Show();
                break;

                default:
                  return --retval;  // toute autre valeur (typiquement au dessus de zero):
                                    // décremente retval avant de la retourner.
                                    // Permet de remonter d'un ou plusieurs sous-menus.
                                    // Si le nombre est plus grand que le nombre de sou-menus impliqués, c'Est l'équivalent de retourner -1
                break;
            }
          }else {
            if(Functions[AppMenu.selection]!=NULL)
              retval = (*Functions[AppMenu.selection])(); // Appelle la fonction enregistrée pour cet item de menu
            else // if function pointer is NULL, return to preceding level.
              return 0;

            switch (retval){
              case -1:      // return all the way to originating caller
                return -1;
              break;

              case 0:   // Stay here, redraw menu
                Show();
              break;
              default:
                return --retval;  // toute autre valeur (typiquement au dessus de zero):
                                  // décremente retval avant de la retourner.
                                  // Permet de remonter d'un ou plusieurs sous-menus.
                                  // Si le nombre est plus grand que le nombre de sou-menus impliqués, c'Est l'équivalent de retourner -1
              break;
            }
          }
        }
      }
    }
};



#endif
