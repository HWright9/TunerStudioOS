#ifndef STORAGE_H
#define STORAGE_H

#include <EEPROM.h>




/*
Current layout of EEPROM data (Version 1) is as follows (All sizes are in bytes):
Note that last page byte is end - 1
|---------------------------------------------------|
|Byte # |Size | End | Description                   |
|---------------------------------------------------|
| 0     |1    |1    | Data structure version        |
| 1     |9    |10   | Reserved                      |
| 10    |128  |138  | configPage1
| 138   |374  |512  | configpage2
| 512   |512  |1024 | configPage3
| 1024  |512  |1536 | configPage4
| 1536  |512  |2048 | configPage5
*/

#define EEPROM_CONFIG1_START 10
#define EEPROM_CONFIG1_END 138     // 128 slots
#define EEPROM_CONFIG2_START 138
#define EEPROM_CONFIG2_END 511     //374 slots
#define EEPROM_CONFIG3_START 512
#define EEPROM_CONFIG3_END 1023     //512 slots
// Note ATmega328P (uno) has 1023 bytes of EEPROM
#define EEPROM_CONFIG4_START 1024
#define EEPROM_CONFIG4_END 1535   //512 slots
#define EEPROM_CONFIG5_START 1536
#define EEPROM_CONFIG5_END 2048   //512 slots
// ATmega1280 and ATmega2560 have 4096 bytes

/*Variables Shared between Functions should have [XXXX_] naming convention where XXXX is the source module identifier*/




/* Functions */

void STOR_writeConfig(uint8_t thePage);
void STOR_writeConfigNoBlock(void);
void STOR_loadConfig(void);
void STOR_eraseEEPROM(void);

#endif // STORAGE_H
