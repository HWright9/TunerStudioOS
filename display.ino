/*
* display.ino
* Concerned with functions that are related to the screen display control a.
*
* Copyright (C) Henry Wright
* A full copy of the license may be found in the projects root directory
*
* Based on code by Josh Stewart for the Speeduino project , and Darren Siepka for the GPIO module see www.Speeduino.com for more info
*/

// Global variables
Adafruit_8x8matrix matrix = Adafruit_8x8matrix(); // call to initialise the matrix structure


/* initalise the 8x8 display; */
void INIT_8x8Matrix(void)
{
  matrix.begin(0x70);  // pass in the address
  matrix.clear();
  matrix.writeDisplay();
}
	