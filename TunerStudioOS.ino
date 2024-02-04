/*
* TunerStudio_OS_dev.ino
*
* Tuner Studio Operating System Development Copyright (C) Henry Wright
* Based on the works of GPIO STD V1.000.
* GPIO STD V1.000 is Copyright (C) Darren Siepka 
* Speeduino is Copyright (C) Josh Stewart
*
* A full copy of the license may be found in the projects root directory 
* A full copy of the license may be found in the speeduino projects root directory
*
*/

//https://developer.mbed.org/handbook/C-Data-Types
#include <stdint.h>
#include <Arduino.h>
//************************************************

#include "globals.h"
#include "timers.h"
#include "init.h"
#include "utils.h"
#include "inputOutput.h"
#include "directcomms.h"
#include "storage.h"
#include "canbus.h"
#include "tableInterp.h"
#include "userfunctions.h"

MCP_CAN CAN0(CAN0_CS);      // Set MCP_CAN CAN0 instance CS to pin 11

struct config1 configPage1;
struct config2 configPage2;
struct config3 configPage3;
#if defined(EEPROM_SIZE_8KB)
struct config4 configPage4;
struct config5 configPage5;
#endif

HardwareSerial &TS_SERIALLink  = Serial;   // setup which serial port connects to TS

#if defined(AUX_SERIAL_ENBL)
    HardwareSerial &AUX_SERIALLink = Serial3; // setup which serial port connects to the speeduino secondary serial
#endif 

uint8_t Ve_t_WarningTimeoutTmr_100ms; // timer with 100ms resolution for warning light timeout.
volatile uint16_t mainLoopCount;

void setup() {
  
  Out_TS.Vars.systembits = 0; // clear all system bits.
  STOR_loadConfig();            
  INIT_setPinMapping();
  INIT_ADC();
  INIT_can0();     //init can interface 0 
  INIT_readOnlyVars();

  TS_SERIALLink.begin(115200);
#if defined(AUX_SERIAL_ENBL)
  AUX_SERIALLink.begin(115200);
#endif  
  
  mainLoopCount = 0;
  Out_TS.Vars.secl = 0;
  BIT_SET(Out_TS.Vars.systembits, BIT_SYSTEM_READY); //set system ready flag
  
  INIT_timers(); //init timers at the end so that ECU timing is good.
}

/* ------------------ MAIN OS TASK SCHEDULER ------------------ */
// put your main code here, to run repeatedly:
void loop()
{
  uint8_t Le_Cnt_TimedTasksThisLoop = 0;
  
  if (BIT_CHECK(TIMR_LoopTmrsBits, BIT_TIMER_5MS)) // 200 hertz
  {
    Le_Cnt_TimedTasksThisLoop++;
    #if defined(AVR_WDT)
    // Having the watchdog here will allow checking for interrupt failure and main loop failure.
    wdt_reset(); // reset the watchdog
    #endif 
    FUNC_5msTask();
    BIT_CLEAR(TIMR_LoopTmrsBits, BIT_TIMER_5MS);
  }

  if (BIT_CHECK(TIMR_LoopTmrsBits, BIT_TIMER_20MS)) //50 hertz
  {
    Le_Cnt_TimedTasksThisLoop++;
    FUNC_20msTask();    
    BIT_CLEAR(TIMR_LoopTmrsBits, BIT_TIMER_20MS);        
  }

  if (BIT_CHECK(TIMR_LoopTmrsBits, BIT_TIMER_50MS)) //20 hertz
  {
    Le_Cnt_TimedTasksThisLoop++;
    FUNC_50msTask(); 
    BIT_CLEAR(TIMR_LoopTmrsBits, BIT_TIMER_50MS);                         
  }
  
  if (BIT_CHECK(TIMR_LoopTmrsBits, BIT_TIMER_75MS)) //13.33 hertz
  {
    Le_Cnt_TimedTasksThisLoop++;
    FUNC_75msTask(); 
    BIT_CLEAR(TIMR_LoopTmrsBits, BIT_TIMER_75MS);                         
  }
  
  if (BIT_CHECK(TIMR_LoopTmrsBits, BIT_TIMER_100MS)) //10 hertz
  {
    Le_Cnt_TimedTasksThisLoop++;
    FUNC_100msTask();
    BIT_CLEAR(TIMR_LoopTmrsBits, BIT_TIMER_100MS);                         
  }  

  if (BIT_CHECK(TIMR_LoopTmrsBits, BIT_TIMER_250MS)) //4 hertz
  {
    Le_Cnt_TimedTasksThisLoop++;    
    FUNC_250msTask();
    BIT_CLEAR(TIMR_LoopTmrsBits, BIT_TIMER_250MS);                         
  }

  if (BIT_CHECK(TIMR_LoopTmrsBits, BIT_TIMER_500MS)) //2 hertz
  {
    Le_Cnt_TimedTasksThisLoop++;    
    FUNC_500msTask();
    BIT_CLEAR(TIMR_LoopTmrsBits, BIT_TIMER_500MS);                         
  }   
 
  if (BIT_CHECK(TIMR_LoopTmrsBits, BIT_TIMER_1000MS)) //1 hertz
  {
    Le_Cnt_TimedTasksThisLoop++;
    FUNC_1000msTask();
    BIT_CLEAR(TIMR_LoopTmrsBits, BIT_TIMER_1000MS);
  } 
 
  /* Serial Comms */
  /* This is a do-while-nothing-else-happening task to avoid serial sending affecting other timed tasks */
  if (Le_Cnt_TimedTasksThisLoop == 0)
  {   
#if defined  (AUX_SERIAL_ENBL)     
    if (AUX_SERIALLink.available() > 0)      // if AUX_SERIALLink has data then do the remote serial command subroutine
    {
      //remote_serial_command();
    }
#endif

    if (TS_SERIALLink.available() > 0)      // if TS_SERIALLink has data then do the direct serial command subroutine(Typical TS link)
    {
      direct_serial_command();
    }

    if (bitRead(Out_TS.Vars.canstatus, BIT_CANSTATUS_CAN0ACTIVATED) == true)
    {
      if(digitalRead(Pin_can0RXInt) == LOW )   // If CAN0_INT pin is low, read receive buffer
      {
        CAN0_INT_routine() ;
      }
    }
  }
  
  /* these next two serial checks are untimed and designed to catch the serial buffer overflow if for some reason there isn't spare time in the main loop. */ 
#if defined  (AUX_SERIAL_ENBL) 
  if (AUX_SERIALLink.available() > 32) 
  {
    //remote_serial_command();   -Implementation TODO             
  }
#endif     
        
  if (TS_SERIALLink.available() > 32) 
  {
    direct_serial_command();
  }
  
  /* Check if we need to write the EEPROM */
  if (bitRead(Out_TS.Vars.systembits, BIT_SYSTEM_BURN_GOOD) == false)
    {
      STOR_writeConfigNoBlock();
    }

  if (mainLoopCount < 65536) { mainLoopCount++; } // update loop counter.
  
}

/* ------------------ USER FUNCTIONS BELOW THIS POINT ------------------ */

/* -------- TIMED TASKS -------- */
/*
* Function: 5msTask() everything in this function runs once every 5ms, 200Hz
* Returns: none
*/
void FUNC_5msTask(void)
{
  canBroadcast_5ms();
}

/*
* Function: 20msTask() everything in this function runs once every 20ms, 50Hz
* Returns: none
*/
void FUNC_20msTask(void)
{
  canBroadcast_20ms();
} //END 20ms Task

/*
* Function: 50msTask() everything in this function runs once every 50ms, 20Hz
* Returns: none
*/
void FUNC_50msTask(void)
{
  canBroadcast_50ms();
} //END 50ms Task

/*
* Function: 75msTask() everything in this function runs once every 75ms, 13.33Hz
* Returns: none
*/
void FUNC_75msTask(void)
{
 
} //END 75ms Task

/*
* Function: 100msTask() everything in this function runs once every 100ms, 10Hz
* Returns: none
*/
void FUNC_100msTask(void)
{  
  canBroadcast_100ms();
  
  recieveCAN_Timeouts();
  
  SYS_setWarningBit();
  
  // Example of floating points
  Out_TS.Vars.Vf_i_TestFloatOut = configPage2.Kf_i_TestFloat1 + configPage2.Kf_i_TestFloat2;
  // Example of 2d 8bit Table lookup
  Out_TS.Vars.dev4 = u8_table2DLookup_u8(configPage2.exampleTable_Xaxis, configPage2.exampleTable_Ydata, sizeof(configPage2.exampleTable_Xaxis), configPage2.exampleLookupValue);
  // Example of 3D 16bit Table lookup
  
  Out_TS.Vars.Ve_i_example3DXLookup = configPage2.example3DXLookup; // this is for sending axes lookup constants back to TS which can only view output chanels as table axis lookup values. Normally you would just have the axis lookup variable in the output chanels.
  Out_TS.Vars.Ve_i_example3DYLookup = configPage2.example3DYLookup;
  Out_TS.Vars.dev3 = u8_table3DLookup_u8(configPage2.example3DTable_Xaxis, configPage2.example3DTable_Yaxis, configPage2.example3DTable_Zdata, 
                                           sizeof(configPage2.example3DTable_Xaxis), sizeof(configPage2.example3DTable_Yaxis), 
                                           configPage2.example3DXLookup, configPage2.example3DYLookup );
  USER_InputOutput();
    
}  //END 100ms Task

/*
* Function: 250msTask() everything in this function runs once every 250ms, 4Hz
* Returns: none
*/
void FUNC_250msTask(void)
{
  

} //END 250ms Task

/*
* Function: 500msTask() everything in this function runs once every 500ms, 2Hz
* Returns: none
*/
void FUNC_500msTask(void)
{
  canBroadcast_500ms();
  USER_blinkCEL(); // blink the build in LED for heartbeat. 
} //END 500ms Task

/*
* Function: 1000msTask() everything in this function runs once every 1000ms, 1Hz
* Returns: none
*/
void FUNC_1000msTask(void)
{
  CAN0_maintenance();
  canBroadcast_1000ms();
  
  Out_TS.Vars.readsPerSecond = COMS_readsPerSecCount;
  COMS_readsPerSecCount = 0;

} //END 1000ms Task

/*
* Function: INIT_readOnlyVars this function runs once on startup to update read only variables relating to memory
* These are saved into config1 so that tuner studio can read them and display them as read only. 
* This a way to get the size of pages and serial data stream into tuner studio for development and error checking.
* It can be removed from developed programs.
* Returns: none
*/
void INIT_readOnlyVars(void)
{
  
  //This sends the ochBlockSize variable that is used in TunerStudio to know how long the serial data stream is going to be.
  configPage1.ochBlockSizeSent = sizeof(Out_TS_t);
  
  //These run once to calculate the page sizes.
  configPage1.page1ActualSize = sizeof(configPage1);
  configPage1.page1CRC = 0;
  STOR_writeConfig(1); // re-write the config in case any of the above changes. normally this does nothing because the EEPROM will skip all the variables that are the same.
  
  configPage2.page2ActualSize = sizeof(configPage2);
  configPage2.page2CRC = 0;
  STOR_writeConfig(2); // re-write the config in case any of the above changes. normally this does nothing because the EEPROM will skip all the variables that are the same.
  
  configPage3.page3ActualSize = sizeof(configPage3);
  configPage3.page3CRC = 0;
  STOR_writeConfig(3); // re-write the config in case any of the above changes. normally this does nothing because the EEPROM will skip all the variables that are the same.
#if defined(EEPROM_SIZE_8KB)
  configPage4.page4ActualSize = sizeof(configPage4);
  configPage4.page4CRC = 0;
  STOR_writeConfig(4); // re-write the config in case any of the above changes. normally this does nothing because the EEPROM will skip all the variables that are the same.
  
  configPage5.page5ActualSize = sizeof(configPage5);
  configPage5.page5CRC = 0;
  STOR_writeConfig(5); // re-write the config in case any of the above changes. normally this does nothing because the EEPROM will skip all the variables that are the same.
#endif
}

/*
* Function: SYS_setWarningBit this function collects any system warnings together into one handy bit to have on the main screen.
* Think of it as a "master caution" light
* Called: 100ms task.
* Returns: none
*/
void SYS_setWarningBit(void)
{
  //The following can be used to show the amount of free memory
  Out_TS.Vars.UTIL_freeRam = UTIL_freeRam();
  
  if ((TIMR_LoopDlyWarnBits > 0) || // any of the loop timer overflow warnings are set
      (Out_TS.Vars.UTIL_freeRam < 500) || // running out of RAM
      (Out_TS.Vars.loopsPerSecond < 200)) // slow loops
  {
    BIT_SET(Out_TS.Vars.systembits, BIT_SYSTEM_WARNING); //set system warning flag
    Ve_t_WarningTimeoutTmr_100ms = 0; 
  }
  else if (Ve_t_WarningTimeoutTmr_100ms < 20) // 20 is 20*100ms = 2sec
  { 
    Ve_t_WarningTimeoutTmr_100ms++;  // incriment timer until timeout.
  } 
  else 
  { 
    BIT_CLEAR(Out_TS.Vars.systembits, BIT_SYSTEM_WARNING); //clear system warning flag after timer expired
  }
}

/* -------- USER ISR VECTORS -------- */
