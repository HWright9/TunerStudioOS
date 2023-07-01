#ifndef INPUTOUTPUT_H
#define INPUTOUTPUT_H

#include "Arduino.h"

/*Variables Shared between Functions should have [XXXX_] naming convention where XXXX is the source module identifier*/


/* Functions */

void INIT_ADC(void);
void INIT_setPinMapping(void);
uint16_t readAnalog(uint8_t AinCH);
uint8_t readDigitalPort(uint8_t Dpin);
uint8_t setDigitalPort(uint8_t Dpin, uint8_t value, uint8_t setOverride);
uint8_t setPortMode(uint8_t Dpin, uint8_t pinmode);
uint8_t setPortOverride(uint8_t Dpin, uint8_t isOverride);



    
#endif
