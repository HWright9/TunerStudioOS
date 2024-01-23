/*
* userfunctions.ino
* A space for user defined code. This code should be called from the main function tasks.
*
* Copyright (C) Henry Wright
* A full copy of the license may be found in the projects root directory
*
*/

#include "userfunctions.h"

/*Variables Local to this module */


/* Functions */

/* USER_blinkCEL
* Run Rate: 500ms
* Toggles the state of the builtin LED on the Arduino.
*/
void USER_blinkCEL(void)
{
  if (readDigitalPort(LED_BUILTIN) == HIGH)
  {
    setDigitalPort(LED_BUILTIN, LOW, OUTPUT_NORMAL);
  }
  else
  {
    setDigitalPort(LED_BUILTIN, HIGH, OUTPUT_NORMAL);
  }
}

/* USER_setOutputs
* Run Rate: 100ms
* Sets outputs and inputs and checks if TestIO is active. Note this can override normal program control of outputs.
*/
void USER_InputOutput(void)
{
  uint8_t i;
  
  for (i = 0; i < BOARD_MAX_DIGITAL_PINS; i++)
  {
    readDigitalPort(i);
  }
  for (i = 0; i <= BOARD_MAX_ADC_PINS; i++)
  {
    readAnalog(i);
  }
  
  Out_TS.Vars.Ve_i_TestByte1 = lowPassFilter_u16(Out_TS.Vars.Analog[0]/4, (uint8_t)configPage2.Ke_i_TestValue, Out_TS.Vars.Ve_i_TestByte1);
  
}
