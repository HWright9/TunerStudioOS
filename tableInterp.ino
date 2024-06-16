/*
* tableInterp.ino
* Table lookup routines for linear interpolation
*
* Copyright (C) Henry Wright
* A full copy of the license may be found in the projects root directory
*
*/

#include "tableInterp.h"

/* 
* General note about dimensions in tables:
* People often confuse the number of 'dimensions' in a table with the number of 'lookup axes.'
* Using a geometric example: 
* A single variable defines the length of line but cannot give direction, this is one dimension. i.e. X
* A table with one lookup axis defines the shape of a line on a flat page, this is two dimensions. i.e. XY
* A map with two lookup axes defines a surface like a curved piece of paper, this is three dimensions. i.e. XYZ
*/

/* Variables local to this function */

/* Functions */

/* Unsigned table lookup using all unsigned bytes (8bit) 
* Xaxis must be either all increasing or all decreasing for this to work 
* Ydata can be increasing and decreasing. */
uint8_t u8_table2DLookup_u8(uint8_t *Xaxis, uint8_t *Ydata, uint8_t tableSize, uint8_t lookupVal )
{
  uint8_t returnVal;
  uint8_t i;
  int32_t temp, Xdiff, Ydiff, m;
  
  
  if (tableSize < 2) { return Ydata[0]; } // error table size too small
  
  //detect increasing or decreasing X axis  
  if (Xaxis[tableSize-1] >= Xaxis[0]) // Increasing or same
  {
    // Short circuit for lookup off the ends of the X axis
    if (lookupVal <= Xaxis[0]) { return Ydata[0]; }
    if (lookupVal >= Xaxis[tableSize-1]) { return Ydata[tableSize-1]; }
    
    i = 1;
    while (i < tableSize)
    {
      if (lookupVal < Xaxis[i]) { break; } // We have found that LookupVal is between this axis point @ i and the point before it.
      else { i++; }
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
      if (lookupVal > Xaxis[i]) { break; }
      else { i++; } // We have found that LookupVal is between this axis point @ i and the point before it.
    }
  }

  m = lookupVal - Xaxis[i-1];
  Xdiff = Xaxis[i] - Xaxis[i-1];
  Ydiff = Ydata[i] - Ydata[i-1];
  returnVal = uint8_t(Ydata[i-1] + (( m * Ydiff ) / Xdiff));
  
 return returnVal;
}

/* Unsigned table lookup using all unsigned bytes (16bit) 
* Xaxis must be either all increasing or all decreasing for this to work 
* Ydata can be increasing and decreasing. */
uint16_t u16_table2DLookup_u16(uint16_t *Xaxis, uint16_t *Ydata, uint16_t tableSize, uint16_t lookupVal )
{
  uint16_t returnVal;
  uint8_t i, sign;
  uint32_t temp, Xdiff, Ydiff, m;
  
  tableSize = tableSize / sizeof(tableSize); // tableSize is byte count, but we need a count of array elements.
  
  if (tableSize < 2) { return Ydata[0]; } // error table size too small
  
  //detect increasing or decreasing X axis  
  if (Xaxis[tableSize-1] >= Xaxis[0]) // Increasing or same
  {
    // Short circuit for lookup off the ends of the X axis
    if (lookupVal <= Xaxis[0]) { return Ydata[0]; }
    if (lookupVal >= Xaxis[tableSize-1]) { return Ydata[tableSize-1]; }
    
    i = 1;
    while (i < tableSize)
    {
      if (lookupVal < Xaxis[i]) { break; } // We have found that LookupVal is between this axis point @ i and the point before it.
      else { i++; }
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
      if (lookupVal > Xaxis[i]) { break; }
      else { i++; } // We have found that LookupVal is between this axis point @ i and the point before it.
    }
  }
  
  sign = 0;
  // Due to multiplication and division we need the full 32bit unsigned integer. AVR does not support 64bit variables so need to use unsigned math with a seperate sign signal.
  if(lookupVal < Xaxis[i-1]) { m = Xaxis[i-1] - lookupVal; BIT_CLEAR(sign,0); } // Negative 3 -
  else { m = lookupVal - Xaxis[i-1]; BIT_SET(sign,0); } // Positive
  
  if(Xaxis[i] < Xaxis[i-1]) { Xdiff = Xaxis[i-1] - Xaxis[i]; BIT_TOGGLE(sign,0);} // Invert sign if negative 4 +
  else { Xdiff = Xaxis[i] - Xaxis[i-1];} // Positive
  
  if(Ydata[i] < Ydata[i-1]) { Ydiff = Ydata[i-1] - Ydata[i]; BIT_TOGGLE(sign,0); } // Invert sign if negative
  else { Ydiff = Ydata[i] - Ydata[i-1]; } // Positive 73 +
  
  if (BIT_CHECK(sign,0) == true) // Positive
  {
	  returnVal = uint16_t((uint32_t)Ydata[i-1] + (( m * Ydiff ) / Xdiff)); // 127+54 = 181
  }
  else
  {
	  returnVal = uint16_t((uint32_t)Ydata[i-1] - (( m * Ydiff ) / Xdiff));
  }
  
  Out_TS.Vars.z1 = m;
  Out_TS.Vars.z2 = Xdiff;
  Out_TS.Vars.z3 = Ydiff;
  
 return returnVal;
}

/* Unsigned 3D table lookup using all unsigned bytes (8bit)
* Xaxis must be either all increasing or all decreasing
* Yaxis must be either all increasing or all decreasing
* Zdata can be increasing and decreasing. */
uint8_t u8_table3DLookup_u8(uint8_t *Xaxis, uint8_t *Yaxis, uint8_t *Zdata, uint8_t tableXSize, uint8_t tableYSize, uint8_t lookupXVal, uint8_t lookupYVal )
{
  uint8_t returnVal;
  uint8_t i,j, Xmax, Ymax;
  uint8_t z1, z2, z3, z4, Xinterp1, Xinterp2;
  
  int32_t Xdiff, Ydiff, Zdiff, m;
  
  if ((tableXSize < 2) || (tableYSize < 2)) { return Zdata[0]; } // error table size too small
  
  //detect increasing or decreasing X axis
  if (Xaxis[tableXSize-1] >= Xaxis[0]) //increasing
  {
    Xmax = Xaxis[tableXSize-1];
    // Short circuit for lookup off the ends of the table
    if (lookupXVal <= Xaxis[0]) { i = 0; }
    else if (lookupXVal >= Xmax) { i = tableXSize-1; }
    else 
    {
      i = 1;
      while (i < tableXSize)
      {
        if (lookupXVal < Xaxis[i]) { break; } // We have found that LookupVal is between this axis point @ i and the point before it.
        else { i++; }
      }
    }
  }
  else //decreasing X axis
  {
    Xmax = Xaxis[0];
    // Short circuit for lookup off the ends of the table
    if (lookupXVal >= Xmax) { i = 0; }
    else if (lookupXVal <= Xaxis[tableXSize-1]) { i = tableXSize-1; }
    else
    { // iterate through table to find axis points either side of the lookup value.
      i = 1;
      while (i < tableXSize)
      {
        if (lookupXVal > Xaxis[i]) { break; } // We have found that LookupVal is between this axis point @ i and the point before it.
        else { i++; }
      }
    }
  }

  //detect increasing or decreasing Y axis
  if (Yaxis[tableYSize-1] >= Yaxis[0]) // increasing
  {
    Ymax = Yaxis[tableYSize-1];
    // Short circuit for lookup off the end of the table
    if (lookupYVal <= Yaxis[0]) { j = 0; }
    else if (lookupYVal >= Ymax) { j = tableYSize-1; }
    else
    {
      j = 1;
      while (j < tableYSize)
      {
        if (lookupYVal < Yaxis[j]) { break; } // We have found that LookupVal is between this axis point @ i and the point before it.
        else { j++; }
      }
    }
  }
  else //decreasing Y axis
  {
    Ymax = Yaxis[0];
    // Short circuit for lookup off the end of the table
    if (lookupYVal >= Ymax) { j = 0; }
    else if (lookupYVal <= Yaxis[tableYSize-1]) { j = tableYSize-1; }
    else
    { // iterate through table to find axis points either side of the lookup value.
      j = 1;
      while (j < tableYSize)
      {
        if (lookupYVal > Yaxis[j]) { break; }
        else { j++; }
      }
    }
  }
/* 
* interpolation performed along x axis first for both y axis points, then interpolated along y axis, using the previously interpolated values.
*
*     y2  Z[i-1,j] (z1)   ---interp1--- Z[i,j] (z2)
*     |                        |
*     |                        |
*     |                        |
*    YVal                      returnVal
*     |                        |
*     |                        |
*     y1  Z[i-1,j-1] (z3) ---interp2--- Z[i,j-1] (z4)               
*         x1----------------- XVal----------x2
*
*/

  //Calculate Z data points z1, z2, z3 and z4 as defined above by looking up Zdata at i and j
  z2 = *(Zdata + (j*tableXSize) + i);
  
  if (i == 0) { z1 = z2; } // Bound Z data to table boundaries
  else { z1 = *(Zdata + (j*tableXSize) + (i-1)); }
  
  if (j == 0) { z4 = z2; } // Bound Z data to table boundaries
  else { z4 = *(Zdata + ((j-1)*tableXSize) + i); }
  
  if (i == 0) { i = 1; } 
  if (j == 0) { j = 1; }
  z3 = *(Zdata + ((j-1)*tableXSize) + (i-1)); // Bound Z data to table boundaries
  
  // Out_TS.Vars.z1 = z1;
  // Out_TS.Vars.z2 = z2;
  // Out_TS.Vars.z3 = z3;
  // Out_TS.Vars.z4 = z4;
  
  // First interpolation along x axis at j
  if (lookupXVal < Xmax) // Check to make sure we are not off the end of the table.
  {
    m = lookupXVal - Xaxis[i-1];
    Xdiff = Xaxis[i] - Xaxis[i-1];
    Zdiff = z2 - z1;
    Xinterp1 = uint8_t(z1 + (( m * Zdiff ) / Xdiff));  
    
    // Second interpolation along x axis at j-1. Note m and Xdiff are the same as before.
    Zdiff = z4 - z3;
    Xinterp2 = uint8_t(z3 + (( m * Zdiff ) / Xdiff));
  }
  else 
  { // off the end of the table in x direction, no interpolation required.
    Xinterp1 = z2;
    Xinterp2 = z4;
  }
  
  // third interpolation between Xinterp 1 and 2 along y axis, need to calculate Ydiff.
  if (lookupYVal < Ymax) // Check to make sure we are not off the end of the table.
  {
    m = lookupYVal - Yaxis[j-1];
    Ydiff = Yaxis[j] - Yaxis[j-1];
    Zdiff = Xinterp1 - Xinterp2;
    returnVal = uint8_t(Xinterp2 + (( m * Zdiff ) / Ydiff));  
  }
  else
  { // off the end of the table in y direction, no interpolation required.
    returnVal = Xinterp1;
  }
  
  // Out_TS.Vars.Xinterp1 = Xinterp1;
  // Out_TS.Vars.Xinterp2 = Xinterp2;
  
 return returnVal;
}
