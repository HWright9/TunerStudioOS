/*
* globals.h
* Global headers, defines and data structures, shared with the entire program.
*
* Copyright (C) Henry Wright
* A full copy of the license may be found in the projects root directory
*
* Based on code by Josh Stewart for the Speeduino project , and Darren Siepka for the GPIO module see www.Speeduino.com for more info a
*/
#ifndef GLOBALS_H
#define GLOBALS_H

#if defined(ARDUINO_ARCH_AVR)
  #define CORE_AVR
  #define AVR_WDT
  
#elif defined(CORE_TEENSY)
  #define EEPROM_SIZE_8KB
  
#elif defined(STM32_MCU_SERIES) || defined(_VARIANT_ARDUINO_STM32_)
  #define CORE_STM32
  #define EEPROM_SIZE_8KB


  extern "C" char* sbrk(int incr); //Used by UTIL_freeRam
  inline unsigned char  digitalPinToInterrupt(unsigned char Interrupt_pin) { return Interrupt_pin; } //This isn't included in the stm32duino libs (yet)
  #define portOutputRegister(port) (volatile byte *)( &(port->regs->ODR) ) //These are defined in STM32F1/variants/generic_stm32f103c/variant.h but return a non byte* value
  #define portInputRegister(port) (volatile byte *)( &(port->regs->IDR) ) //These are defined in STM32F1/variants/generic_stm32f103c/variant.h but return a non byte* value

#else
  #error Incorrect board selected. Currently AVR Mega2560 and UNO supported. Please select the correct board and upload again
#endif  

// now set specific processor compile flags
#if defined(__AVR_ATmega1280__) || defined(ARDUINO_AVR_MEGA2560) || defined(__AVR_ATmega2561__)
  #define EEPROM_SIZE_8KB
  #define AUX_SERIAL_ENBL
  #define MEGA_AVR
  #define BOARD_MAX_IO_PINS  58 //digital pins + analog channels + 1
  #define BOARD_MAX_DIGITAL_PINS 52
  #define BOARD_MAX_ADC_PINS 15
  
#elif defined(ARDUINO_AVR_NANO)
  #define BOARD_MAX_IO_PINS 20 //digital pins + analog channels + 1
  #define BOARD_MAX_DIGITAL_PINS 13
  #define BOARD_MAX_ADC_PINS 6
  
#elif defined(ARDUINO_AVR_UNO)
  #define BOARD_MAX_IO_PINS  20 //digital pins + analog channels + 1
  #define BOARD_MAX_DIGITAL_PINS 13
  #define BOARD_MAX_ADC_PINS 6

#else
  #error Incorrect board selected. Currently AVR Mega2560, NANO and UNO supported. Please select the correct board and upload again
#endif 

// global true/false statements
#define LOW                                0
#define HIGH                               1

#define false                              0
#define true                               1

#define OUTPUT_NORMAL                       0
#define OUTPUT_OVERRIDE                     1

#define DPIN_DISABLED                        0 // for configuring board pins
#define APIN_ENABLED                         1 // for configuring board pins

//define masks for system status'
#define BIT_SYSTEM_READY                    0
#define BIT_SYSTEM_WARNING                  1
#define BIT_SYSTEM_2                        2
#define BIT_SYSTEM_BURN_GOOD                7

//Define masks for CanStatus
#define BIT_CANSTATUS_CAN0ACTIVATED         0  //can0 has enabled
#define BIT_CANSTATUS_CAN0FAILED            1  //can0 soft failure (will retry)
#define BIT_CANSTATUS_CAN1ACTIVATED         2  //can1 has enabled
#define BIT_CANSTATUS_CAN1FAILED            3  //can1 failed o configure
#define BIT_CANSTATUS_CAN0MSGFAIL           4  //can0 message send failure.
#define BIT_CANSTATUS_CAN1MSGFAIL           5  //can1 message send failure.
#define BIT_CANSTATUS_CAN0RXMSGERR          6  //can0 Recieve error (timeout or other)
#define BIT_CANSTATUS_CAN1RXMSGERR          7  //can1 Recieve error (timeout or other)


//Define bitmask for testoutputs
#define BIT_TESTHW_ACTIVE                   1  // testactive flag

//define bitmasks for remote output status
#define REMOTE_OUT_OFF                      0
#define REMOTE_OUT_ON                       1
#define REMOTE_OUT_OPENCIRCUIT              2
#define REMOTE_OUT_SHORTCIRCUIT             3
#define REMOTE_OUT_THERMALOVERLOAD          4
#define REMOTE_OUT_CURRENTOVERLOAD          5
#define REMOTE_OUT_unused6                  6
#define REMOTE_OUT_unused7                  7

//define bitmasks for remote input status
#define REMOTE_IN_OFF                      0
#define REMOTE_IN_ON                       1
#define REMOTE_IN_OPENCIRCUIT              2
#define REMOTE_IN_SHORTCIRCUIT             3
#define REMOTE_IN_THERMALOVERLOAD          4
#define REMOTE_IN_CURRENTOVERLOAD          5
#define REMOTE_IN_unused6                  6
#define REMOTE_IN_unused7                  7

#define addressoffset     0x100

#define TACHO_PULSE_HIGH() *tach_pin_port |= (tach_pin_mask)
#define TACHO_PULSE_LOW() *tach_pin_port &= ~(tach_pin_mask)
#define TACHO_PULSE_TOGGLE() *tach_pin_port = *tach_pin_port^tach_pin_mask // Toggle Tacho

//Handy bitsetting macros
#define BIT_SET(a,b)        ((a) |= (1<<(b)))
#define BIT_CLEAR(a,b)      ((a) &= ~(1<<(b)))
#define BIT_WRITE(a,b,c)    (bitWrite(a,b,c))
#define BIT_CHECK(var,pos)  ((var) & (1<<(pos)))               //gives and answer of the decimal value of the binary position being tested if was 1.
#define BIT_sCHECK(var,pos) (((var) & (1<<(pos)))>>pos)       // gives a 1 or 0 answer according to if the bit at pos was 1 or 0

#define BIT_TIMER_1000MS        0
#define BIT_TIMER_500MS         1
#define BIT_TIMER_250MS         2
#define BIT_TIMER_100MS         3
#define BIT_TIMER_75MS          4
#define BIT_TIMER_50MS          5
#define BIT_TIMER_20MS          6
#define BIT_TIMER_5MS           7

//Can RX message status
#define CANRX_MOTECPLM_DFLT    0 // bit 0 is motec PLM.
#define CANRX_UNDEFINEDMSG_DFLT      1 // next message and so on...

/* Global Variables Outside status */
uint8_t tsCanId = 0;          // this is the tunerstudio canID for the device you are requesting data from , this is 0 for the main ecu in the system which is usually the speeduino ecu . 
                              // this value is set in Tunerstudio when configuring your Speeduino
uint8_t thistsCanId = 4;      // this is the tunerstudio canId of this device

const unsigned char ECU_signature[]    = "speeduino_TS_OS_V0.40_dev";       //this must match the ini
const unsigned char ECU_RevNum[]       = "TS_OS_V0.40_dev";      //this is what is displayed in the TS header bar

const uint8_t  data_structure_version = 2; //This identifies the data structure when reading / writing.
const uint8_t  page_1_size = 128;
const uint16_t page_2_size = 374;
const uint16_t page_3_size = 512;
#if defined(EEPROM_SIZE_8KB)
const uint16_t page_4_size = 512;
const uint16_t page_5_size = 512;
#endif

uint8_t currentPage = 0; // TS controlled page for reading and writing EEPROM.

/* The global serial transmit status object.
* All variables in this list will be transmitted to Tuner Studio in the order presented here.
* Its critical that the order of variables here must match exactly what tuner studio .ini file is set up to recieve in the [OutputChannels] section.
* The total size of this variable is captured in ochBlockSizeSent which can be read in Tuner Studio on page 1.
* Its highly reccomended to keep the variable names the same between the .ini file and this code.
*/
typedef struct Out_TS_t
{
  uint8_t secl; // counter of seconds 0-255 looping, required for TS comms.
  uint8_t systembits; //system status bits
  uint8_t LoopDlyWarnBits; //indicator that a l
  uint8_t canstatus;    //canstatus bitfield
  uint16_t canRXmsg_dflt; //check if CAN RX messages are defaulted due to RX timeout
  uint16_t loopsPerSecond;
  uint16_t readsPerSecond; // how many datalog reads in the last sec
  uint16_t UTIL_freeRam;
  uint8_t testIO_hardware;//testIO_hardware
  uint16_t digitalPorts0_15_out;
  uint16_t digitalPorts16_31_out;
  uint16_t digitalPorts32_47_out;
  uint16_t digitalPorts48_63_out;
  uint16_t Analog[16];    // 16bit analog value data array for local analog(0-15)
  
  /* Examples below here, can be removed for your project */
  uint16_t dev1;          //developer use only
  uint16_t dev2;          //developer use only
  uint16_t dev3;          //developer use only
  uint16_t dev4;          //developer use only
  float Vf_i_TestFloatOut;
  uint8_t Ve_i_TestByte1;
  float Ve_Eqr_Sensor1;
  uint16_t Ve_i_example3DXLookup;  // example variables for tracking table axes points in TS.
  uint16_t Ve_i_example3DYLookup;  // example variables for tracking table axes points in TS.
  uint16_t z1;          //developer use only
  uint16_t z2;          //developer use only
  uint16_t z3;          //developer use only
  uint16_t z4;          //developer use only
  uint16_t Xinterp1;          //developer use only
  uint16_t Xinterp2;          //developer use only
};

// this union of structures is to make it easier to transmit multiple data types via serial
typedef union Out_TS_Pac_t
{
  Out_TS_t Vars;
  byte byteData[sizeof(Out_TS_t)];
}; 
Out_TS_Pac_t Out_TS;


typedef struct digitalPorts_t
{
  uint16_t value;
  uint16_t isOverride;
  uint16_t isOutput;
};


//-------------------------------------------------------------------------------
//Page 1 of the config - See the ini file for further reference
struct __attribute__ ( ( packed ) ) config1 
{
  uint16_t page1ActualSize;  //TS READ ONLY: to check page size in development. This should never exceed the defined page sizes.
  uint32_t page1CRC;         //TS READ ONLY: Future expansion EEPROM CRC for error checking.
  uint8_t ochBlockSizeSent;  //TS READ ONLY: to check the serial data size for development. Match this with "ochBlockSize" parameter in the TS .ini file
  
  // 8 Bits in a byte for CAN config
  uint8_t can0Enable: 1;                 //bit flags for canmodule configuration
  uint8_t can0Baud: 4;             // CANO Baud Rate defined in MCP_CAN library v1.5 mcp_can_dfs.h
  uint8_t can0XTalFreq: 2;          // MCP2515 board chip frequency defined in MCP_CAN library v1.5 mcp_can_dfs.h
  uint8_t can0Unusedbits: 1;
  uint8_t can0RXIntPin: 6;
  uint8_t unused1_8_bits: 2;

  uint8_t analogSelectorPin: 4;
  uint8_t analogSelectorEn: 1;
  uint8_t allowHWTestMode: 1;  // EEPROM based lockout of the hardware test mode. Prevents inadvertent serial data from accidently enabling this mode.
  uint8_t allowEEPROMClear: 1;  // EEPROM based lockout of the EEPROM wipe function. Prevents inadvertent serial data from accidently enabling this mode.
  uint8_t unused1_9_bits: 1;
  
  uint16_t canRXmsg_MotecPLM; // can Address for motec PLM message.


//#if defined(CORE_AVR)
};
//#else
//  } __attribute__((__packed__)); //The 32 bi systems require all structs to be fully packed
//#endif

//};

//-------------------------------------------------------------------------------
//Page 2 of the config - See the ini file for further reference
struct __attribute__ ( ( packed ) ) config2 
{
  uint16_t page2ActualSize;  //TS READ ONLY: to check page size in development. This should never exceed the defined page sizes.
  uint32_t page2CRC;         //TS READ ONLY: Future expansion EEPROM CRC for error checking.
  
  /* Examples below here, can be removed for your project */
  
  uint8_t exampleTable_Xaxis[8]; //8 byte axis points
  uint8_t exampleTable_Ydata[8]; //8 byte data
  uint8_t exampleLookupValue; // lookup value for table

  int16_t Ke_i_TestValue;      // 2 byte signed variable
  float Kf_i_TestFloat1;       // 4 bytes float in arduino world
  float Kf_i_TestFloat2;       // 4 bytes float in arduino world
  
  uint8_t example3DTable_Xaxis[8]; //8 byte axis points
  uint8_t example3DTable_Yaxis[4]; //4 byte axis points
  uint8_t example3DTable_Zdata[32]; // 8x4 bytes of data.
  uint8_t example3DXLookup; // lookup value for X axis of table
  uint8_t example3DYLookup; // lookup value for X axis of table

//#if defined(CORE_AVR)
};
//#else
//  } __attribute__((__packed__)); //The 32 bit systems require all structs to be fully packed
//#endif
//};

//-------------------------------------------------------------------------------
//Page 3 of the config -canbus broadcast,obd and config- See the ini file for further reference
struct __attribute__ ( ( packed ) ) config3 
{
  uint16_t page3ActualSize;  //TS READ ONLY: to check page size in development. This should never exceed the defined page sizes.
  uint32_t page3CRC;         //TS READ ONLY: Future expansion EEPROM CRC for error checking.

//  uint8_t unused3_0_511[506];

//#if defined(CORE_AVR)
};
//#else
//  } __attribute__((__packed__)); //The 32 bit systems require all structs to be fully packed
//#endif

#if defined(EEPROM_SIZE_8KB)
//-------------------------------------------------------------------------------
//Page 4 of the config - See the ini file for further reference
struct __attribute__ ( ( packed ) ) config4 
{
  uint16_t page4ActualSize;  //TS READ ONLY: to check page size in development. This should never exceed the defined page sizes.
  uint32_t page4CRC;         //TS READ ONLY: Future expansion EEPROM CRC for error checking.
 // uint8_t unused4_0_511[506];

//#if defined(CORE_AVR)
};
//#else
//  } __attribute__((__packed__)); //The 32 bit systems require all structs to be fully packed
//#endif

//-------------------------------------------------------------------------------
//Page 5 of the config - See the ini file for further reference
struct __attribute__ ( ( packed ) ) config5 
{
  uint16_t page5ActualSize;  //TS READ ONLY: to check page size in development. This should never exceed the defined page sizes.
  uint32_t page5CRC;         //TS READ ONLY: Future expansion EEPROM CRC for error checking.

 // byte unused5_0_511[506];                                  //2

//#if defined(CORE_AVR)
};
//#else
//  } __attribute__((__packed__)); //The 32 bit systems require all structs to be fully packed
//#endif
#endif
 //declare io pins

//Pins
byte Pin_can0RXInt;
byte Pin_analogSelector;


// global variables 
extern struct statuses currentStatus; 
extern struct config1 configPage1;  
extern struct config2 configPage2;
extern struct config3 configPage3;
#if defined(EEPROM_SIZE_8KB)
extern struct config4 configPage4;
extern struct config5 configPage5;
#endif

struct digitalPorts_t digitalPorts0_15;
struct digitalPorts_t digitalPorts16_31;
struct digitalPorts_t digitalPorts32_47;
struct digitalPorts_t digitalPorts48_63;
#endif // GLOBALS_H                              
