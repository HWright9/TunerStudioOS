/*
* directcomms.ino
* Concerned with commuications with TunerStudio and applications that support that protocol
*
* Copyright (C) Henry Wright
* A full copy of the license may be found in the projects root directory
*
* Based on code by Josh Stewart for the Speeduino project , and Darren Siepka for the GPIO module see www.Speeduino.com for more info a
*/

/*Variables Local to this module */

uint16_t COMS_readsPerSecCount; // used for calculating the number of serial data reads per sec.
//byte fullStatus[FULLSTATUS_SIZE];
byte currentSerialCmd = 0x00; // Current serial command in progress.
byte Serial_r_Cmnd = false; // if true the 'r' command is currently in progress and awaiting more data.
byte COMS_EEPROMBurnCmnd = false; // if true the burn is currently in progress and awaiting eeprom availabiltiy.

/* Functions */

/*
* This is called when a command is received over serial from TunerStudio
* It parses the command and calls the relevant function.
* The 'A', 'B' etc. commands are reffered to as the 'Legacy commands' which was the older way TS and MegaTune used to communicate. It's robust and easy to test on the bench.
* The 'r' followed by command and data is the newer way and has faster and better optimised routines.
* Both routines work, but new work should use the 'r' commands.
*/
void direct_serial_command()
{
  //uint32_t serialLoopStartMicros = micros();
  uint16_t theOffset, theLength;
  byte tmp;
  
  if (Serial_r_Cmnd == false) { currentSerialCmd = TS_SERIALLink.read(); } // if 'r' is pending more data from TS so stay on this command.
  
    switch (currentSerialCmd)
          {
          case 'A':
                  direct_sendValues(0, configPage1.ochBlockSizeSent, 60);//(offset,packet size lenght,cmd)
                  if(COMS_readsPerSecCount < 65535) { COMS_readsPerSecCount++; }
          break; 
           
          case 'B': // Burn current values to eeprom
                  //A 2nd byte of data is required after the 'P' specifying the new page number.
                  while (TS_SERIALLink.available() == 0) {}
                  currentPage = TS_SERIALLink.read();
                  BIT_CLEAR(Out_TS.Vars.systembits, BIT_SYSTEM_BURN_GOOD); //clear burn_good flag
                  STOR_writeConfig(currentPage);
          break;

          case 'C': // test communications. This is used by Tunerstudio to see whether there is an ECU on a given serial port
                  //      testComm();
          break;

          case 'E': // receive command button commands
                  byte tmp;
                  uint16_t theOffset;
                  while (TS_SERIALLink.available() == 0) {}
                  tmp = TS_SERIALLink.read();
                  while (TS_SERIALLink.available() == 0) {}
                  theOffset = (TS_SERIALLink.read()<<8) | tmp;
                 // theOffset = word(TS_SERIALLink.read(), tmp);
                  commandButtons(theOffset);
          break;
          
          case 'F': // send serial protocol version
                  TS_SERIALLink.print("001");
          break;

          case 'P': // set the current page
                    //A 2nd byte of data is required after the 'P' specifying the new page number.
                  while (TS_SERIALLink.available() == 0) {}
                  currentPage = TS_SERIALLink.read();
                  if (currentPage >= '0') {//This converts the ascii number char into binary
                  currentPage -= '0';
                  }
          break;
      
          case 'Q': // send code version
               // TS_SERIALLink.print("speeduino mini_GPIOV3.003 201711");
                    for (unsigned int sg = 0; sg < sizeof(ECU_signature) - 1; sg++)
                        {
                        TS_SERIALLink.write(ECU_signature[sg]);  
                        }
          break;
          
          case 'S': // send code version
                    for (unsigned int sg = 0; sg < sizeof(ECU_RevNum) - 1; sg++)
                        {
                        TS_SERIALLink.write(ECU_RevNum[sg]);
                        Out_TS.Vars.secl = 0; //This is required in TS3 due to its stricter timings
                        }
          break;

          case 'V': // send VE table and constants in binary
                while (TS_SERIALLink.available() == 0) {}
                tmp = TS_SERIALLink.read();
                while (TS_SERIALLink.available() == 0) {}
                theOffset = (TS_SERIALLink.read()<<8) | tmp;
               // theOffset = word(TS_SERIALLink.read(), tmp);
                while (TS_SERIALLink.available() == 0) {}
                tmp = TS_SERIALLink.read();
                while (TS_SERIALLink.available() == 0) {}
                theLength = (TS_SERIALLink.read()<<8) | tmp;
 
                direct_sendPage(theOffset,theLength,thistsCanId,0);
          break;

          case 'W': // receive new data value at 'W'+<offset>+<newbyte>
                //A 2nd byte of data is required after the 'P' specifying the new page number.
                while (TS_SERIALLink.available() == 0) {}
                tmp = TS_SERIALLink.read();
                while (TS_SERIALLink.available() == 0) {}
                theOffset = (TS_SERIALLink.read()<<8) | tmp;
                         //theOffset = word(TS_SERIALLink.read(), tmp);  this was replaced with the line above as not all cores supported the word option
                while (TS_SERIALLink.available() == 0) {}
                tmp = TS_SERIALLink.read();
                direct_receiveValue(theOffset, tmp);                      // 
                  
          break;


          case 'r':
              byte cmd;
              byte tsCanId_sent; 
              if (TS_SERIALLink.available() >= 6)
              {
                tsCanId_sent = TS_SERIALLink.read(); //Read the $tsCanId
                cmd = TS_SERIALLink.read();

                tmp = TS_SERIALLink.read();
                theOffset = word(TS_SERIALLink.read(), tmp);
                tmp = TS_SERIALLink.read();
                if (cmd == 87) { theLength = tmp; }        //if is "W" length is only 1 byte, ignore other byte
                else { theLength = word(TS_SERIALLink.read(), tmp); }
                if (tsCanId_sent == thistsCanId)        //was the canid sent by TS the same as this device TScanID?
                {
                  dolocal_rCommands(cmd, tsCanId_sent, theOffset, theLength);
                }
                Serial_r_Cmnd = false;
              }
              else
              {
                Serial_r_Cmnd = true;
              }
          break;
    }

//Out_TS.Vars.dev2 = (uint16_t)(micros() - serialLoopStartMicros); 
 
}

void dolocal_rCommands(uint8_t commandletter, uint8_t canid, uint16_t theoffset, uint16_t theLength)
{
  
    switch (commandletter)
           {
           case 15:    //  0x0f 
                    for (unsigned int sg = 0; sg < sizeof(ECU_signature) - 1; sg++)
                        {
                        TS_SERIALLink.write(ECU_signature[sg]);  
                        }
           break;
                        
           case 14:  //   0x0e
                    for (unsigned int sg = 0; sg < sizeof(ECU_RevNum) - 1; sg++)
                        {
                        TS_SERIALLink.write(ECU_RevNum[sg]);
                        Out_TS.Vars.secl = 0; //This is required in TS3 due to its stricter timings
                        }     
           break;
                        
           case 48:    //previously 0x30:
                                // TS_SERIALLink.print("got to 3d");
                                 // direct_sendValues(offset, length, cmd);
           break;
                        
           case 60:  //(0x3c+120 == 0xB4(112dec)):     0x3C  
                   direct_sendValues(theoffset, theLength, 60);
                   if(COMS_readsPerSecCount < 65535) { COMS_readsPerSecCount++; }
           break;

           case 66: // r version of B == 0x42
                    // Burn current values to eeprom
                    if (bitRead(Out_TS.Vars.systembits, BIT_SYSTEM_BURN_GOOD) == false) // Previous burn not complete while another burn command has been issued.
                    {
                      while (bitRead(Out_TS.Vars.systembits, BIT_SYSTEM_BURN_GOOD) == false)
                      { // If multiple burn commands recieved just sit and wait for previous burn to finish. Blocking, but robust.
                        STOR_writeConfigNoBlock(); 
                      }
                    }
                    // If we are here, either after waiting for the previous burn or with no burn pending
                    // COMS_EEPROMBurnCmnd = false and eeprom is good to write next burn command.
                    currentPage = byte(theoffset);
                    BIT_CLEAR(Out_TS.Vars.systembits, BIT_SYSTEM_BURN_GOOD); //clear burn_good flag
                    //STOR_writeConfigNoBlock(); is called from the main loop to check on burn progress.
           break;
           
           case 69: // r version of E == 0x45
                  commandButtons(theoffset);
           break;
                               
           case 80:  //r version of P == 0x50
                  currentPage = byte(theoffset);
           break;
          
           case 86:  //r version of V == 0x56
                  direct_sendPage(theoffset,theLength,thistsCanId,86);
           break;
                    
           case 87:  //r version of W(0x57)
                 // int valueOffset; //cannot use offset as a variable name, it is a reserved word for several teensy libraries
                  direct_receiveValue(theoffset, theLength);  //TS_SERIALLink.read());                    
           break;
       } //closes the switch/case 
}


void direct_receiveValue(uint16_t rvOffset, uint8_t newValue)
{      
        
  void* pnt_configPage;//This only stores the address of the value that it's pointing to and not the max size

  switch (currentPage)
  {

    case 1:
      pnt_configPage = &configPage1; //Setup a pointer to the relevant config page
     //For some reason, TunerStudio is sending offsets greater than the maximum page size. I'm not sure if it's their bug or mine, 
     //but the fix is to only update the config page if the offset is less than the maximum size
      if ( rvOffset < page_1_size)
      {
        *((uint8_t *)pnt_configPage + (uint8_t)rvOffset) = newValue; //
      }
      break;

    case 2:
      pnt_configPage = &configPage2; //Setup a pointer to the relevant config page
     //For some reason, TunerStudio is sending offsets greater than the maximum page size. I'm not sure if it's their bug or mine, 
     //but the fix is to only update the config page if the offset is less than the maximum size
      if ( rvOffset < page_2_size)
      {
        *((uint8_t *)pnt_configPage + (uint16_t)rvOffset) = newValue; //
      }
    break;

    case 3:
      pnt_configPage = &configPage3; //Setup a pointer to the relevant config page
     //For some reason, TunerStudio is sending offsets greater than the maximum page size. I'm not sure if it's their bug or mine,
     //but the fix is to only update the config page if the offset is less than the maximum size
      if ( rvOffset < page_3_size)
      {
        *((uint8_t *)pnt_configPage + (uint16_t)rvOffset) = newValue; //
      }
    break;
#if defined(EEPROM_SIZE_8KB)
    case 4:
      pnt_configPage = &configPage4; //Setup a pointer to the relevant config page
     //For some reason, TunerStudio is sending offsets greater than the maximum page size. I'm not sure if it's their bug or mine,
     //but the fix is to only update the config page if the offset is less than the maximum size
      if ( rvOffset < page_4_size)
      {
        *((uint8_t *)pnt_configPage + (uint16_t)rvOffset) = newValue; //
      }
    break;
          
    case 5:
      pnt_configPage = &configPage5; //Setup a pointer to the relevant config page
     //For some reason, TunerStudio is sending offsets greater than the maximum page size. I'm not sure if it's their bug or mine,
     //but the fix is to only update the config page if the offset is less than the maximum size
      if ( rvOffset < page_5_size)
      {
        *((uint8_t *)pnt_configPage + (uint16_t)rvOffset) = newValue; //
      }
    break;
#endif
  }
}

 //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
sendPage() packs the data within the current page (As set with the 'P' command)
into a buffer and sends it.
Note that some translation of the data is required to lay it out in the way TunerStudio expect it
useChar - If true, all values are send as chars, this is for the serial command line interface. TunerStudio expects data as raw values, so this must be set false in that case
*/
void direct_sendPage(uint16_t send_page_offset, uint16_t send_page_Length, byte can_id, byte cmd)
{

 //currentPage = pagenum;
 
        void* pnt_configPage;

        switch (currentPage)
          {

            case 1:
                {
                // currentTitleIndex = 27;

                pnt_configPage = &configPage1; //Create a pointer to Page 1 in memory  
                  //send_page_Length = page_1_size; 
                }
            break;  

            case 2:
                {
                // currentTitleIndex = 27;

                pnt_configPage = &configPage2; //Create a pointer to Page 2 in memory  
                  //send_page_Length = page_2_size; 
                }
            break;

            case 3:
                {
                pnt_configPage = &configPage3; //Create a pointer to Page 3 in memory  
                  //send_page_Length = page_3_size; 
                }
            break;
#if defined(EEPROM_SIZE_8KB)        
            case 4:
                {
                pnt_configPage = &configPage4; //Create a pointer to Page 4 in memory  
                  //send_page_Length = page_4_size; 
                }
            break;
            
            case 5:
                {
                pnt_configPage = &configPage5; //Create a pointer to Page 5 in memory  
                  //send_page_Length = page_5_size; 
                }
            break;
#endif
          }
    
          //All other bytes can simply be copied from the config table
          
          uint8_t response[send_page_Length];
          for ( uint16_t x = 0; x < send_page_Length; x++)
            {
              response[x] = *((uint8_t *)pnt_configPage +(uint16_t)(send_page_offset)+ (uint16_t)(x)); //Each byte is simply the location in memory of the configPage + the offset(not used) + the variable number (x)
            }

          if (cmd == 206)   //came via passthrough from serial3
            {
#if defined(AUX_SERIAL_ENBL)
              AUX_SERIALLink.print("r");
              AUX_SERIALLink.write(thistsCanId);                //canId of the device you are requesting data from
              AUX_SERIALLink.write(cmd);                       //  
              AUX_SERIALLink.write(0x00);                       // dummy offset lsb
              AUX_SERIALLink.write(0x00);                       // dummy offset msb
              AUX_SERIALLink.write(lowByte(send_page_Length));  // length lsb
              AUX_SERIALLink.write(highByte(send_page_Length)); // length msb
              AUX_SERIALLink.write((uint8_t *)&response, sizeof(response));
#endif              
            }
          else
          {  
          TS_SERIALLink.write((uint8_t *)&response, sizeof(response));
          }
      
}

/*
This function is used to store calibration data sent by Tuner Studio.
*/
void direct_receiveCalibration(byte tableID)
{

}

/*
this function sends the realtime data to TS
*/ 
void direct_sendValues(uint16_t theOffset, uint16_t packetLength, uint8_t cmd)
{
  uint8_t valuesSize =  sizeof(Out_TS_t);
  if (packetLength > 0x00) // Make sure we have some data to send
  {
    //direct_read_realtime();    
    if (packetLength > valuesSize) { packetLength = valuesSize; }
    
    if (cmd == 60) // Respond to console
    {
      for(uint16_t x=theOffset; x < (theOffset + packetLength); x++)
      {
        TS_SERIALLink.write(Out_TS.byteData[x]);
      }
    }
    else if (cmd == 180) // respond to 2nd serial link
    {
#if defined(AUX_SERIAL_ENBL)
      //TS_SERIALLink.print("r was sent");
      AUX_SERIALLink.write("r");         //confirm cmd letter 
      AUX_SERIALLink.write(0x00);           //canid
      AUX_SERIALLink.write(cmd);          //confirm cmd
      AUX_SERIALLink.write(lowByte(theOffset));                       //start theOffset lsb
      AUX_SERIALLink.write(highByte(theOffset));                      //start theOffset msb
      AUX_SERIALLink.write(lowByte(packetLength));      //confirm no of byte to be sent
      AUX_SERIALLink.write(highByte(packetLength));      //confirm no of byte to be sent
#endif
      
      for(uint16_t x=theOffset; x < (theOffset + packetLength); x++)
      {
        TS_SERIALLink.write(Out_TS.byteData[x]);
      } 
    }
  }
}

void commandButtons(uint16_t cmdCombined)
{
  
 // Out_TS.Vars.dev1 = cmdCombined;
  if(cmdCombined == 256)
  {// cmd is stop    
    BIT_CLEAR(Out_TS.Vars.testIO_hardware, BIT_TESTHW_ACTIVE);    //clear testactive flag
    digitalPorts0_15.isOverride = 0; // clear all override flags.
    digitalPorts16_31.isOverride = 0;
    digitalPorts32_47.isOverride = 0;
    digitalPorts48_63.isOverride = 0;
  }

  else if (cmdCombined == 257) // cmd is enable
  { 
    if (configPage1.allowHWTestMode == true)
    {
      BIT_SET(Out_TS.Vars.testIO_hardware, BIT_TESTHW_ACTIVE);  //set testactive flag
    }
  }
  
  else if (cmdCombined == 258) //cmdClearEEPROM
  {
    //Clear EEPROM routine in here. this will write the whole EEPROM with 0x00 and typically take long enough to force tuner studio to attempt re-connection. After it does it will show the differences as cleared.
    if (configPage1.allowEEPROMClear == true)
    {
      noInterrupts();
      #if defined(AVR_WDT)
      wdt_disable();
      #endif
      setDigitalPort(LED_BUILTIN, LOW, OUTPUT_NORMAL);
      STOR_eraseEEPROM();
      STOR_loadConfig();
      while(1){ delay(10000); }  // Wait forever for the user to powercycle the ECU.
    }
  }
  
  else if (cmdCombined == 259) //cmdReInit
  {
    //Re-calls the init routine. sort of a soft reset.
    if (configPage1.allowHWTestMode== true)
    {
      noInterrupts();
      setup();
      interrupts();
    }
  }
  
  // Turn Port to logical HIGH
  else if ((cmdCombined >=513) && (cmdCombined <= 576)) //64 digital ports
  {
    // Turn a port to HIGH state and set override flag
    if(BIT_CHECK(Out_TS.Vars.testIO_hardware, BIT_TESTHW_ACTIVE))
    {
      setDigitalPort((cmdCombined-513), HIGH, OUTPUT_OVERRIDE);
      //BIT_SET(Out_TS.Vars.digOut, (cmdCombined-513));
    }
  }
  
  // Turn Port to logical LOW
  else if ((cmdCombined >=769) && (cmdCombined <= 832)) //64 digital ports
  {
    // Turn a port to LOW state and set override flag
    if(BIT_CHECK(Out_TS.Vars.testIO_hardware, BIT_TESTHW_ACTIVE))
      {
        setDigitalPort((cmdCombined-769), LOW, OUTPUT_OVERRIDE);
        //BIT_CLEAR(Out_TS.Vars.digOut, (cmdCombined-769));
      }
  }
  
  //Can add a switch case under here for more specific commands

}  
