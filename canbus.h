#ifndef CANBUS_H
#define CANBUS_H

#include <SPI.h>
#include <mcp_can.h>          //see canbus.h for all canbus config

/*--------------- canbus config options -------------- */
/* CAN 0 */
#define CAN0_INT        2         // Set INT to pin 2
#if defined(ARDUINO_AVR_MEGA2560)
  #define CAN0_CS         53        // Set CS to pin 53 on Mega
#else
  #define CAN0_CS         10        // Set CS to pin 10 on UNO
#endif 

#define CAN_XTAL_8MHZ           0  // Different MCP2515 boards have different crystals.
#define CAN_XTAL_16MHZ          1
/*-----------------------------------------------------*/

/*Variables Shared between Functions should have [XXXX_] naming convention where XXXX is the source module identifier*/


/*Functions */
void CAN0_INT_routine(void);
void INIT_can0(void);
void CAN0_maintenance(void);
void Send_CAN0_message(byte bcChan, uint16_t theaddress, byte *thedata);
void receive_CAN0_message(void);
void obd_command(byte usecan);
uint16_t obd_response(byte therequestedPID, uint16_t therequestedCANID);


void canBroadcast_5ms(void);
void canBroadcast_20ms(void);
void canBroadcast_50ms(void);
void canBroadcast_100ms(void);
void canBroadcast_500ms(void);
void canBroadcast_1000ms(void);


void recieveCAN_Timeouts(void);
void canRx_MotecPLM_O2 (uint8_t len, uint8_t rxBuf);
void canRx_MotecPLM_O2_Dflt(void);


#endif
