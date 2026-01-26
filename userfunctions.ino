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

/* USER_ButtonTest
* Run Rate: 20ms
* Read Button Inputs and Set LED's
*/
void USER_ButtonTest(void)
{
  uint16_t buttonADC = readAnalog(Pin_analogButtons);

  if ((buttonADC > 900) || (buttonADC < 100)) // No buttons pressed, or disconnected
  {
    BIT_CLEAR(Vb_b_buttonsStatus,BUTTON_RED);
    BIT_CLEAR(Vb_b_buttonsStatus,BUTTON_WHITE);
  }
  else if ((buttonADC > 660) && (buttonADC < 700)) // Red Button Pressed
  {
    BIT_SET(Vb_b_buttonsStatus,BUTTON_RED);
    BIT_CLEAR(Vb_b_buttonsStatus,BUTTON_WHITE);
    
  }
  else if ((buttonADC > 490) && (buttonADC < 530)) // White Button Pressed
  {
    BIT_CLEAR(Vb_b_buttonsStatus,BUTTON_RED);
    BIT_SET(Vb_b_buttonsStatus,BUTTON_WHITE);
    setDigitalPort(Pin_LEDGREEN, 0, OUTPUT_NORMAL); // turn on LED
  }
  
  else if ((buttonADC > 390) && (buttonADC < 430)) //Both Buttons Pressed
  {
    BIT_SET(Vb_b_buttonsStatus,BUTTON_RED);
    BIT_SET(Vb_b_buttonsStatus,BUTTON_WHITE);
  }
  //else invalid or waiting.

    
  if(configPage1.LEDSEnbl == false) // not using the LEDs for anything else
  {    
    if (bitRead(Vb_b_buttonsStatus,BUTTON_RED) == true)
    {
      setDigitalPort(Pin_LEDRED, 0, OUTPUT_NORMAL); // turn on LED
    }
    else
    {
      setDigitalPort(Pin_LEDRED, 1, OUTPUT_NORMAL); // turn off LED
    }
    
    if (bitRead(Vb_b_buttonsStatus,BUTTON_WHITE) == true)
    {
      setDigitalPort(Pin_LEDGREEN, 0, OUTPUT_NORMAL); // turn on LED
    }
    else
    {
      setDigitalPort(Pin_LEDGREEN, 1, OUTPUT_NORMAL); // turn off LED
    }
  }
  
  if (bitRead(Vb_b_buttonsStatus,BUTTON_WHITE) == true)
  {
    BIT_TOGGLE(Out_TS.Vars.Va_b_SDLoggingStatus, BIT_SDLOG_MANACTIVE);  //toggle logging 
  }
}
