#ifndef DIRECTCOMMS_H
#define DIRECTCOMMS_H

#define FULLSTATUS_SIZE 67 // Size of full status array in bytes

/*Variables Shared between Functions should have [XXXX_] naming convention where XXXX is the source module identifier*/
extern uint16_t COMS_readsPerSecCount; 
extern byte COMS_EEPROMBurnCmnd; // if true the burn is currently in progress and awaiting eeprom availabiltiy.


void direct_serial_command();
void dodirect_rCommands(uint8_t commandletter, uint8_t canid, uint16_t theoffset, uint16_t thelength);
void direct_receiveValue(uint16_t rvOffset, uint8_t newValue);
void direct_sendPage(uint16_t send_page_offset,uint16_t send_page_Length, byte can_id, byte cmd);
void direct_read_realtime();
void direct_sendValues(uint16_t offset, uint16_t packetLength, byte cmd);
void commandButtons(uint16_t cmdCombined);

#endif // DIRECTCOMMS_H
