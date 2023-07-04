/*
* storage.ino
* Concerned with writing and reading from non-volitile memory.
*
* Copyright (C) Henry Wright
* A full copy of the license may be found in the projects root directory
*
* Based on code by Josh Stewart for the Speeduino project , and Darren Siepka for the GPIO module see www.Speeduino.com for more info
*/

#include "storage.h"

/* Variables Local to this function */
uint16_t EEPROM_Addr = 0; // current EEPROM address for burn

/* Functions */

//A non-blocking EEPROM update routine called from new 'r' command. Actually it does block but only when multiple burn requests chain together.
void STOR_writeConfigNoBlock(void) 
{
  uint16_t EEPROM_PageStart, EEPROM_PageEnd;
  byte* pnt_configPage;
  
  switch (currentStatus.currentPage)
  {
    case 1:
      pnt_configPage = (uint8_t *)&configPage1; //Create a pointer to Page 1 in memory
      EEPROM_PageStart = EEPROM_CONFIG1_START;
      EEPROM_PageEnd = EEPROM_CONFIG1_END;
    break;
    case 2:
      pnt_configPage = (uint8_t *)&configPage2; //Create a pointer to Page 2 in memory
      EEPROM_PageStart = EEPROM_CONFIG2_START;
      EEPROM_PageEnd = EEPROM_CONFIG2_END;
    break;
    case 3:
      pnt_configPage = (uint8_t *)&configPage3; //Create a pointer to Page 3 in memory
      EEPROM_PageStart = EEPROM_CONFIG3_START;
      EEPROM_PageEnd = EEPROM_CONFIG3_END;
    break;
#if defined(EEPROM_SIZE_8KB)
    case 4:
      pnt_configPage = (uint8_t *)&configPage4; //Create a pointer to Page 4 in memory
      EEPROM_PageStart = EEPROM_CONFIG4_START;
      EEPROM_PageEnd = EEPROM_CONFIG4_END;
    break;
    case 5:
      pnt_configPage = (uint8_t *)&configPage5; //Create a pointer to Page 5 in memory
      EEPROM_PageStart = EEPROM_CONFIG5_START;
      EEPROM_PageEnd = EEPROM_CONFIG5_END;
    break;
#endif
    default:
    break;
  }
    
  while (EEPROM_PageStart + EEPROM_Addr < EEPROM_PageEnd)
  {
    if (eeprom_is_ready())
    {        
      EEPROM.update(EEPROM_PageStart + EEPROM_Addr, *((uint8_t *)pnt_configPage + (uint8_t)(EEPROM_Addr)));
      EEPROM_Addr++;
    }
    else
    {
      break; //break out of loop and wait for eeprom to be ready before resuming
    }
  }
  
  if (EEPROM_PageStart + EEPROM_Addr >= EEPROM_PageEnd) 
  {
    BIT_SET(currentStatus.systembits, BIT_SYSTEM_BURN_GOOD); //set burn_good flag
    EEPROM_Addr = 0;
  }    
}

/*
Takes the current configuration (config pages and maps)
and writes them to EEPROM as per the layout defined in storage.h, this is the old "blocking" routine called using the "B" command
*/
void STOR_writeConfig(uint8_t thePage)
{
  /*
  We only ever write to the EEPROM where the new value is different from the currently stored byte
  This is due to the limited write life of the EEPROM (Approximately 100,000 writes)
  */
  
 // int offset;
  //Create a pointer to the config page
  
  //void* pnt_configPage;//This only stores the address of the value that it's pointing to and not the max size
  //byte* pnt_configPage;

  if(EEPROM.read(0) != data_structure_version) { EEPROM.write(0,data_structure_version); }   //Write the data structure version
 
  switch (thePage)
        {
         case 1:
         /*---------------------------------------------------
         | Config page 1 (See storage.h for data layout)
         | 128 byte long config table
         -----------------------------------------------------*/
         EEPROM.put(EEPROM_CONFIG1_START,configPage1);
         
         // pnt_configPage = (uint8_t *)&configPage1; //Create a pointer to Page 1 in memory
  
         // for(uint16_t x = EEPROM_CONFIG1_START; x < EEPROM_CONFIG1_END ; x++)     //EEPROM_CONFIG1_END 
            // {         
             // if(EEPROM.read(x) != *((uint8_t *)pnt_configPage + (uint8_t)(x - EEPROM_CONFIG1_START))) { EEPROM.write(x, *((uint8_t *)pnt_configPage + (uint8_t)(x - EEPROM_CONFIG1_START))); }        
            // }
  
         break;

         case 2:
         /*---------------------------------------------------
         | Config page 2 (See storage.h for data layout)
         | 705 byte long config table
         -----------------------------------------------------*/
         EEPROM.put(EEPROM_CONFIG2_START,configPage2);
         // pnt_configPage = (uint8_t *)&configPage2; //Create a pointer to Page 2 in memory
  
         // for(uint16_t x=EEPROM_CONFIG2_START; x<EEPROM_CONFIG2_END; x++) 
            // { 
             //if(EEPROM.read(x) != *((uint8_t *)pnt_configPage + (uint16_t)(x - EEPROM_CONFIG2_START))) { EEPROM.write(x, *((uint8_t *)pnt_configPage + (uint16_t)(x - EEPROM_CONFIG2_START))); }
             // if(EEPROM.read(x) != *(pnt_configPage + byte(x - EEPROM_CONFIG2_START))) { EEPROM.write(x, *(pnt_configPage + byte(x - EEPROM_CONFIG2_START)));}
            // }
  
         break;

         case 3:
         /*---------------------------------------------------
         | Config page 3 (See storage.h for data layout)
         | 256 byte long config table
          -----------------------------------------------------*/
         EEPROM.put(EEPROM_CONFIG3_START,configPage3);          
         // pnt_configPage = (uint8_t *)&configPage3; //Create a pointer to Page 3 in memory  
  
         // for(uint16_t x=EEPROM_CONFIG3_START; x<EEPROM_CONFIG3_END; x++) 
            // {       
             // if(EEPROM.read(x) != *((uint8_t *)pnt_configPage + (uint16_t)(x - EEPROM_CONFIG3_START))) { EEPROM.write(x, *((uint8_t *)pnt_configPage + (uint16_t)(x - EEPROM_CONFIG3_START))); }
             //if(EEPROM.read(x) != *(pnt_configPage + byte(x - EEPROM_CONFIG3_START))) { EEPROM.write(x, *(pnt_configPage + byte(x - EEPROM_CONFIG3_START)));}    
            // }
  
         break;
#if defined(EEPROM_SIZE_8KB)
         case 4:
         /*---------------------------------------------------
         | Config page 4 (See storage.h for data layout)
         | 256 byte long config table
         -----------------------------------------------------*/
         EEPROM.put(EEPROM_CONFIG4_START,configPage4);          
         // pnt_configPage = (uint8_t *)&configPage4; //Create a pointer to Page 4 in memory
  
         // for(uint16_t x=EEPROM_CONFIG4_START; x<EEPROM_CONFIG4_END; x++) 
            // { 
             // if(EEPROM.read(x) != *((uint8_t *)pnt_configPage + (uint16_t)(x - EEPROM_CONFIG4_START))) { EEPROM.write(x, *((uint8_t *)pnt_configPage + (uint16_t)(x - EEPROM_CONFIG4_START))); }
            //if(EEPROM.read(x) != *(pnt_configPage + byte(x - EEPROM_CONFIG4_START))) { EEPROM.write(x, *(pnt_configPage + byte(x - EEPROM_CONFIG4_START)));}
            // }
  
         break;
         
         case 5:
         /*---------------------------------------------------
         | Config page 5 (See storage.h for data layout)
         | 512 byte long config table
         -----------------------------------------------------*/ 
         EEPROM.put(EEPROM_CONFIG5_START,configPage5);         
         // pnt_configPage = (uint8_t *)&configPage5; //Create a pointer to Page 5 in memory
  
         // for(uint16_t x=EEPROM_CONFIG5_START; x<EEPROM_CONFIG5_END; x++) 
            // { 
             // if(EEPROM.read(x) != *((uint8_t *)pnt_configPage + (uint16_t)(x - EEPROM_CONFIG5_START))) { EEPROM.write(x, *((uint8_t *)pnt_configPage + (uint16_t)(x - EEPROM_CONFIG5_START))); }
            //if(EEPROM.read(x) != *(pnt_configPage + byte(x - EEPROM_CONFIG5_START))) { EEPROM.write(x, *(pnt_configPage + byte(x - EEPROM_CONFIG5_START)));}
            // }
  
         break;
#endif
       }     

  BIT_SET(currentStatus.systembits, BIT_SYSTEM_BURN_GOOD); //set burn_good flag
}

//---------------------------------------------------------------------------------------------------------------------------------------
/* void STOR_loadConfig(void) Copies all pages from EEPROM into RAM */
void STOR_loadConfig(void)
 {
   EEPROM.get(EEPROM_CONFIG1_START, configPage1);
   EEPROM.get(EEPROM_CONFIG2_START, configPage2);
   EEPROM.get(EEPROM_CONFIG3_START, configPage3);
#if defined(EEPROM_SIZE_8KB)
   EEPROM.get(EEPROM_CONFIG4_START, configPage4);
   EEPROM.get(EEPROM_CONFIG5_START, configPage5);
#endif
   
   BIT_SET(currentStatus.systembits, BIT_SYSTEM_BURN_GOOD); //set burn_good flag since RAM = EEPROM
}

//---------------------------------------------------------------------------------------------------

/* void STOR_eraseEEPROM(void) overwrites all the EEPROM with 0x00 */
void STOR_eraseEEPROM(void)
{
  for (int i = 0 ; i < EEPROM.length() ; i++) {
  EEPROM.update(i, 0);
  }
}
