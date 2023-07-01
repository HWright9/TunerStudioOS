/*
* canbus.ino
* Concerned with commuications via Controller Aera Network (CAN) Bus module
*
* Copyright (C) Henry Wright
* A full copy of the license may be found in the projects root directory
*
* Based on code by Josh Stewart for the Speeduino project , and Darren Siepka for the GPIO module see www.Speeduino.com for more info a
*/

/*Variables Local to this function*/
long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
uint8_t can0_Msg_FailCntr;
uint8_t senddata[8] = {0x00, 0x00, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};

void CAN0_INT_routine(void)
{ 
  #if OBD_CANPORT == 0
        #if OBD_ACTIVE == 1
            //obd_command(0);
        #else
            receive_CAN0_message();
        #endif
  #else
    receive_CAN0_message();     
  #endif    
}

// CAN bus maintenance, call this at a slow rate to recover cleanly from disconnections and enable/disable CAN
void CAN0_maintenance(void)
{
  if (configPage1.can0Enable == true)
  {
    if (bitRead(currentStatus.canstatus, BIT_CANSTATUS_CAN0ACTIVATED) == false) { INIT_can0(); }    //init can interface 0
    
    else if ((bitRead(currentStatus.canstatus, BIT_CANSTATUS_CAN0FAILED) == true)) // CAN bus failed to send many messages, Attempt re-init.
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
       BIT_SET(currentStatus.canstatus, BIT_CANSTATUS_CAN0ACTIVATED);
       can0_Msg_FailCntr = 0;
       BIT_CLEAR(currentStatus.canstatus, BIT_CANSTATUS_CAN0FAILED);
        //TS_SERIALLink.println("CAN BUS Shield init ok!");
    }
    else
    {
      BIT_CLEAR(currentStatus.canstatus, BIT_CANSTATUS_CAN0ACTIVATED);
      //TS_SERIALLink.println("CAN BUS Shield init fail");
      //TS_SERIALLink.println("Init CAN BUS Shield again");
    }
  }
  else
  { // User disabled CAN
    BIT_CLEAR(currentStatus.canstatus, BIT_CANSTATUS_CAN0ACTIVATED);
    BIT_CLEAR(currentStatus.canstatus, BIT_CANSTATUS_CAN0FAILED);
    can0_Msg_FailCntr = 0;
  }
  
}


//----------------------------------------------------------------------------------------
void Send_CAN0_message(byte bcChan, uint16_t theaddress, byte *thedata)
{

  byte CANStat = CAN0.sendMsgBuf(theaddress, 0, 8, thedata);
  currentStatus.dev1 = CANStat;    
  if(CANStat == CAN_OK)
  {
    //Serial.println("Message Sent Successfully!");
   BIT_CLEAR(currentStatus.canstatus, BIT_CANSTATUS_CAN0MSGFAIL);
   BIT_CLEAR(currentStatus.canstatus, BIT_CANSTATUS_CAN0FAILED);
   can0_Msg_FailCntr = 0;
  } 
  else
  {
    //Serial.println("Error Sending Message...");
    BIT_SET(currentStatus.canstatus, BIT_CANSTATUS_CAN0MSGFAIL);
    if (can0_Msg_FailCntr < 255) { can0_Msg_FailCntr++; }
  }  

  if (can0_Msg_FailCntr > 50)
  {
    BIT_SET(currentStatus.canstatus, BIT_CANSTATUS_CAN0FAILED); // This stops any further tries at sending messages until re-init
  }
}

//---------------------------------------------------------------------------------------------

void receive_CAN0_message()
  {
    CAN0.readMsgBuf(&rxId, &len, rxBuf);      // Read data: len = data length, buf = data byte(s)
         
    if((rxId & 0x80000000) == 0x80000000)     // Determine if ID is standard (11 bits) or extended (29 bits)
      {
       // id is extended 29bit address
       //sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (rxId & 0x1FFFFFFF), len);
      }
    else  
      {
       // id is std 11 bit
      }
 

}

void canBroadcast_5ms(void)
{
  if ((configPage1.can0Enable == true) && 
      (bitRead(currentStatus.canstatus, BIT_CANSTATUS_CAN0ACTIVATED) == true) &&
      (bitRead(currentStatus.canstatus, BIT_CANSTATUS_CAN0FAILED) == false))
  {
    byte canmsg[] = { 0, 0, 0, 0, 0, 0, 0, 5 };
    Send_CAN0_message(0, 0x500, canmsg);
  }
}

void canBroadcast_20ms(void)
{
  if ((configPage1.can0Enable == true) && 
      (bitRead(currentStatus.canstatus, BIT_CANSTATUS_CAN0ACTIVATED) == true) &&
      (bitRead(currentStatus.canstatus, BIT_CANSTATUS_CAN0FAILED) == false))
  {
  byte canmsg[] = { 0, 0, 0, 0, 0, 0, 0, 20 };
  Send_CAN0_message(0, 0x501, canmsg);
  }
}

void canBroadcast_50ms(void)
{
  if ((configPage1.can0Enable == true) && 
      (bitRead(currentStatus.canstatus, BIT_CANSTATUS_CAN0ACTIVATED) == true) &&
      (bitRead(currentStatus.canstatus, BIT_CANSTATUS_CAN0FAILED) == false))
  {
  byte canmsg[] = { 0, 0, 0, 0, 0, 0, 0, 50 };
  Send_CAN0_message(0, 0x502, canmsg);
  }
}

void canBroadcast_100ms(void)
{
  if ((configPage1.can0Enable == true) && 
      (bitRead(currentStatus.canstatus, BIT_CANSTATUS_CAN0ACTIVATED) == true) &&
      (bitRead(currentStatus.canstatus, BIT_CANSTATUS_CAN0FAILED) == false))
  {
  byte canmsg[] = { 0, 0, 0, 0, 0, 0, 0, 100 };
  Send_CAN0_message(0, 0x503, canmsg);
  }
}

void canBroadcast_500ms(void)
{
  if ((configPage1.can0Enable == true) && 
      (bitRead(currentStatus.canstatus, BIT_CANSTATUS_CAN0ACTIVATED) == true) &&
      (bitRead(currentStatus.canstatus, BIT_CANSTATUS_CAN0FAILED) == false))
  {
  byte canmsg[] = { 0, 0, 0, 0, 0, 0, 5, 000 };
  Send_CAN0_message(0, 0x504, canmsg);
  }
}

void canBroadcast_1000ms(void)
{
  if ((configPage1.can0Enable == true) && 
      (bitRead(currentStatus.canstatus, BIT_CANSTATUS_CAN0ACTIVATED) == true) &&
      (bitRead(currentStatus.canstatus, BIT_CANSTATUS_CAN0FAILED) == false))
  {
  byte canmsg[] = { 0, 0, 0, 0, 0, 0, 7, 000 };
  Send_CAN0_message(0, 0x505, canmsg);
  }
}
