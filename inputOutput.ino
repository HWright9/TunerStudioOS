/*
* inputOutput.ino
* Concerned with functions that are a called to read or set digital and analgue outputs.
*
* Copyright (C) Henry Wright
* A full copy of the license may be found in the projects root directory
*
* Based on code by Josh Stewart for the Speeduino project , and Darren Siepka for the GPIO module see www.Speeduino.com for more info
*/

#include "inputOutput.h"

/* Variables Local to this function*/

/* Functions */

void INIT_ADC()
{
    //Disconnect Digital Pins from ADC for less noise and power consumption
    //DIDR0 = 0×01;
   //This sets the ADC (Analog to Digitial Converter) to run at 1Mhz, greatly reducing analog read times (MAP/TPS) when using the standard analogRead() function
    //1Mhz is the fastest speed permitted by the CPU without affecting accuracy
    //Please see chapter 11 of 'Practical Arduino' (http://books.google.com.au/books?id=HsTxON1L6D4C&printsec=frontcover#v=onepage&q&f=false) for more detail
#if defined(CORE_AVR)
     BIT_SET(ADCSRA,ADPS2);
     BIT_CLEAR(ADCSRA,ADPS1);
     BIT_CLEAR(ADCSRA,ADPS0); 
#endif
}

/* Pin mapping */
void INIT_setPinMapping(void)
{
  //disable all override flags.
  digitalPorts0_15.isOverride = 0; // clear all override flags.
  digitalPorts16_31.isOverride = 0;
  digitalPorts32_47.isOverride = 0;
  digitalPorts48_63.isOverride = 0;
  
  digitalPorts0_15.isOutput = 0; // clear all output flags, to be set below in setPortMode as required.
  digitalPorts16_31.isOutput = 0;
  digitalPorts32_47.isOutput = 0;
  digitalPorts48_63.isOutput = 0;
  
  //system pins
  setPortMode(LED_BUILTIN, OUTPUT);  
  if (configPage1.can0RXIntPin > DPIN_DISABLED) { Pin_can0RXInt = pinTranslate(configPage1.can0RXIntPin); setPortMode(Pin_can0RXInt,INPUT_PULLUP); }
  //if (configPage1.analogSelectorEn == APIN_ENABLED) { Pin_analogSelector = configPage1.analogSelectorPin; }
  
  /* from Setup for EPB */
  // initialize pins
  setPortMode(Df_i_OrangeLEDPin, OUTPUT);
  setPortMode(Df_i_RedLEDPin, OUTPUT);
  setPortMode(Df_i_SeatbeltLEDPin, OUTPUT);
    
  //analogue pins
  pinMode(Df_i_AccelPedalPin, INPUT); 
  pinMode(Df_i_VoltageInputPin, INPUT); 
  pinMode(Df_i_ActuatorPosPin, INPUT); 
  
  //Motor Driver output 1
  setPortMode(Df_i_M1NAPin, OUTPUT);
  setPortMode(Df_i_M1NBPin, OUTPUT);
  setPortMode(Df_i_M1PWMPin, OUTPUT);
  setPortMode(Df_i_M1EN_M1DiagPin, INPUT); //input writing to this enables or disables internal pullup
  setPortMode(Df_i_M1CSPin, INPUT);

  //Motor Driver output 2
  setPortMode(Df_i_M2NAPin, OUTPUT);
  setPortMode(Df_i_M2NBPin, OUTPUT);
  setPortMode(Df_i_M2PWMPin, OUTPUT);
  setPortMode(Df_i_M2EN_M1DiagPin, INPUT); //input writing to this enables or disables internal pullup
  setPortMode(Df_i_M2CSPin, INPUT);
  
  //Cruise control 
  setPortMode(Df_i_CruiseSolenoidPin, OUTPUT);
  
  // Relay Power to controller
  setPortMode(Df_i_PowerControlPin, OUTPUT);

  // Speed inputs
  setPortMode(Df_i_SpeedometerPin, INPUT);
  setPortMode(Df_i_TachometerPin, INPUT);
  
  // 12v to 5V conversion board power and ground
  setPortMode(Df_i_12VBoard5VPin, OUTPUT);
  setPortMode(Df_i_12VBoardGndPin, OUTPUT);
  
  // Digital inputs from various systems
  setPortMode(Df_i_IgnitionPin, INPUT);
  setPortMode(Df_i_ClutchTopPin, INPUT);
  setPortMode(Df_i_BrakeApplyPin, INPUT);
  setPortMode(Df_i_LightsOnPin, INPUT);
  
  //Reversing gearbox inputs
  setPortMode(Df_i_RevBoxFWDPin, INPUT_PULLUP);
  setPortMode(Df_i_RevBoxReversePin, INPUT);
  setPortMode(Df_i_MainBoxNeutralPin, INPUT);
  
  // Dashboard Switch Inputs
  setPortMode(Df_i_SwApplyPin, INPUT);
  setPortMode(Df_i_SwReleasePin, INPUT);
  setPortMode(Df_i_MF_GreenSW_Pin, INPUT);
  setPortMode(Df_i_MF_RedSW_Pin, INPUT);
  
  //Attach interrupts:
  attachInterrupt(5, Tacho_ISR, FALLING); //attach interrupt to int5 (tacho input)
  attachInterrupt(4, Speedo_ISR, CHANGE); //attach interrupt to int4 (speedo input)
  interrupts();  //enable interrupts //noInterrupts ();
  
}

/* Pin default positions */
void INIT_pinDefaults(void);
{
  // Provide power and ground to 12V-5V input board
  setDigitalPort(Df_i_12VBoard5VPin, HIGH, false);
  setDigitalPort(Df_i_12VBoardGndPin, LOW, false);
  
  //switch power control to battery power
  setDigitalPort(Df_i_PowerControlPin, HIGH, false); //high is on
  
  //Init the motor drivers, these pins double as enable and diag ports and are both inputs and outputs.
  setDigitalPort(Df_i_M2EN_M1DiagPin, HIGH, false);
  setDigitalPort(Df_i_M2EN_M2DiagPin, HIGH, false);
  
  //Default LED conditions are on to perform bulb test
  setDigitalPort(Df_i_RedLEDPin, LOW, false); //low is on
  setDigitalPort(Df_i_OrangeLEDPin, LOW, false); //low is on
  setDigitalPort(Df_i_SeatbeltLEDPin, LOW, false); //low is on  
  
  //init motor control 1
  setDigitalPort(Df_i_M1NAPin, LOW, false);
  setDigitalPort(Df_i_M1NBPin, LOW, false);
    
  //init motor control 2
  setDigitalPort(Df_i_M2NAPin, LOW, false);
  setDigitalPort(Df_i_M2NBPin, LOW, false);

  // init Cruise
  setDigitalPort(Df_i_CruiseSolenoidPin, LOW, false);
}

uint16_t readAnalog(uint8_t AinCH)
{
  uint16_t tempReading = 0;
  
  if (AinCH < 16)
  {
    tempReading = analogRead(pinTranslateAnalog(AinCH));   //read the adc channel
    #if defined(ARDUINO_AVR_MEGA2560)
    tempReading = analogRead(pinTranslateAnalog(AinCH));   //read it a second time to get a more stable /faster read
    #endif
    
    #if defined(MCU_STM32F103C8)
    tempReading >>= 2;  //rescales to max 1024 value else would be 0-4096
    #endif
    Out_TS.Vars.Analog[AinCH] = tempReading;
  }
    
return tempReading;
}

/* Read the value of the digital input pin and align digialPorts variable for datalogging *
* function will return the value of the overriden variable if the pin is currently being overridden
* returns: 0 = LOW (or out of range of digital pins, 1 = HIGH */
uint8_t readDigitalPort(uint8_t Dpin)
{
  uint8_t pinValue = 0;
  if (Dpin < BOARD_MAX_IO_PINS)
  {
    if (Dpin < 16)
    {
      if (bitRead(digitalPorts0_15.isOverride, Dpin) == true)
      { 
        pinValue = bitRead(digitalPorts0_15.value, Dpin); 
      }
      else 
      { 
        pinValue = digitalRead(Dpin); 
        bitWrite(digitalPorts0_15.value, Dpin, pinValue); 
      }
    }
    else if (Dpin < 32)
    {
      if (bitRead(digitalPorts16_31.isOverride, Dpin-16) == true)
      { 
        pinValue = bitRead(digitalPorts16_31.value, Dpin-16); 
      }
      else 
      { 
        pinValue = digitalRead(Dpin); 
        bitWrite(digitalPorts16_31.value, Dpin-16, pinValue); 
      }
    }
    else if (Dpin < 48)
    {
      if (bitRead(digitalPorts32_47.isOverride, Dpin-32) == true)
      { 
        pinValue = bitRead(digitalPorts32_47.value, Dpin-32); 
      }
      else 
      { 
        pinValue = digitalRead(Dpin); 
        bitWrite(digitalPorts32_47.value, Dpin-32, pinValue); 
      }
    }
    else if (Dpin < 64)
    {
      if (bitRead(digitalPorts48_63.isOverride, Dpin-48) == true)
      { 
        pinValue = bitRead(digitalPorts48_63.value, Dpin-48); 
      }
      else 
      { 
        pinValue = digitalRead(Dpin); 
        bitWrite(digitalPorts48_63.value, Dpin-48, pinValue); 
      }
    }
  }
  //else pinValue is not configured as an output (out of pin range)
  
  //Align output data to TS
  Out_TS.Vars.digitalPorts0_15_out = digitalPorts0_15.value;
  Out_TS.Vars.digitalPorts16_31_out = digitalPorts16_31.value;
  Out_TS.Vars.digitalPorts32_47_out = digitalPorts32_47.value;
  Out_TS.Vars.digitalPorts48_63_out = digitalPorts48_63.value;
  
  return pinValue;
}


/* Set the digital port as commanded by value and align the digialPorts variable for datalogging *
* This works for both input pins and output pins. When it's an input pin the only operation is to decide if the override bit becomes set.
* override bits are cleared elsewhere in the program.
* function will error if the pin is not configured as an output or out of the valid range of ports
* setOverride will enable the output to be changed even if user overrides are active. Normal code should not set this.
* returns: 0 = write successful, 1 = failed */
uint8_t setDigitalPort(uint8_t Dpin, uint8_t value, uint8_t setOverride)
{
  if (Dpin < BOARD_MAX_IO_PINS)
  {
    if (Dpin < 16)
    {
      if (bitRead(digitalPorts0_15.isOutput, Dpin) == true) 
      { // Port is set as Output
        if ((bitRead(digitalPorts0_15.isOverride, Dpin) == true) && (setOverride == false)) { return 0; } // value overriden by existing override
        else 
        { // isOverride is false and Either we wish to set the override, or we are not using the override.
          digitalWrite(Dpin, value); 
          bitWrite(digitalPorts0_15.value, Dpin, value);
          bitWrite(digitalPorts0_15.isOverride, Dpin, setOverride); 
        }
      }
      else if (setOverride == true)
      { // Port is set as input and now requested to be overriden
        bitWrite(digitalPorts0_15.isOverride, Dpin, setOverride); // This is cleared elsewhere. 
        bitWrite(digitalPorts0_15.value, Dpin, value);
      }
    }
    else if (Dpin < 32)
    {
      if (bitRead(digitalPorts16_31.isOutput, Dpin-16) == true) 
      { // Port is set as Output
        if ((bitRead(digitalPorts16_31.isOverride, Dpin-16) == true) && (setOverride == false)) { return 0; } // value overriden by existing override
        else 
        { // isOverride is false and Either we wish to set the override, or we are not using the override.
          digitalWrite(Dpin, value); 
          bitWrite(digitalPorts16_31.value, Dpin-16, value);
          bitWrite(digitalPorts16_31.isOverride, Dpin-16, setOverride); 
        }
      }
      else if (setOverride == true)
      { // Port is set as input and now requested to be overriden
        bitWrite(digitalPorts16_31.isOverride, Dpin-16, setOverride); // This is cleared elsewhere. 
        bitWrite(digitalPorts16_31.value, Dpin-16, value);
      }
    }
    else if (Dpin < 48)
    {
      if (bitRead(digitalPorts32_47.isOutput, Dpin-32) == true) 
      { // Port is set as Output
        if ((bitRead(digitalPorts32_47.isOverride, Dpin-32) == true) && (setOverride == false)) { return 0; } // value overriden by existing override
        else 
        { // isOverride is false and Either we wish to set the override, or we are not using the override.
          digitalWrite(Dpin, value); 
          bitWrite(digitalPorts32_47.value, Dpin-32, value);
          bitWrite(digitalPorts32_47.isOverride, Dpin-32, setOverride); 
        }
      }
      else if (setOverride == true)
      { // Port is set as input and now requested to be overriden
        bitWrite(digitalPorts32_47.isOverride, Dpin-32, setOverride); // This is cleared elsewhere. 
        bitWrite(digitalPorts32_47.value, Dpin-32, value);
      }
    }
    else if (Dpin < 64)
    {
      if (bitRead(digitalPorts48_63.isOutput, Dpin-48) == true) 
      { // Port is set as Output
        if ((bitRead(digitalPorts48_63.isOverride, Dpin-48) == true) && (setOverride == false)) { return 0; } // value overriden by existing override
        else 
        { // isOverride is false and Either we wish to set the override, or we are not using the override.
          digitalWrite(Dpin, value); 
          bitWrite(digitalPorts48_63.value, Dpin-48, value);
          bitWrite(digitalPorts48_63.isOverride, Dpin-48, setOverride); 
        }
      }
      else if (setOverride == true)
      { // Port is set as input and now requested to be overriden
        bitWrite(digitalPorts48_63.isOverride, Dpin-48, setOverride); // This is cleared elsewhere. 
        bitWrite(digitalPorts48_63.value, Dpin-48, value);
      }
    }
    else{ return 1; } // error Output not in allowed pin range

    return 0; // Output within allowed pin range
  }
  else { return 1; } // error Output not in allowed pin range
    
}

/* Set the digital pin as input, input pullup or output *
* function will error if out of the valid range of ports
* pinmode is standard Arduino notation of OUTPUT, INPUT or INPUT_PULLUP 
* returns: 0 = set successful, 1 = failed */
uint8_t setPortMode(uint8_t Dpin, uint8_t pinmode)
{
  if (Dpin < BOARD_MAX_IO_PINS)
  {
    pinMode(Dpin, pinmode);
    if (Dpin < 16)
    {
      if (pinmode == OUTPUT) { bitWrite(digitalPorts0_15.isOutput, Dpin, true); }
      else { bitWrite(digitalPorts0_15.isOutput, Dpin, false); }
    }
    else if (Dpin < 32)
    {
      if (pinmode == OUTPUT) { bitWrite(digitalPorts16_31.isOutput, Dpin-16, true); }
      else { bitWrite(digitalPorts16_31.isOutput, Dpin-16, false); }
    }
    else if (Dpin < 48)
    {
      if (pinmode == OUTPUT) { bitWrite(digitalPorts32_47.isOutput, Dpin-32, true); }
      else { bitWrite(digitalPorts32_47.isOutput, Dpin-32, false); }
    }
    else if (Dpin < 64)
    {
      if (pinmode == OUTPUT) { bitWrite(digitalPorts48_63.isOutput, Dpin-48, true); }
      else { bitWrite(digitalPorts48_63.isOutput, Dpin-48, false); }
    }
    else { return 1; }
  }
  else { return 1; }
  
  return 0;
}

/* Set the digital pin as being in the override state *
* function will error if out of the valid range of ports 
* isOverride is true or false
* returns: 0 = set successful, 1 = failed */
uint8_t setPortOverride(uint8_t Dpin, uint8_t isOverride)
{
  if (Dpin < BOARD_MAX_IO_PINS)
  {
    if (Dpin < 16)
    {
      if (isOverride == true) { bitWrite(digitalPorts0_15.isOverride, Dpin, true); }
      else { bitWrite(digitalPorts0_15.isOverride, Dpin, false); }
    }
    else if (Dpin < 32)
    {
      if (isOverride == true) { bitWrite(digitalPorts16_31.isOverride, Dpin-16, true); }
      else { bitWrite(digitalPorts16_31.isOverride, Dpin-16, false); }
    }
    else if (Dpin < 48)
    {
      if (isOverride == true) { bitWrite(digitalPorts32_47.isOverride, Dpin-32, true); }
      else { bitWrite(digitalPorts32_47.isOverride, Dpin-32, false); }
    }
    else if (Dpin < 64)
    {
      if (isOverride == true) { bitWrite(digitalPorts48_63.isOverride, Dpin-48, true); }
      else { bitWrite(digitalPorts48_63.isOverride, Dpin-48, false); }
    }
    else { return 1; }
  }
  else { return 1; }
  
  return 0;
}
