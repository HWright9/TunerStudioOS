/*
Table lookup function header file
*/ 

#ifndef TABLEINTERP_H
#define TABLEINTERP_H

/*Variables Shared between Functions should have [XXXX_] naming convention where XXXX is the source module identifier*/


/* Functions */

uint8_t u8_table2DLookup_u8(uint8_t *Xaxis, uint8_t *tableData, uint8_t tableSize, uint8_t lookupVal );
uint16_t u16_table2DLookup_u16(uint16_t *Xaxis, uint16_t *Ydata, uint16_t tableSize, uint16_t lookupVal );


uint8_t u8_table3DLookup_u8(uint8_t *Xaxis, uint8_t *Yaxis, uint8_t *Zdata, uint8_t tableXSize, uint8_t tableYSize, uint8_t lookupXVal, uint8_t lookupYVal );


#endif //TABLEINTERP_H