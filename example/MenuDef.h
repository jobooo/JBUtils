/*Exemple de construction de JBMenu
 * 
 */

#ifndef MenuExample_H
#define MenuExample_H

#include "JBOledGUI.h"

// ************ DÉFINITION D'UN MENU: DÉBUT *********************
#define MainMenuNbItems 5  // Combien d'items dans ce menu

// Textes de chaque Item:
const char *MainMenuTitles[MainMenuNbItems] = {
  //"1234567890123", 13 (en mode 16x8) char positions in menu 19 en mode 6x8. Il faut mettre des espaces pour que toute la ligne selectionée soit blanche
    
    " MIN. CUISSON",
    " QUI CUISINE?",
    " WiFi        ",
    " BYE         ",
    " RESET       "
};

FunctionPointer MainMenuFunctions[MainMenuNbItems]; // Tableau des pointeurs sur les fonction à éxécuter si un item est sélectionné. (NULL si c'est un sous menu ou "EXit"
JBMenu *MainMenuSubMenus[MainMenuNbItems];         // Tableau des pointeurs vers les éventuels sous menus

JBMenu MainMenu(MainMenuNbItems,MainMenuTitles,MainMenuFunctions,MainMenuSubMenus);           // l'objet JBMenu de ce menu

// *********** DÉFINITION D'UN MENU: FIN **************

// ************ DÉFINITION D'UN SOUS-MENU: DÉBUT *********************
#define QuiMenuNbItems 5  // Combien d'items dans ce menu

// Textes de chaque Item:
const char *QuiMenuTitles[QuiMenuNbItems] = {
  //"1234567890123", 13 (en mode 16x8) char positions in menu 19 en mode 6x8. Il faut mettre des espaces pour que toute la ligne selectionée soit blanche
    
    " PAPOUX      ",
    " MOMIE       ",
    " CLEMOUE     ",
    " LION        ",
    " BYE BYE la  "
};

FunctionPointer QuiMenuFunctions[QuiMenuNbItems]; // Tableau des pointeurs sur les fonction à éxécuter si un item est sélectionné. (NULL si c'est un sous menu ou "EXit"
JBMenu *QuiMenuSubMenus[QuiMenuNbItems];         // Tableau des pointeurs vers les éventuels sous menus

JBMenu QuiMenu(QuiMenuNbItems,QuiMenuTitles,QuiMenuFunctions,QuiMenuSubMenus);           // l'objet JBMenu de ce menu

// *********** DÉFINITION D'UN MENU: FIN **************

void InitMenus(){
  /* MAIN MENU */
  //MainMenuSubMenus[1]=&SoulierMenu;  // Puis assigner les sous-menus au besoin
  MainMenu.Init();
  QuiMenu.Init();
  for (size_t i = 0; i < QuiMenuNbItems; i++)  {
    QuiMenu.Functions[i]=NULL;
    
  }
  Serial << "Init() AppMenu Selection:" << QuiMenu.AppMenu.selection << nl;
};




#endif
