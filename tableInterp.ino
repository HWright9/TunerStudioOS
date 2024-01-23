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
* A single variable defines the length of line but cannot give direction, this is one dimension.
* A table with one lookup axis defines the shape of a line on a flat page, this is two dimensions.
* A map with two lookup axes defines a surface like a curved piece of paper, this is three dimensions.
*/

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

/* Unsigned 3D table lookup using all unsigned bytes (16bit)
* Xaxis must be either all increasing or all decreasing
* Yaxis must be either all increasing or all decreasing
* Zdata can be increasing and decreasing. */
uint16_t u16_table3DLookup_u16(uint16_t *Xaxis, uint16_t *Yaxis, uint16_t *Zdata, uint8_t tableXSize, uint8_t tableYSize, uint16_t lookupXVal, uint16_t lookupYVal )
{
  uint16_t returnVal;
  uint8_t i,j;
  uint16_t x1, x2, y1, y2, Zx1y1, Zx1y2, Zx2y1, Zx2y2;
  
  int32_t Xdiff, Ydiff, Zdiff, temp, m, Xinterp1, Xinterp2;
  
  //detect increasing or decreasing X axis
  if (Xaxis[1] >= Xaxis[0]) //increasing
  {
    // Short circuit for lookup off the ends of the table
    if (lookupXVal <= Xaxis[0]) { i = 0; x1 = Xaxis[i]; x2 = x1; }
    else if (lookupXVal >= Xaxis[tableXSize-1]) { i = tableXSize-1; x1 = Xaxis[i]; x2 = x1; }
    else 
    { // iterate through table to find axis points either side of the lookup value.
      i = 1;
      while (i < tableXSize)
      {
        if (lookupXVal < Xaxis[i])
        { 
          x1 = Xaxis[i-1];
          x2 = Xaxis[i];
          break;
        }
        else
        {
          i++;
        }
      }
    }
  }
  else //decreasing X axis
  {
    // Short circuit for lookup off the ends of the table
    if (lookupXVal >= Xaxis[0]) { i = 0; x1 = Xaxis[i]; x2 = x1; }
    else if (lookupXVal <= Xaxis[tableXSize-1]) { i = tableXSize-1; x1 = Xaxis[i]; x2 = x1; }
    else
    { // iterate through table to find axis points either side of the lookup value.
      i = 1;
      while (i < tableXSize)
      {
        if (lookupXVal > Xaxis[i])
        { 
          x1 = Xaxis[i];
          x2 = Xaxis[i-1];
          break;
        }
        else
        {
          i++;
        }
      }
    }
  }

  //detect increasing or decreasing Y axis
  if (Yaxis[1] >= Yaxis[0]) // increasing
  {
    // Short circuit for lookup off the end of the table
    if (lookupYVal <= Yaxis[0]) { j = 0; y1 = Yaxis[j]; y2 = y1; }
    else if (lookupYVal >= Yaxis[tableYSize-1]) { j = tableYSize-1; y1 = Yaxis[j]; y2 = y1; }
    else
    { // iterate through table to find axis points either side of the lookup value.
      j = 1;
      while (j < tableYSize)
      {
        if (lookupYVal < Yaxis[j])
        { 
          y1 = Yaxis[j-1];
          y2 = Yaxis[j];
          break;
        }
        else
        {
          j++;
        }
      }
    }
  }
  else //decreasing Y axis
  {
    // Short circuit for lookup off the end of the table
    if (lookupYVal >= Yaxis[0]) { j = 0; y1 = Yaxis[j]; y2 = y1; }
    else if (lookupYVal <= Yaxis[tableYSize-1]) { j = tableYSize-1; y1 = Yaxis[j]; y2 = y1; }
    else
    { // iterate through table to find axis points either side of the lookup value.
      j = 1;
      while (j < tableYSize)
      {
        if (lookupYVal > Yaxis[j])
        { 
          y1 = Yaxis[j];
          y2 = Yaxis[j-1];
          break;
        }
        else
        {
          j++;
        }
      }
    }
  }

/* 
* interpolation performed along x axis first for both y axis points, then interpolated along y axis, using the previously interpolated values.
*
*     y1  Z[i-1,j]  ---interp1--- Z[i,j]
*     |                 |
*     |                 |
*     |                 |
*    YVal               returnVal
*     |                 |
*     |                 |
*
*     y2  Z[i-1,j-1]---interp2--- Z[i,j-1]                
*         x2-----------XVal-------x1
*
*/

  // account for i and/or j at 0.
  // Zx1y1 = Zdata[i,j];
  // if (i > 0) { Zx2y1 = Zdata[i-1,j]; } else { Zx2y1 = Zdata[i,j]; }
  // if (j > 0) { Zx1y2 = Zdata[i,j-1]; } else { Zx1y2 = Zdata[i,j]; }
  // if (i > 0) 
  // { 
    // if (j > 0) { Zx2y2 = Zdata[i-1,j-1]; }
    // else { Zx2y2 = Zdata[i-1,j]; }
  // }
  // else 
  // {  
    // if (j > 0) { Zx2y2 = Zdata[i,j-1]; }
    // else { Zx2y2 = Zdata[i,j]; }
  // }  

  Zx1y1 = *((Zdata + i * tableYSize) + j);
  if (i > 0) { Zx2y1 = *((Zdata + (i - 1) * tableYSize) + j); } else { Zx2y1 = Zx1y1; }
  if (j > 0) { Zx1y2 = *((Zdata + i * tableYSize) + (j - 1)); } else { Zx1y2 = Zx1y1; }
  if (i > 0) 
  { 
    if (j > 0) { Zx2y2 = *((Zdata + (i - 1) * tableYSize) + (j - 1)); }
    else { Zx2y2 = *((Zdata + (i - 1) * tableYSize) + j); }
  }
  else 
  {  
    if (j > 0) { Zx2y2 = *((Zdata + i * tableYSize) + (j - 1)); }
    else { Zx2y2 = Zx1y1; }
  }    

  // First interpolation along x axis at j-1
  if (lookupXVal < x1) // Check to make sure we are not off the end of the table.
  { 
    m = lookupXVal - x1;
    Xdiff = x2 - x1;
    Zdiff = Zx1y2 - Zx2y2;
    temp = ( m * Zdiff ) / Xdiff;
    Xinterp1 = Zx2y2 + (uint16_t)temp;
    
    // Second interpolation along x axis at j, m and Xdiff are the same as before.
    Zdiff = Zx1y1 - Zx2y1;
    temp = ( m * Zdiff ) / Xdiff;
    Xinterp2 = Zx2y1 + (uint16_t)temp;
  }
  else 
  { // off the end of the table in x direction.
    Xinterp1 = Zx2y2;
    Xinterp2 = Zx2y1;
  }
  
  // third interpolation between Xinterp 1 and 2 along yaxis, need to calculate Ydiff.
  if (lookupYVal < y1) // Check to make sure we are not off the end of the table.
  { 
    m = lookupYVal - y1;
    Ydiff = y2 - y1;
    Zdiff = Xinterp2 - Xinterp1;
    temp = ( m * Zdiff ) / Ydiff;
    returnVal = Xinterp1 + (uint16_t)temp;
  }
  else
  { // off the end of the table in y direction.
    returnVal = Xinterp1;
  }
  
 return returnVal;
}
