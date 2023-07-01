/*
* tableInterp.ino
* Table lookup routines for linear interpolation
*
* Copyright (C) Henry Wright
* A full copy of the license may be found in the projects root directory
*
*/

#include "tableInterp.h"

/* Variables local to this function */

/* Functions */

/* Unsigned table lookup using all unsigned bytes (16bit)
* Xaxis must be either all increasing or all decreasing for this to work 
* Ydata can be increasing and decreasing. */
uint16_t u16_table2DLookup_u16(uint16_t *Xaxis, uint16_t *Ydata, uint8_t tableSize, uint16_t lookupVal )
{
  uint16_t returnVal;
  uint8_t i;
  uint16_t x1, x2, y1, y2;
  int32_t Xdiff, Ydiff, m;
  bool increasing = false;
  
  //detect increasing or decreasing X axis
  if (Xaxis[1] >= Xaxis[0]) { increasing = true; } 
  
  if (increasing == true)
  {
    // Short circuit for lookup off the end of the table
    if (lookupVal <= Xaxis[0]) { return Ydata[0]; }
    if (lookupVal >= Xaxis[tableSize-1]) { return Ydata[tableSize-1]; }
    
    i = 1;
    while (i < tableSize)
    {
      if (lookupVal < Xaxis[i])
      { 
        x1 = Xaxis[i-1];
        x2 = Xaxis[i];
        y1 = Ydata[i-1];
        y2 = Ydata[i];
        i = tableSize;
      }
      else
      {
        i++;
      }
    }
  }
  else //decreasing X axis
  {
    // Short circuit for lookup off the end of the table
    if (lookupVal >= Xaxis[0]) { return Ydata[0]; }
    if (lookupVal <= Xaxis[tableSize-1]) { return Ydata[tableSize-1]; }
    
    i = 1;
    while (i < tableSize)
    {
      if (lookupVal > Xaxis[i])
      { 
        x1 = Xaxis[i];
        x2 = Xaxis[i-1];
        y1 = Ydata[i];
        y2 = Ydata[i-1];
        i = tableSize;
      }
      else
      {
        i++;
      }
    }
  }

  m = lookupVal - x1;
  Xdiff = x2 - x1;
  Ydiff = y2 - y1;

  int32_t temp;
  //temp = ((Ydiff * 256) / Xdiff) / 256;

  temp = ( m * Ydiff ) / Xdiff;
  returnVal = y1 + (uint16_t)temp;
 return returnVal;
}

/* Unsigned table lookup using all unsigned bytes (8bit) 
* Xaxis must be either all increasing or all decreasing for this to work 
* Ydata can be increasing and decreasing. */
uint8_t u8_table2DLookup_u8(uint8_t *Xaxis, uint8_t *Ydata, uint8_t tableSize, uint8_t lookupVal )
{
  uint8_t returnVal;
  uint8_t i;
  uint8_t x1, x2, y1, y2;
  int32_t Xdiff, Ydiff, m;
  bool increasing = false;
  
  //detect increasing or decreasing X axis
  if (Xaxis[1] >= Xaxis[0]) { increasing = true; } 
  
  if (increasing == true)
  {
    // Short circuit for lookup off the end of the table
    if (lookupVal <= Xaxis[0]) { return Ydata[0]; }
    if (lookupVal >= Xaxis[tableSize-1]) { return Ydata[tableSize-1]; }
    
    i = 1;
    while (i < tableSize)
    {
      if (lookupVal < Xaxis[i])
      { 
        x1 = Xaxis[i-1];
        x2 = Xaxis[i];
        y1 = Ydata[i-1];
        y2 = Ydata[i];
        i = tableSize;
      }
      else
      {
        i++;
      }
    }
  }
  else //decreasing X axis
  {
    // Short circuit for lookup off the end of the table
    if (lookupVal >= Xaxis[0]) { return Ydata[0]; }
    if (lookupVal <= Xaxis[tableSize-1]) { return Ydata[tableSize-1]; }
    
    i = 1;
    while (i < tableSize)
    {
      if (lookupVal > Xaxis[i])
      { 
        x1 = Xaxis[i];
        x2 = Xaxis[i-1];
        y1 = Ydata[i];
        y2 = Ydata[i-1];
        i = tableSize;
      }
      else
      {
        i++;
      }
    }
  }

  m = lookupVal - x1;
  Xdiff = x2 - x1;
  Ydiff = y2 - y1;

  int32_t temp;
  //temp = ((Ydiff * 256) / Xdiff) / 256;

  temp = ( m * Ydiff ) / Xdiff;
  returnVal = y1 + (uint8_t)temp;
 return returnVal;
}