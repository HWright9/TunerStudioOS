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
uint32_t CANRxMillis = 0;


// CAN bus maintenance, call this at a slow rate to recover cleanly from disconnections and enable/disable CAN
void CAN0_maintenance(void)
{
  if (configPage1.can0Enable == true)
  {
    if (bitRead(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0ACTIVATED) == false) { INIT_can0(); }    //init can interface 0
    
    else if ((bitRead(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0FAILED) == true)) // CAN bus failed to send many messages, Attempt re-init.
    {
      byte canmsg[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
      Send_CAN0_message(0, 0x799, canmsg);
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
  
}


//----------------------------------------------------------------------------------------
void Send_CAN0_message(byte bcChan, uint16_t theaddress, byte *thedata)
{

  byte CANStat = CAN0.sendMsgBuf(theaddress, 0, 8, thedata);
  //Out_TS.Vars.dev1 = CANStat;    
  if(CANStat == CAN_OK)
  {
    //Serial.println("Message Sent Successfully!");
   BIT_CLEAR(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0MSGFAIL);
   BIT_CLEAR(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0FAILED);
   can0_Msg_FailCntr = 0;
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
      
      if (((configPage3.Ke_e_SDLogAuto == true) || 
           (BIT_CHECK(Out_TS.Vars.Va_b_SDLoggingStatus, BIT_SDLOG_MANACTIVE))) && 
          (configPage3.Ke_e_SDLogMode == SDLOGMODE_LOGASCII))
      {
        SDCARD_Write_ASCII_CAN();
      }
      
      if (((configPage3.Ke_e_SDLogAuto == true) || 
           (BIT_CHECK(Out_TS.Vars.Va_b_SDLoggingStatus, BIT_SDLOG_MANACTIVE))) && 
          (configPage3.Ke_e_SDLogMode == SDLOGMODE_LOGDATA))
      {
        SDCARD_Write_Data_CAN();
      }
       
      // if (0) // serial port print 
      // {
        // Serial.print(rxId, HEX); // print ID
        // Serial.print(" "); 
        // Serial.print(len, HEX); // print DLC
        // Serial.print(" ");
        
        // for (int i = 0; i<len; i++)  
        // {  // print the data
          // Serial.print(rxBuf[i],HEX);
          // Serial.print(" ");
        // }
      // }        
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
    CANRxMillis = millis();
    dataString += String(CANRxMillis); dataString += delimiter;
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
    if(SD.exists("/logging/LOGDat.txt") == true)
    {
      SD.remove("/logging/LOGDat.txt");
    }
        
    SDCRD_F_DataFile = SD.open("/logging/LOGDat.txt", FILE_WRITE);
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
    CANRxMillis = millis();
    SDCRD_F_DataFile.write(CANRxMillis);
    SDCRD_F_DataFile.write(rxId);
    SDCRD_F_DataFile.write(len);
    SDCRD_F_DataFile.write(rxBuf,len);
  }
    
}

void SDCARD_Maint(void)
{
  if ((CANRxMillis > 0) && (millis() > (CANRxMillis + 1000)))
  {
    if (Ve_e_SDFileStatus == SDFILE_OPEN) 
    {
      BIT_CLEAR(Out_TS.Vars.Va_b_SDLoggingStatus, BIT_SDLOG_LOGGING); //set logging status
      BIT_CLEAR(Out_TS.Vars.Va_b_SDLoggingStatus, BIT_SDLOG_FILEOPEN); //set logging status
      SDCRD_F_DataFile.println("LogEnd");
      SDCRD_F_DataFile.close();
      Ve_e_SDFileStatus = SDFILE_CLOSED;
      //Serial.println("SDFILE_CLOSED");
    }
  }
}

// if (configPage3.Ke_e_SDLogMode == SDLOGMODE_REPLAY)
void canSendSDRecordedData(void)
{
 
  if(Ve_e_SDFileStatus == SDFILE_CLOSED)
  {
    if(SD.exists("/logging/LOGDat.txt") == true) // check for file
    {
      SDCRD_F_DataFile = SD.open("/logging/LOGDat.txt", FILE_WRITE);
      if(SDCRD_F_DataFile != 0) 
      { 
        Ve_e_SDFileStatus = SDFILE_OPEN;     
      }
      else 
      { 
        Ve_e_SDFileStatus = SDFILE_ERR; 
      }
    }
    else
    {
      Ve_e_SDFileStatus = SDFILE_ERR; // file does not exist
    }
  }
  
  if(Ve_e_SDFileStatus == SDFILE_OPEN)
  {
    // Read file and send via CAN, save variable with next ms to send can data.
  }
}


void canBroadcast_5ms(void)
{
  if ((configPage1.can0Enable == true) && 
      (bitRead(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0ACTIVATED) == true) &&
      (bitRead(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0FAILED) == false))
  {
    byte canmsg[] = { 0, 0, 0, 0, 0, 0, 0, 5 };
    Send_CAN0_message(0, 0x500, canmsg);
  }
}


void canBroadcast_20ms(void)
{
  if ((configPage1.can0Enable == true) && 
      (bitRead(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0ACTIVATED) == true) &&
      (bitRead(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0FAILED) == false))
  {
  byte canmsg[] = { 0, 0, 0, 0, 0, 0, 0, 20 };
  Send_CAN0_message(0, 0x501, canmsg);
  }
}

void canBroadcast_50ms(void)
{
  if ((configPage1.can0Enable == true) && 
      (bitRead(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0ACTIVATED) == true) &&
      (bitRead(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0FAILED) == false))
  {
  byte canmsg[] = { 0, 0, 0, 0, 0, 0, 0, 50 };
  Send_CAN0_message(0, 0x502, canmsg);
  }
}

void canBroadcast_100ms(void)
{
  if ((configPage1.can0Enable == true) && 
      (bitRead(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0ACTIVATED) == true) &&
      (bitRead(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0FAILED) == false))
  {
  byte canmsg[] = { 0, 0, 0, 0, 0, 0, 0, 100 };
  Send_CAN0_message(0, 0x503, canmsg);
  }
}

void canBroadcast_500ms(void)
{
  if ((configPage1.can0Enable == true) && 
      (bitRead(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0ACTIVATED) == true) &&
      (bitRead(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0FAILED) == false))
  {
  byte canmsg[] = { 0, 0, 0, 0, 0, 0, 5, 000 };
  Send_CAN0_message(0, 0x504, canmsg);
  }
}

void canBroadcast_1000ms(void)
{
  if ((configPage1.can0Enable == true) && 
      (bitRead(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0ACTIVATED) == true) &&
      (bitRead(Out_TS.Vars.Va_b_canstatus, BIT_CANSTATUS_CAN0FAILED) == false))
  {
  byte canmsg[] = { 0, 0, 0, 0, 0, 0, 7, 000 };
  Send_CAN0_message(0, 0x505, canmsg);
  }
}

/* CAN RX Messages Below here */


/* End CAN RX Messages */
