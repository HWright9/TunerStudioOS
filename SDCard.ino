/*
* SDCard.ino
* Concerned with functions related to SD card reading and writing.
*
* Copyright (C) Henry Wright
* A full copy of the license may be found in the projects root directory
*
* Based on code by Josh Stewart for the Speeduino project , and Darren Siepka for the GPIO module see www.Speeduino.com for more info a
*/

/* Variables Local to this function*/


/* Functions*/

void INIT_SDCARD(void)
{
  if (configPage1.SD_CardEnbl == true)
  {
    
    if (!SD.begin(Pin_SDCardCS)) 
    {
      Ve_e_SDFileStatus = SDFILE_ERR;
    }
    else // SD init ok
    { 
      
      if (SD.exists("/logging") == false)
      {
        SD.mkdir("/logging");
      }
      Ve_e_SDFileStatus = SDFILE_CLOSED;
    }
  }
  
  
}
