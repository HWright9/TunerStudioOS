/*
* canbus.ino
* Concerned with commuications via Controller Aera Network (CAN) Bus module
*
* Copyright (C) Henry Wright
* A full copy of the license may be found in the projects root directory
*
* Based on code by Josh Stewart for the Speeduino project , and Darren Siepka for the GPIO module see www.Speeduino.com for more info a
*/

//#include "SDCard.h"

/*Variables Local to this function*/
uint32_t rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
uint8_t can0_Msg_FailCntr;
uint32_t CANLogMillis = 0;
uint32_t sdFileLogPrev = 0;

uint8_t Ve_e_SDFileSendStat;

struct s_sdLogBuff{
  uint32_t logMillis;
  uint32_t canID;
  unsigned char length;
  unsigned char canmsgData[8]; 
};
  
s_sdLogBuff sdLogBuff;

// CAN bus maintenance, call this at a slow rate to recover cleanly from disconnections and enable/disable CAN
void CAN0_maintenance(void)
{
  if (configPage1.can0Enable == true)
  {
    if (bitRead(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0ACTIVATED) == false) { INIT_can0(); }    //init can interface 0
    
    else if ((bitRead(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0FAILED) == true)) // CAN bus failed to send many messages, Attempt re-init.
    {
      INIT_can0();
      //byte canmsg[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
      //Send_CAN0_message(0, 0x799, canmsg);
    }
  }
}


void INIT_can0(void)
{
  if (configPage1.can0Enable == true)
  {
    byte CANStat = CAN0.begin(MCP_ANY, (uint32_t)configPage1.can0Baud, (uint8_t)configPage1.can0XTalFreq); // init can bus : baudrate = CAN_1000KBPS, frequency MCP_8MHZ
    
    if(CANStat == CAN_OK)  
    {
       CAN0.setMode(MCP_NORMAL);
       BIT_SET(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0ACTIVATED);
       can0_Msg_FailCntr = 0;
       BIT_CLEAR(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0FAILED);
        //TS_SERIALLink.println("CAN BUS Shield init ok!");
    }
    else
    {
      BIT_CLEAR(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0ACTIVATED);
      //TS_SERIALLink.println("CAN BUS Shield init fail");
      //TS_SERIALLink.println("Init CAN BUS Shield again");
    }
  }
  else
  { // User disabled CAN
    BIT_CLEAR(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0ACTIVATED);
    BIT_CLEAR(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0FAILED);
    can0_Msg_FailCntr = 0;
  }
  Ve_e_SDFileSendStat = SD_FILE_SEND_INIT;
}


//----------------------------------------------------------------------------------------
void Send_CAN0_message(uint16_t theaddress, byte len, byte *thedata)
{

  byte CANStat = CAN0.sendMsgBuf(theaddress, 0, len, thedata);
  //Out_TS.Vars.dev1 = CANStat;    
  if(CANStat == CAN_OK)
  {
    //Serial.println("Message Sent Successfully!");
   BIT_CLEAR(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0MSGFAIL);
   BIT_CLEAR(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0FAILED);
   can0_Msg_FailCntr = 0;
   if(configPage1.LEDSEnbl == true)
   {
     BIT_TOGGLE(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_LEDRED);
     uint8_t Le_b_ledStat = !bitRead(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_LEDRED);
     setDigitalPort(Pin_LEDRED, Le_b_ledStat , OUTPUT_NORMAL); // Set LED to variable status.
   }
  } 
  else
  {
    //Serial.println("Error Sending Message...");
    BIT_SET(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0MSGFAIL);
    if (can0_Msg_FailCntr < 255) { can0_Msg_FailCntr++; }
  }  

  if (can0_Msg_FailCntr > 50)
  {
    BIT_SET(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0FAILED); // This stops any further tries at sending messages until re-init
  }
}

//---------------------------------------------------------------------------------------------

void receive_CAN0_message()
{
  uint8_t canErr = CAN_OK;
  
  while((configPage3.Ke_b_canRxEnbl == true) && (CAN0.checkReceive() == CAN_MSGAVAIL)) // Not using int2
  { 
    canErr = CAN0.readMsgBuf(&rxId, &len, rxBuf);      // Read data: len = data length, buf = data byte(s)

    if ((canErr == CAN_OK) && ((rxId & 0x80000000) != 0x80000000))  // alternate would be CAN_NOMSG, also not extended frame, id is std 11 bit, Not 29bit
    {
      if(Ve_b_DebugON == true) 
      { 
        Serial.print (rxId,HEX); Serial.print(" ");
        Serial.print (configPage3.Ke_h_CANIDMin,HEX); Serial.print(" ");
        Serial.print (configPage3.Ke_h_CANIDMax,HEX); Serial.println(" ");
      }
      if ((rxId >= configPage3.Ke_h_CANIDMin) && (rxId <= configPage3.Ke_h_CANIDMax))
      {
        BIT_CLEAR(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0RXFILTER);
        // record the counts of recieved msg IDs      
        uint8_t Le_IDUpdate = false;
        for(uint8_t idx = 0; idx < NUM_OF_CAN_RX_IDS; idx++)
        {
          if(rxId == Out_TS.Vars.Va_h_CANRxIDs[idx]) //RXid is in the array
          {
            if (Out_TS.Vars.Va_cnt_CANRXIDsCnt[idx] < UINT_MAX) { Out_TS.Vars.Va_cnt_CANRXIDsCnt[idx]++; }
            Le_IDUpdate = true;
            idx = NUM_OF_CAN_RX_IDS; // exit the for loop
          }
          else if (Out_TS.Vars.Va_h_CANRxIDs[idx] == 0)
          {
            Out_TS.Vars.Va_h_CANRxIDs[idx] = rxId; //RXid not in the array and there is a blank spot so add it.
            Le_IDUpdate = true;
            idx = NUM_OF_CAN_RX_IDS; // exit the for loop
          }
          else
          {
            // do nothing
          }
        }
        
        if (Le_IDUpdate == false)
        {
          Out_TS.Vars.Ve_b_CANRXArrayOverflow = true;
        }
        
        if ((configPage3.Ke_e_SDLogMode == SDLOGMODE_LOGASCII) &&
            ((configPage3.Ke_e_SDLogAuto == true) || 
            (bitRead(Out_TS.Vars.Va_b_SDLoggingStatus, BIT_SDLOG_MANACTIVE))))
        {
          SDCARD_Write_ASCII_CAN();          
        }
        
        if ((configPage3.Ke_e_SDLogMode == SDLOGMODE_LOGDATA) &&
            ((configPage3.Ke_e_SDLogAuto == true) || 
            (bitRead(Out_TS.Vars.Va_b_SDLoggingStatus, BIT_SDLOG_MANACTIVE))))
        {
          SDCARD_Write_Data_CAN();
        }
        
        if(configPage1.LEDSEnbl == true)
        {
          BIT_TOGGLE(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_LEDGREEN);
          uint8_t Le_b_ledStat = !bitRead(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_LEDGREEN);
          setDigitalPort(Pin_LEDGREEN, Le_b_ledStat, OUTPUT_NORMAL); // Set LED to variable status.
        }
      }
      else
      {
        // Msg recieved but was filtered.
        BIT_SET(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0RXFILTER);
      }

    }
  }
}

//Handles timeouts for CAN messages not recieved, Called every 100ms.
void recieveCAN_Timeouts(void)
{
   
  // Check for any faults to set flag
  if (Out_TS.Vars.canRXmsg_dflt > 0x00) { BIT_SET(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0RXMSGERR); }
  else { BIT_CLEAR(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0RXMSGERR); }
}
  
  

void SDCARD_Write_ASCII_CAN(void)
{
  if(Ve_e_SDFileStatus == SDFILE_CLOSED)
  {
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    SDCRD_F_DataFile = SD.open("/logging/LOGASCII.txt", FILE_WRITE);
    if(SDCRD_F_DataFile != 0) 
    { 
      Ve_e_SDFileStatus = SDFILE_OPEN;
      SDCRD_F_DataFile.println("LogStart");      
    }
    else 
    { 
      Ve_e_SDFileStatus = SDFILE_ERR; 
    }
  }
  
  if(Ve_e_SDFileStatus == SDFILE_OPEN)
  {
    BIT_SET(Out_TS.Vars.Va_b_SDLoggingStatus, BIT_SDLOG_LOGGING); //set logging status
    BIT_SET(Out_TS.Vars.Va_b_SDLoggingStatus, BIT_SDLOG_FILEOPEN); //set logging status
    // make a string for assembling the data to log to SD:
    char delimiter = ',';
    String dataString = "";
    CANLogMillis = millis();
    dataString += String(CANLogMillis); dataString += delimiter;
    dataString += String(rxId, HEX); dataString += delimiter;
    dataString += String(len, HEX); dataString += delimiter;
    for (int i = 0; i<len; i++)  
    {
      dataString += String(rxBuf[i], HEX); dataString += delimiter;
    }
    SDCRD_F_DataFile.println(dataString); //write string to sd card
    //Serial.println(dataString);
  }
    
}


void SDCARD_Write_Data_CAN(void)
{
  if(Ve_e_SDFileStatus == SDFILE_CLOSED)
  {
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    //data files are not appended, just logged for replay, so we delete the old file.
    if(SD.exists("/logging/LOGDAT.DAT") == true)
    {
      SD.remove("/logging/LOGDAT.DAT");
    }
        
    SDCRD_F_DataFile = SD.open("/logging/LOGDAT.DAT", FILE_WRITE);
    if(SDCRD_F_DataFile != 0) 
    { 
      Ve_e_SDFileStatus = SDFILE_OPEN;     
    }
    else 
    { 
      Ve_e_SDFileStatus = SDFILE_ERR; 
    }
  }
  
  if(Ve_e_SDFileStatus == SDFILE_OPEN)
  {
    BIT_SET(Out_TS.Vars.Va_b_SDLoggingStatus, BIT_SDLOG_LOGGING); //set logging status
    BIT_SET(Out_TS.Vars.Va_b_SDLoggingStatus, BIT_SDLOG_FILEOPEN); //set logging status
    //write data directly.
    CANLogMillis = millis();

    sdLogBuff.logMillis = CANLogMillis;
    sdLogBuff.canID = rxId;
    sdLogBuff.length = len;
    for (int i = 0; i<len; i++)  
    {
      sdLogBuff.canmsgData[i] = rxBuf[i];
    }

    SDCRD_F_DataFile.write((byte*)&sdLogBuff, sizeof(sdLogBuff));
    // SDCRD_F_DataFile.write((byte*)&CANLogMillis, sizeof(CANLogMillis));
    // SDCRD_F_DataFile.write((byte*)&rxId, sizeof(rxId));
    // SDCRD_F_DataFile.write(len);
    // SDCRD_F_DataFile.write(rxBuf,len);
  }
    
}

void SDCARD_Maint(void)
{
  if ((CANLogMillis > 0) && (millis() > (CANLogMillis + 1000)))
  {
    if (Ve_e_SDFileStatus == SDFILE_OPEN) 
    {
      BIT_CLEAR(Out_TS.Vars.Va_b_SDLoggingStatus, BIT_SDLOG_LOGGING); //set logging status
      BIT_CLEAR(Out_TS.Vars.Va_b_SDLoggingStatus, BIT_SDLOG_FILEOPEN); //set logging status
      SDCRD_F_DataFile.println("LogEnd");
      SDCRD_F_DataFile.close();
      Ve_e_SDFileStatus = SDFILE_CLOSED;
      Ve_e_SDFileSendStat = SD_FILE_SEND_END;
      //Serial.println("SDFILE_CLOSED");
    }
  }
}

void canSendSDRecordedData(void)
{
  uint8_t FileStat = -1;
  
  switch (Ve_e_SDFileSendStat)
  {
    case SD_FILE_SEND_INIT:
      
      if(SD.exists("/logging/LOGDAT.DAT") == true) // check for file
      {
        if(Ve_e_SDFileStatus == SDFILE_OPEN) // if already open. Close it
        {
          SDCRD_F_DataFile.close();
        }
        
        SDCRD_F_DataFile = SD.open("/logging/LOGDAT.DAT", FILE_READ); // open file
        if(SDCRD_F_DataFile != 0) 
        { 
          Ve_e_SDFileStatus = SDFILE_OPEN;
          BIT_SET(Out_TS.Vars.Va_b_SDLoggingStatus, BIT_SDLOG_FILEOPEN); //set logging status
          
          // inital read of SD card.
          if (SDCRD_F_DataFile.available() >= sizeof(sdLogBuff))
          {
            FileStat = SDCRD_F_DataFile.read(&sdLogBuff, sizeof(sdLogBuff)); // read a row of data
            
            if(FileStat == -1)
            {
              Ve_e_SDFileSendStat = SD_FILE_SEND_END;
            }
            
            sdFileLogPrev = sdLogBuff.logMillis; // record timestamp for next time.
            CANLogMillis = millis(); // record system time stamp for next time.
            Send_CAN0_message(sdLogBuff.canID, sdLogBuff.length, sdLogBuff.canmsgData);
            Ve_e_SDFileSendStat = SD_FILE_SEND_SENT;
          }
          else
          {
            Ve_e_SDFileSendStat = SD_FILE_SEND_END;
          }
        }
        else  // error opening file.
        { 
          Ve_e_SDFileStatus = SDFILE_ERR;
          Ve_e_SDFileSendStat = SD_FILE_SEND_END;
        }
      }
      else // file does not exist
      {
        Ve_e_SDFileStatus = SDFILE_ERR; 
        Ve_e_SDFileSendStat = SD_FILE_SEND_END;
      }
    break;
    
    case SD_FILE_SEND_PENDING:
      if ((millis() - CANLogMillis) >= (sdLogBuff.logMillis - sdFileLogPrev))
      {
        if((sdLogBuff.canID >= configPage3.Ke_h_CANIDMin) && (sdLogBuff.canID <= configPage3.Ke_h_CANIDMax))
        {
          Send_CAN0_message(sdLogBuff.canID, sdLogBuff.length, sdLogBuff.canmsgData);
          BIT_SET(Out_TS.Vars.Va_b_SDLoggingStatus, BIT_SDLOG_SENTDATA); //set sent data flag
        }
        sdFileLogPrev = sdLogBuff.logMillis; // record timestamp for next time.
        CANLogMillis = millis(); // record system time stamp for next time.
        Ve_e_SDFileSendStat = SD_FILE_SEND_SENT;
      }
    break;
    
    case SD_FILE_SEND_SENT:
      if (SDCRD_F_DataFile.available() >= sizeof(sdLogBuff))
      {
        FileStat = SDCRD_F_DataFile.read(&sdLogBuff, sizeof(sdLogBuff)); // read a row of data
        Ve_e_SDFileSendStat = SD_FILE_SEND_PENDING;
        
        if(FileStat == -1)
        {
          Ve_e_SDFileSendStat = SD_FILE_SEND_END;
        }
      }
      else
      {
        Ve_e_SDFileSendStat = SD_FILE_SEND_END;
      }
    break;
    
    case SD_FILE_SEND_END:
      
      BIT_CLEAR(Out_TS.Vars.Va_b_SDLoggingStatus, BIT_SDLOG_SENTDATA); //clear sent data flag
      
      if (Ve_e_SDFileStatus == SDFILE_OPEN)
      {
        SDCRD_F_DataFile.close();
        BIT_CLEAR(Out_TS.Vars.Va_b_SDLoggingStatus, BIT_SDLOG_FILEOPEN); //set logging status
      }
      
      if (bitRead(Out_TS.Vars.Va_b_SDLoggingStatus, BIT_SDLOG_REPLAYRESET))
      {
        BIT_CLEAR(Out_TS.Vars.Va_b_SDLoggingStatus, BIT_SDLOG_REPLAYRESET);
        Ve_e_SDFileSendStat = SD_FILE_SEND_INIT;
      }
    break;
      
  }
  
}


void canBroadcast_5ms(void)
{
  if ((configPage1.can0Enable == true) && 
      (bitRead(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0ACTIVATED) == true) &&
      (bitRead(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0FAILED) == false))
  {
    byte canmsg[] = { 0, 0, 0, 0, 0, 0, 0, 5 };
    Send_CAN0_message(0x500, 8, canmsg);
  }
}


void canBroadcast_20ms(void)
{
  if ((configPage1.can0Enable == true) && 
      (bitRead(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0ACTIVATED) == true) &&
      (bitRead(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0FAILED) == false))
  {
  byte canmsg[] = { 0, 0, 0, 0, 0, 0, 0, 20 };
  Send_CAN0_message(0x501, 8, canmsg);
  }
}

void canBroadcast_50ms(void)
{
  if ((configPage1.can0Enable == true) && 
      (bitRead(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0ACTIVATED) == true) &&
      (bitRead(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0FAILED) == false))
  {
  byte canmsg[] = { 0, 0, 0, 0, 0, 0, 0, 50 };
  Send_CAN0_message(0x502, 8, canmsg);
  }
}

void canBroadcast_100ms(void)
{
  if ((configPage1.can0Enable == true) && 
      (bitRead(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0ACTIVATED) == true) &&
      (bitRead(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0FAILED) == false))
  {
  byte canmsg[] = { 0x08, 0x04, 0xFF, 0xFF, 0xFF, 0xFF, 0x35, 100 };
  canmsg[0] = highByte(Out_TS.Vars.dev4);
  canmsg[1] = lowByte(Out_TS.Vars.dev4);
  Send_CAN0_message(0x402, 8, canmsg);  // engine rpm
  }
}

void canBroadcast_500ms(void)
{
  if ((configPage1.can0Enable == true) && 
      (bitRead(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0ACTIVATED) == true) &&
      (bitRead(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0FAILED) == false))
  {
  byte canmsg[] = { 0, 0, 0, 0, 0, 0, 5, 5 };
  Send_CAN0_message(0x504, 8, canmsg);
  }
}

void canBroadcast_1000ms(void)
{
  if ((configPage1.can0Enable == true) && 
      (bitRead(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0ACTIVATED) == true) &&
      (bitRead(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0FAILED) == false))
  {
  byte canmsg[] = { 0, 0, 0, 0, 0, 0, 7, 10 };

  Send_CAN0_message(0x505, 8, canmsg);
  }
}

/* CAN RX Messages Below here */


/* End CAN RX Messages */
