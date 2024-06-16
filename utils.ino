/*
* utils.ino
* Utilitised by the system internally.
*
* Copyright (C) Henry Wright
* A full copy of the license may be found in the projects root directory
*
* Based on code by Josh Stewart for the Speeduino project , and Darren Siepka for the GPIO module see www.Speeduino.com for more info
*/

/*Variables Local to this module */


/* Functions */


/*
  Returns how much free dynamic memory exists (between heap and stack)
*/
#include "utils.h"

uint16_t UTIL_freeRam(void)
{
#if defined (CORE_AVR)
  extern int __heap_start, *__brkval;
  uint16_t v;

  return (uint16_t) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);

#elif defined(CORE_TEENSY)
  uint32_t stackTop;
  uint32_t heapTop;

  // current position of the stack.
  stackTop = (uint32_t) &stackTop;

  // current position of heap.
  void* hTop = malloc(1);
  heapTop = (uint32_t) hTop;
  free(hTop);

  // The difference is the free, available ram.
  return (uint16_t)stackTop - heapTop;
#elif defined(CORE_STM32)
  char top = 't';
  return &top - reinterpret_cast<char*>(sbrk(0));  //what is sbrk?
//  return 0;
#endif
}

/** Translate between the pin list that appears in TS and the actual pin numbers.
For the **digital IO**, this will simply return the same number as the rawPin value as those are mapped directly.
For **analog pins**, it will translate them into the correct internal pin number.
* @param rawPin - High level pin number
* @return Translated / usable pin number
*/
uint8_t pinTranslate(uint8_t rawPin)
{
  uint8_t outputPin = rawPin;
#if defined (ARDUINO_AVR_MEGA2560)
  if(rawPin > BOARD_MAX_DIGITAL_PINS) { outputPin = A8 + (outputPin - BOARD_MAX_DIGITAL_PINS - 1); }
#else
//do nothing?
#endif 

  return outputPin;
}
/** Translate a pin number (0 - 22) to the relevant Ax (analog) pin reference.
* This is required as some ARM chips do not have all analog pins in order (EG pin A15 != A14 + 1).
* */
uint8_t pinTranslateAnalog(uint8_t rawPin)
{
  uint8_t outputPin = rawPin;
  switch(rawPin)
  {
    case 0: outputPin = A0; break;
    case 1: outputPin = A1; break;
    case 2: outputPin = A2; break;
    case 3: outputPin = A3; break;
    case 4: outputPin = A4; break;
    case 5: outputPin = A5; break;
    #if BOARD_MAX_ADC_PINS >= 6
      case 6: outputPin = A6; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 7
      case 7: outputPin = A7; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 8
      case 8: outputPin = A8; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 9
      case 9: outputPin = A9; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 10
      case 10: outputPin = A10; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 11
      case 11: outputPin = A11; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 12
      case 12: outputPin = A12; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 13
      case 13: outputPin = A13; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 14
      case 14: outputPin = A14; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 15
      case 15: outputPin = A15; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 16
      case 16: outputPin = A16; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 17
      case 17: outputPin = A17; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 18
      case 18: outputPin = A18; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 19
      case 19: outputPin = A19; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 20
      case 20: outputPin = A20; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 21
      case 21: outputPin = A21; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 22
      case 22: outputPin = A22; break;
    #endif
  }

  return outputPin;
}

/**
 * @brief Calculates a low pass filter using an "alpha value." Returns the filtered value
 * 
 * @param input The input value to be used to update the previous value.
 * @param alpha The filter value. 128 would be a 50% per loop filter. Valid range 0-255
 * @param prior The previous value.
 */
uint16_t lowPassFilter_u16(uint32_t input, uint8_t alpha, uint32_t prior)
{
  uint16_t result = (uint16_t)input;
  if ((input != prior) && (alpha > 0)) // only need to calculate filter if there is a difference or a filter value
  {
    result = (((input * (256 - alpha)) + (prior * (uint32_t)alpha))) >> 8;
    if (result == (uint16_t)prior) // non-convergence detected
    { 
      if (input > prior) { result = (uint16_t)prior + 1; }
      else { result = (uint16_t)prior - 1; }
    } // Ensures filter convergence to input value when alpha filters are large and changes in result are <1.
  }
  // else result = input. and nothing to do.
  return result;
}


/**
 * @brief Toggles a bit
 * 
 * @param input The input value to be used to update the previous value.
 * @param alpha The filter value. 128 would be a 50% per loop filter. Valid range 0-255
 * @param prior The previous value.
 */