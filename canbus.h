#ifndef CANBUS_H
#define CANBUS_H

#include <mcp_can.h>          //see canbus.h for all canbus config
#include <SPI.h> 

/*--------------- canbus config options -------------- */
/* CAN 0 */
#define CAN0_INT        2         // Set INT to pin 2
#if defined(ARDUINO_AVR_MEGA2560)
  #define CAN0_CS         53        // Set CS to pin 11 on Mega
#elif defined(ARDUINO_AVR_UNO)
  #define CAN0_CS         10        // Set CS to pin 10 on UNO
#else
  #error Incorrect board selected. Currently AVR Mega2560 and UNO supported. Please select the correct board and upload again
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


#endif
