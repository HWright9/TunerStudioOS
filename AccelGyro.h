#ifndef ACCELGYRO_H
#define ACCELGYRO_H

#include "Arduino.h"

#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
#include "MPU6050.h"

/*Variables Shared between Functions should have [XXXX_] naming convention where XXXX is the source module identifier*/


/* Functions */

void INIT_gyro(void);
void INIT_8x8Matrix(void);

#endif