/*
* accelgyro.ino
* Concerned with functions that are related to the screen display control and the internal accelerometer/gyro.
*
* Copyright (C) Henry Wright
* A full copy of the license may be found in the projects root directory
*
* Based on code by Josh Stewart for the Speeduino project , and Darren Siepka for the GPIO module see www.Speeduino.com for more info
*/

/* Initalisation routine for the gyro */
void INIT_gyro(void)
{
  accelgyro.initialize();
   /*
   *          |   ACCELEROMETER    |           GYROSCOPE
   * DLPF_CFG | Bandwidth | Delay  | Bandwidth | Delay  | Sample Rate
   * ---------+-----------+--------+-----------+--------+-------------
   * 0        | 260Hz     | 0ms    | 256Hz     | 0.98ms | 8kHz
   * 1        | 184Hz     | 2.0ms  | 188Hz     | 1.9ms  | 1kHz
   * 2        | 94Hz      | 3.0ms  | 98Hz      | 2.8ms  | 1kHz
   * 3        | 44Hz      | 4.9ms  | 42Hz      | 4.8ms  | 1kHz
   * 4        | 21Hz      | 8.5ms  | 20Hz      | 8.3ms  | 1kHz
   * 5        | 10Hz      | 13.8ms | 10Hz      | 13.4ms | 1kHz
   * 6        | 5Hz       | 19.0ms | 5Hz       | 18.6ms | 1kHz
   * 7        |   -- Reserved --   |   -- Reserved --   | Reserved
   * */
  accelgyro.setDLPFMode(5); //set up for 10Hz operation
    
   /* Get full-scale accelerometer range.
   * The FS_SEL parameter allows setting the full-scale range of the accelerometer
   * sensors as described in the table below.
   *
   * <pre>
   * 0 = +/- 2g
   * 1 = +/- 4g
   * 2 = +/- 8g
   * 3 = +/- 16g
   */
  accelgyro.setFullScaleAccelRange(0); //set to +/- 2g
  //put offsets in here
  
  if (!(accelgyro.testConnection()))
  { // Check for connection for Accellerometer if failed put warning.
    //Serial.println("Warning: MPU6050 connection failed");
  }
  
  Ve_g_AxRawCompOffset = accelgyro.getXGyroOffset();
  Ve_g_AyRawCompOffset = accelgyro.getYGyroOffset();
}

/* initalise the 8x8 display; */
void INIT_8x8Matrix(void)
{
  matrix.begin(0x70);  // pass in the address
  matrix.clear();
  matrix.writeDisplay();
}
	