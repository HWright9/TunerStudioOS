/*
These are some utility functions and variables used through the main code
*/ 
#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

/*Variables Shared between Functions should have [XXXX_] naming convention where XXXX is the source module identifier*/


/* Functions */

uint16_t UTIL_freeRam();

uint8_t pinTranslate(uint8_t rawPin);
uint8_t pinTranslateAnalog(uint8_t rawPin);
uint16_t lowPassFilter_u16(uint32_t input, uint8_t alpha, uint32_t prior);


#endif // UTILS_H
