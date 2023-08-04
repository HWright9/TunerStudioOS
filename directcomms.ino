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
byte fullStatus[FULLSTATUS_SIZE];
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
                  direct_sendValues(0, FULLSTATUS_SIZE, 60);//(offset,packet size lenght,cmd)
                  if(COMS_readsPerSecCount < 65535) { COMS_readsPerSecCount++; }
          break; 
           
          case 'B': // Burn current values to eeprom
                  //A 2nd byte of data is required after the 'P' specifying the new page number.
                  while (TS_SERIALLink.available() == 0) {}
                  currentStatus.currentPage = TS_SERIALLink.read();
                  BIT_CLEAR(currentStatus.systembits, BIT_SYSTEM_BURN_GOOD); //clear burn_good flag
                  STOR_writeConfig(currentStatus.currentPage);
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
                  currentStatus.currentPage = TS_SERIALLink.read();
                  if (currentStatus.currentPage >= '0') {//This converts the ascii number char into binary
                  currentStatus.currentPage -= '0';
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
                        currentStatus.secl = 0; //This is required in TS3 due to its stricter timings
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

//currentStatus.dev2 = (uint16_t)(micros() - serialLoopStartMicros); 
 
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
                        currentStatus.secl = 0; //This is required in TS3 due to its stricter timings
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
                    if (bitRead(currentStatus.systembits, BIT_SYSTEM_BURN_GOOD) == false) // Previous burn not complete while another burn command has been issued.
                    {
                      while (bitRead(currentStatus.systembits, BIT_SYSTEM_BURN_GOOD) == false)
                      { // If multiple burn commands recieved just sit and wait for previous burn to finish. Blocking, but robust.
                        STOR_writeConfigNoBlock(); 
                      }
                    }
                    // If we are here, either after waiting for the previous burn or with no burn pending
                    // COMS_EEPROMBurnCmnd = false and eeprom is good to write next burn command.
                    currentStatus.currentPage = byte(theoffset);
                    BIT_CLEAR(currentStatus.systembits, BIT_SYSTEM_BURN_GOOD); //clear burn_good flag
                    //STOR_writeConfigNoBlock(); is called from the main loop to check on burn progress.
           break;
           
           case 69: // r version of E == 0x45
                  commandButtons(theoffset);
           break;
                               
           case 80:  //r version of P == 0x50
                  currentStatus.currentPage = byte(theoffset);
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

  switch (currentStatus.currentPage)
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

        switch (currentStatus.currentPage)
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
 this function reads the realtime data into the fullStatus array
*/
 void direct_read_realtime()
 {
  fullStatus[0] = currentStatus.secl; //secl is simply a counter that increments each second. Used to track unexpected resets (Which will reset this count to 0)
  fullStatus[1] = currentStatus.systembits; //Squirt Bitfield
  fullStatus[2] = currentStatus.canstatus;  //canstatus Bitfield
  fullStatus[3] = lowByte(currentStatus.loopsPerSecond);
  fullStatus[4] = highByte(currentStatus.loopsPerSecond);
  fullStatus[5] = lowByte(currentStatus.UTIL_freeRam);
  fullStatus[6] = highByte(currentStatus.UTIL_freeRam);
  fullStatus[7] = lowByte(mainLoopCount);
  fullStatus[8] = highByte(mainLoopCount);
  fullStatus[9] = TIMR_LoopDlyWarnBits; // Bitfield of warnings that a task was delayed to the point where it was skipped.
  fullStatus[10] = 0x00; // Spare was .dev1
  fullStatus[11] = lowByte(currentStatus.dev2);
  fullStatus[12] = highByte(currentStatus.dev2);
  fullStatus[13] = currentStatus.testIO_hardware;
  fullStatus[14] = lowByte(digitalPorts0_15.value);
  fullStatus[15] = highByte(digitalPorts0_15.value);
  fullStatus[16] = lowByte(digitalPorts16_31.value);
  fullStatus[17] = highByte(digitalPorts16_31.value);
  fullStatus[18] = lowByte(digitalPorts32_47.value);    
  fullStatus[19] = highByte(digitalPorts32_47.value);
  fullStatus[20] = lowByte(digitalPorts48_63.value);    
  fullStatus[21] = highByte(digitalPorts48_63.value);
  fullStatus[22] = lowByte(currentStatus.Analog[0]);
  fullStatus[23] = highByte(currentStatus.Analog[0]);
  fullStatus[24] = lowByte(currentStatus.Analog[1]);
  fullStatus[25] = highByte(currentStatus.Analog[1]);
  fullStatus[26] = lowByte(currentStatus.Analog[2]);
  fullStatus[27] = highByte(currentStatus.Analog[2]);
  fullStatus[28] = lowByte(currentStatus.Analog[3]);
  fullStatus[29] = highByte(currentStatus.Analog[3]);  
  fullStatus[30] = lowByte(currentStatus.Analog[4]);
  fullStatus[31] = highByte(currentStatus.Analog[4]);
  fullStatus[32] = lowByte(currentStatus.Analog[5]);
  fullStatus[33] = highByte(currentStatus.Analog[5]);
  fullStatus[34] = lowByte(currentStatus.Analog[6]);
  fullStatus[35] = highByte(currentStatus.Analog[6]);
  fullStatus[36] = lowByte(currentStatus.Analog[7]);
  fullStatus[37] = highByte(currentStatus.Analog[7]);
  fullStatus[38] = lowByte(currentStatus.Analog[8]);
  fullStatus[39] = highByte(currentStatus.Analog[8]);
  fullStatus[40] = lowByte(currentStatus.Analog[9]);
  fullStatus[41] = highByte(currentStatus.Analog[9]);
  fullStatus[42] = lowByte(currentStatus.Analog[10]);
  fullStatus[43] = highByte(currentStatus.Analog[10]);
  fullStatus[44] = lowByte(currentStatus.Analog[11]);
  fullStatus[45] = highByte(currentStatus.Analog[11]);  
  fullStatus[46] = lowByte(currentStatus.Analog[12]);
  fullStatus[47] = highByte(currentStatus.Analog[12]);
  fullStatus[48] = lowByte(currentStatus.Analog[13]);
  fullStatus[49] = highByte(currentStatus.Analog[13]);
  fullStatus[50] = lowByte(currentStatus.Analog[14]);
  fullStatus[51] = highByte(currentStatus.Analog[14]);
  fullStatus[52] = lowByte(currentStatus.Analog[15]);
  fullStatus[53] = highByte(currentStatus.Analog[15]);    
  //Vf_i_TestFloatOut
  fullStatus[54] = VS_serialData.serialPacket[0];
  fullStatus[55] = VS_serialData.serialPacket[1];
  fullStatus[56] = VS_serialData.serialPacket[2];
  fullStatus[57] = VS_serialData.serialPacket[3];
  fullStatus[58] = VS_serialData.serialPacket[4]; //Ve_i_TestByte1
  fullStatus[59] = lowByte(currentStatus.dev1);
  fullStatus[60] = highByte(currentStatus.dev1);
  fullStatus[61] = lowByte(currentStatus.dev3);
  fullStatus[62] = highByte(currentStatus.dev3);
  fullStatus[63] = lowByte(currentStatus.dev4);
  fullStatus[64] = highByte(currentStatus.dev4);
  fullStatus[65] = lowByte(currentStatus.readsPerSecond);
  fullStatus[66] = highByte(currentStatus.readsPerSecond);
  
  
 }

/*
this function sends the realtime data to TS
*/ 
void direct_sendValues(uint16_t theOffset, uint16_t packetLength, uint8_t cmd)
{
  if (packetLength > 0x00) // Make sure we have some data to send
  {
    direct_read_realtime();    
    if (packetLength > FULLSTATUS_SIZE) { packetLength = FULLSTATUS_SIZE; }
    
    if (cmd == 60) // Respond to console
    {
      for(uint16_t x=theOffset; x < (theOffset + packetLength); x++)
      {
        TS_SERIALLink.write(fullStatus[x]);
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
        TS_SERIALLink.write(fullStatus[x]);
      } 
    }
  }
}

void commandButtons(uint16_t cmdCombined)
{
  
 // currentStatus.dev1 = cmdCombined;
  if(cmdCombined == 256)
  {// cmd is stop    
    BIT_CLEAR(currentStatus.testIO_hardware, BIT_TESTHW_ACTIVE);    //clear testactive flag
    digitalPorts0_15.isOverride = 0; // clear all override flags.
    digitalPorts16_31.isOverride = 0;
    digitalPorts32_47.isOverride = 0;
    digitalPorts48_63.isOverride = 0;
  }

  else if (cmdCombined == 257) // cmd is enable
  { 
    if (configPage1.allowHWTestMode == true)
    {
      BIT_SET(currentStatus.testIO_hardware, BIT_TESTHW_ACTIVE);  //set testactive flag
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
    if(BIT_CHECK(currentStatus.testIO_hardware, BIT_TESTHW_ACTIVE))
    {
      setDigitalPort((cmdCombined-513), HIGH, OUTPUT_OVERRIDE);
      //BIT_SET(currentStatus.digOut, (cmdCombined-513));
    }
  }
  
  // Turn Port to logical LOW
  else if ((cmdCombined >=769) && (cmdCombined <= 832)) //64 digital ports
  {
    // Turn a port to LOW state and set override flag
    if(BIT_CHECK(currentStatus.testIO_hardware, BIT_TESTHW_ACTIVE))
      {
        setDigitalPort((cmdCombined-769), LOW, OUTPUT_OVERRIDE);
        //BIT_CLEAR(currentStatus.digOut, (cmdCombined-769));
      }
  }
  
  //Can add a switch case under here for more specific commands

}  
