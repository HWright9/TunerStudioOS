TunerStudio_OS_dev

Tuner Studio Operating System Development Copyright (C) Henry Wright
Based on the works of GPIO STD V1.000.
GPIO STD V1.000 is Copyright (C) Darren Siepka 
Speeduino is Copyright (C) Josh Stewart

A full copy of the license may be found in the projects root directory 
A full copy of the license may be found in the speeduino projects root directory

This program is intended to run on Arduino based systems programed via the Arduino IDE such as the Arduino Mega2560. 
It enables communication with Tuner Studio (TS), a calibration and data acquisition tool available from  https://www.tunerstudio.com/

The basic functionality is to:
1.	Define variables which are saved to and recalled from the internal memory (EEPROM) of the Arduino.
2.	Send the variables in memory to TS and receive new data when variables in TS are changed.
3.	Write (Burn) these variables to the EEPROM.
4.	Send measurement variables at a defined rate to TS for display and logging.
5.	Receive commands to execute certain functions on the Arduino. i.e. switch an output on or off.
6.	Provide a task based timing system to execute tasks with feedback if those tasks have overrun.

In addition, a simple CAN_BUS broadcast implementation using the MCP2515 transceiver over spi is implemented.
It is up to the user to build their own functionality into this operating system.
The Latest release can be found in the ->release sub directory.
Documentation can be found in the ->docs sub directory.
Latest version of this program can be found on github.


July-2023 HRW.