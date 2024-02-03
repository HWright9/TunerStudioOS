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

uint8_t canRx_MotecPLM_O2_tmr = 0;

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
    if (bitRead(Out_TS.Vars.canstatus, BIT_CANSTATUS_CAN0ACTIVATED) == false) { INIT_can0(); }    //init can interface 0
    
    else if ((bitRead(Out_TS.Vars.canstatus, BIT_CANSTATUS_CAN0FAILED) == true)) // CAN bus failed to send many messages, Attempt re-init.
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
       BIT_SET(Out_TS.Vars.canstatus, BIT_CANSTATUS_CAN0ACTIVATED);
       can0_Msg_FailCntr = 0;
       BIT_CLEAR(Out_TS.Vars.canstatus, BIT_CANSTATUS_CAN0FAILED);
        //TS_SERIALLink.println("CAN BUS Shield init ok!");
    }
    else
    {
      BIT_CLEAR(Out_TS.Vars.canstatus, BIT_CANSTATUS_CAN0ACTIVATED);
      //TS_SERIALLink.println("CAN BUS Shield init fail");
      //TS_SERIALLink.println("Init CAN BUS Shield again");
    }
  }
  else
  { // User disabled CAN
    BIT_CLEAR(Out_TS.Vars.canstatus, BIT_CANSTATUS_CAN0ACTIVATED);
    BIT_CLEAR(Out_TS.Vars.canstatus, BIT_CANSTATUS_CAN0FAILED);
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
   BIT_CLEAR(Out_TS.Vars.canstatus, BIT_CANSTATUS_CAN0MSGFAIL);
   BIT_CLEAR(Out_TS.Vars.canstatus, BIT_CANSTATUS_CAN0FAILED);
   can0_Msg_FailCntr = 0;
  } 
  else
  {
    //Serial.println("Error Sending Message...");
    BIT_SET(Out_TS.Vars.canstatus, BIT_CANSTATUS_CAN0MSGFAIL);
    if (can0_Msg_FailCntr < 255) { can0_Msg_FailCntr++; }
  }  

  if (can0_Msg_FailCntr > 50)
  {
    BIT_SET(Out_TS.Vars.canstatus, BIT_CANSTATUS_CAN0FAILED); // This stops any further tries at sending messages until re-init
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
       if (rxId == configPage1.canRXmsg_MotecPLM)
       {
         canRx_MotecPLM_O2(len, rxBuf);
       }
      }
}

//Handles timeouts for CAN messages not recieved, Called every 100ms.
void recieveCAN_Timeouts(void)
{
  if (configPage1.canRXmsg_MotecPLM > 0x00) // Enabled Message
  {
    if(canRx_MotecPLM_O2_tmr < 255) { canRx_MotecPLM_O2_tmr++; }
    if(canRx_MotecPLM_O2_tmr > 10) { canRx_MotecPLM_O2_Dflt(); }
  }
  else
  {
    canRx_MotecPLM_O2_tmr = 0;
    BIT_CLEAR(Out_TS.Vars.canRXmsg_dflt, CANRX_MOTECPLM_DFLT); // reset default flag    
  }
  
  // Check for any faults to set flag
  if (Out_TS.Vars.canRXmsg_dflt > 0x00) { BIT_SET(Out_TS.Vars.canstatus, BIT_CANSTATUS_CAN0RXMSGERR); }
  else { BIT_CLEAR(Out_TS.Vars.canstatus, BIT_CANSTATUS_CAN0RXMSGERR); }
}
  

void canBroadcast_5ms(void)
{
  if ((configPage1.can0Enable == true) && 
      (bitRead(Out_TS.Vars.canstatus, BIT_CANSTATUS_CAN0ACTIVATED) == true) &&
      (bitRead(Out_TS.Vars.canstatus, BIT_CANSTATUS_CAN0FAILED) == false))
  {
    byte canmsg[] = { 0, 0, 0, 0, 0, 0, 0, 5 };
    Send_CAN0_message(0, 0x500, canmsg);
  }
}

void canBroadcast_20ms(void)
{
  if ((configPage1.can0Enable == true) && 
      (bitRead(Out_TS.Vars.canstatus, BIT_CANSTATUS_CAN0ACTIVATED) == true) &&
      (bitRead(Out_TS.Vars.canstatus, BIT_CANSTATUS_CAN0FAILED) == false))
  {
  byte canmsg[] = { 0, 0, 0, 0, 0, 0, 0, 20 };
  Send_CAN0_message(0, 0x501, canmsg);
  }
}

void canBroadcast_50ms(void)
{
  if ((configPage1.can0Enable == true) && 
      (bitRead(Out_TS.Vars.canstatus, BIT_CANSTATUS_CAN0ACTIVATED) == true) &&
      (bitRead(Out_TS.Vars.canstatus, BIT_CANSTATUS_CAN0FAILED) == false))
  {
  byte canmsg[] = { 0, 0, 0, 0, 0, 0, 0, 50 };
  Send_CAN0_message(0, 0x502, canmsg);
  }
}

void canBroadcast_100ms(void)
{
  if ((configPage1.can0Enable == true) && 
      (bitRead(Out_TS.Vars.canstatus, BIT_CANSTATUS_CAN0ACTIVATED) == true) &&
      (bitRead(Out_TS.Vars.canstatus, BIT_CANSTATUS_CAN0FAILED) == false))
  {
  byte canmsg[] = { 0, 0, 0, 0, 0, 0, 0, 100 };
  Send_CAN0_message(0, 0x503, canmsg);
  }
}

void canBroadcast_500ms(void)
{
  if ((configPage1.can0Enable == true) && 
      (bitRead(Out_TS.Vars.canstatus, BIT_CANSTATUS_CAN0ACTIVATED) == true) &&
      (bitRead(Out_TS.Vars.canstatus, BIT_CANSTATUS_CAN0FAILED) == false))
  {
  byte canmsg[] = { 0, 0, 0, 0, 0, 0, 5, 000 };
  Send_CAN0_message(0, 0x504, canmsg);
  }
}

void canBroadcast_1000ms(void)
{
  if ((configPage1.can0Enable == true) && 
      (bitRead(Out_TS.Vars.canstatus, BIT_CANSTATUS_CAN0ACTIVATED) == true) &&
      (bitRead(Out_TS.Vars.canstatus, BIT_CANSTATUS_CAN0FAILED) == false))
  {
  byte canmsg[] = { 0, 0, 0, 0, 0, 0, 7, 000 };
  Send_CAN0_message(0, 0x505, canmsg);
  }
}

/* CAN RX Messages Below here */

/* Recieve MotecPLM Can message frame on defined CAN ID */
void canRx_MotecPLM_O2 (uint8_t len, uint8_t rxBuf[])
{
  if ((len == 8)) // Check msg on correct address and data length is correct
  {
    canRx_MotecPLM_O2_tmr = 0; //reset timeout
    BIT_CLEAR(Out_TS.Vars.canRXmsg_dflt, CANRX_MOTECPLM_DFLT); // reset default flag
    
    //byte0 Compound ID, not used
    
    // Check O2 data is valid using sensor status
    if (rxBuf[7] == 0x00)
    {
      //byte1 and 2 Calibrated Sensor Output Value Hi:lo*1 = x.xxxLa
      uint32_t result = (rxBuf[1] << 8) | rxBuf[2]; //(highByte << 8) | lowByte - this is EQR from PLM
      
      Out_TS.Vars.Ve_Eqr_Sensor1 = ((float)result)/1000.0;
      
    }
    else
    {
      Out_TS.Vars.Ve_Eqr_Sensor1 = 0.0;
    }

      
    //byte3 Heater duty cycle Byte*1 = xxx%

    //byte4 Device Internal Temperature Byte*195/10-500 = xxx.xC

    //byte5 Zp (Pump cell impedance) Byte*1 = X ohm

    //byte6 Diagnostic Field 1

    //byte7 sensor state
    /*
    switch (canRxMsg->data[7])
    {
      case (0x00):
      EQRLH_State = e_EQRState_RUN;
      break;
      
      case (0x01):
      EQRLH_State = e_EQRState_CONTROL_WAIT;
      break;

      case (0x02):
      EQRLH_State = e_EQRState_PUMP_WAIT;
      break;

      case (0x03):
      EQRLH_State = e_EQRState_WARM_UP;
      break;

      case (0x04):
      EQRLH_State = e_EQRState_NO_HEATER;
      break;

      case (0x05):
      EQRLH_State = e_EQRState_STOP;
      break;

      case (0x06):
      EQRLH_State = e_EQRState_PUMP_OFF;
      break;

      default:
      EQRLH_State = e_EQRState_STOP;
      break;
    }
    */
  }
}

// Default action when message times out
void canRx_MotecPLM_O2_Dflt(void)
{
  BIT_SET(Out_TS.Vars.canRXmsg_dflt, CANRX_MOTECPLM_DFLT);
  Out_TS.Vars.Ve_Eqr_Sensor1 = 0.0;
}

/* End CAN RX Messages */
