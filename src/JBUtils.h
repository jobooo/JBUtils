/*IN THIS FILE:
 * ftoa
 * MyResetFunction; 
 * JBMap
 * JBRound
  */

#ifndef JBUTILS_H
#define JBUTILS_H
#define JBUTILS_VERSION "2021-04-01"
#include <Arduino.h>
#include <Streaming.h>

#define NL "\n"
#define nl NL
#define ln <<'\n'
#define lln <<'\n' <<
#define sp << ' ' <<


typedef int (*FunctionPointer)();  // Declare typedef du type pointeur sur une fonction


float JBMap(float x, float in_min, float in_max, float out_min, float out_max){
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float JBRound(float finput, int NbSignificantDigitAfterDot){
    float f1,f2;
    unsigned long mult, ui1;
    
    if(NbSignificantDigitAfterDot==0) mult=1;
    else {    // Ici je dois me gosser un exposant car la function pow(); est buggée! (10exp2=99!!)
      mult=1; 
      for (int i=0;i<NbSignificantDigitAfterDot;i++) mult*=10;
    }    
    ui1=(finput*mult);
    f2=finput*mult-(unsigned long)(finput*mult);
    if (f2>=0.5)  ui1+=1;
    f1=(float)((float)ui1/mult);
    return f1;
}


/*
* Pour avoir une fonction MyResetFunction(), avec un arduino, copier la ligne suivante dans le programme principal 
* entre les #include et le setup(). Note le compilateur capote si on la met ailleurs.
* FunctionPointer MyResetFunction=0; // déclare la function "MyResetFunction()" à l'adresse  zéro  ce qui provoque un reset quand on l'appelle

* Pour la même chose avec un ESP32s, utiliser plutôt: 
* ESP.restart();  // ne nécéssite aucun #include
*/

/*
 * ftoa(double d,char *buffer, int precision)
 */
char *ftoa(double d,char *buffer, int precision) {
  long wholePart = (long) d;
  // Deposit the whole part of the number.
  itoa(wholePart,buffer,10);
  // Now work on the faction if we need one.
  if (precision > 0) {
    
    // We do, so locate the end of the string and insert
    // a decimal point.
    char *endOfString = buffer;
    while (*endOfString != '\0') endOfString++;
    *endOfString++ = '.';
    
    // Now work on the fraction, be sure to turn any negative
    // values positive.
    if (d < 0) {
      d *= -1;
      wholePart *= -1;
    }
    double fraction = d - wholePart;
    while (precision > 0) {
      // Multipleby ten and pull out the digit.
      fraction *= 10;
      wholePart = (long) fraction;
      *endOfString++ = '0' + wholePart;
      // Update the fraction and move on to the
      // next digit.
      fraction -= wholePart;
      precision--;
    }
    // Terminate the string.
    *endOfString = '\0';
  }
   return buffer;
}

#endif
