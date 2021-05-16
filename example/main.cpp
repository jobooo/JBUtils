#include <Arduino.h>
#include <Streaming.h>
#include "JBUtils.h"
#include "JBPitons.h"
#include "JBOledGUI.h"
#include "MenuDef.h"

JBOLED Ecran; 
JBValueSelector Selector(&Ecran,16,17,18,0,127,0); // Push Button with Rotary qui uptae l'écran
JBOledHorizDataBar BlowBar(0,20,25);    // barre horizontale au coordonées (x,y) 0,20 pixel et d'une hauteur de 25 pixels

/* Menu function pointers*/
int SelectorExample(void);
int HorizontalBarExample(void);
int fSoftwareReset(void);

char sValue[14]; // La valeur à afficher;

void setup() {
  Selector.Name=(char *)"Piton Rotary encoder";
  Serial.begin(115200);

  Serial << nl << "Mon Logiciel ESP32s"<<nl;
  Serial << "ESP.getCpuFreqMHz() = " << ESP.getCpuFreqMHz()<<nl;
  
  Ecran.Init(true);

  /*  
  *   Assignation des pointeur de fonction aux items du menu. 
  *   Voir la déficnition de menus dans le fichier MenuDef.h là où l'on défini les menus
  */ 
  MainMenuFunctions[0]=NULL;    // NULL signifie EXIT ou un sous-menu (a êtr edéfini dans le tableau des sous-menu). Le menu retournera 0, le menu parent restera IDLE en monde menu
  MainMenuFunctions[1]=&SelectorExample;
  MainMenuFunctions[2]=&HorizontalBarExample;    
  MainMenuFunctions[3]=&fSoftwareReset; // Je n'ai pas trouvé pourquoi, mais mettre directemeent "MainMenuFunctions[3] = MyResetFunction" ne fonctionne pas. 
  MainMenu.Rot=&Selector;
  
  InitMenus(); // Modifier cette fonction de même 

  Ecran.Clear();
}

void loop() {

  if(Selector.CheckRotary()!=0){
    Serial << Selector.Value << nl;
    ltoa(Selector.Value,sValue,10); 
    ssd1306_printFixed(8,32, "             ", STYLE_BOLD); 
    ssd1306_printFixed(8,32, sValue, STYLE_BOLD); 
  }

  if(Selector.PushButton->PushedAndReleased()){
      Serial << nl << "enter menu ";
      MainMenu.Run();
      Ecran.Clear();
  }
    
}

int SelectorExample(){
  
  Serial << "Selected Value = " << Selector.Run("Func 1",0,1024,0) << nl;
  Ecran.Clear();
  return -1;
}

int HorizontalBarExample(){
  // L'objet Selector va déja de 0 à 127. On peut le prendre directement pour donner les coordonées  
  Ecran.Clear();
  BlowBar.Draw(Selector.Value);

  while (!Selector.PushButton->PushedAndReleased())
    if(Selector.CheckRotary()!=0)
      BlowBar.Draw(Selector.Value);  

  Ecran.Clear();
  return -1;  
  // Note importante: la valeur de retour d'une fontion callée par le menu détermine le niveau de retour dans la structure de menu et sous menus. 
  // Par exemple, return 3, ferait remonter de 3 niveaux dans les menus (ou jusqu'au caller originel s'il y a moins que 3)
  // return 0, ne remonte pas et redraw le menu. 
  // return -1 retourne jusqu'au caller original (le loop() par exemple.)

}
int fSoftwareReset(){
  Serial << "Before Reset"<<nl;
  ESP.restart();  // for arduino use: MyResetFunction();
  Serial << "After Reset - ne devrait jamais arriver!"<<nl;
  return -1;
}