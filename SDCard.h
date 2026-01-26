/*
*Init functions
*
* "SDCard.h"
*/ 

#ifndef SDCARD_H
#define SDCARD_H

#include <SPI.h>
#include <SD.h>

#define SDFILE_CLOSED 0
#define SDFILE_OPEN   1
#define SDFILE_ERR    3

/*Variables Shared between Functions should have [XXXX_] naming convention where XXXX is the source module identifier*/

File SDCRD_F_DataFile;


uint8_t Ve_e_SDFileStatus; 


void INIT_SDCARD(void);



#endif //SDCARD_H