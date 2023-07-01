/*
* timers.ino
* Concerned with setup of internal times and the interrupt service routine for the 1ms task which is the basis of the OS task timer.
*
* Copyright (C) Henry Wright
* A full copy of the license may be found in the projects root directory
*
* Based on code by Josh Stewart for the Speeduino project , and Darren Siepka for the GPIO module see www.Speeduino.com for more info
*/

/*
Timers are used for having actions performed repeatedly at a fixed interval (Eg every 100ms)
They should not be confused with Schedulers, which are for performing an action once at a given point of time in the future

Timers are typically low resolution (Compared to Schedulers), with maximum frequency currently being approximately every 5ms
*/
#include "timers.h"

/* Variables local to this module */
volatile uint16_t loop1ms = 5; // Not zero otherwise MOD% compare functions will trigger early. This is closer to the expected behaviour of having the 1000ms task execute ~1sec after init.
volatile uint8_t LoopDelayWarningCntr = 0;
volatile uint8_t TIMR_LoopTmrsBits = 0;
volatile uint8_t TIMR_LoopDlyWarnBits = 0; // Bitfield of warnings that a task was delayed to the point where it was skipped. This will affect any counters in that function.

/* Functions */

void INIT_timers()
{
#if defined(CORE_AVR)
  //#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(ARDUINO_AVR_ATmega324)
   //Configure Timer2 for our low-freq interrupt code.
   
  TCCR2A = 0; //disable timer 2 while we run setup
  TCCR2B = 0; //disable timer 2 while we run setup
  
  TIMSK2 = (1<<OCIE2A);          //Timer2 Set Overflow Interrupt enabled.
  OCR2A = 124;                   //Timer 2 set overflow compapre to 124 should equal 1ms, including the zero.
  TCCR2A = (1<<WGM21);           //Timer2 CTC MODE Clear timer on Compare match, WGM21 and WGM20
  TCNT2  = 0;                     // Zero timer.
  /* Now configure the prescaler to CPU clock divided by 128 = 125Khz */
  TCCR2B = (1<<WGM22) | (1<<CS22)  | (1<<CS20); // Set bits for 128 prescaler.,  WGM22
  TIFR2 = (1<<OCF2A) | (1<<OCF2B) | (1<<TOV2); //Clear the compare flag bits and overflow flag bit

#elif defined (CORE_TEENSY)
   //Uses the PIT timer on Teensy.
   lowResTimer.begin(oneMSInterval, 1000);

#elif defined(MCU_STM32F103C8)
  Timer4.setPeriod(1000);  // Set up period
  // Set up an interrupt
  Timer4.setMode(TIMER_CH1, TIMER_OUTPUT_COMPARE);
  Timer4.attachInterrupt(1, oneMSInterval);

#endif

#if defined(AVR_WDT)
 //Enable the watchdog timer for 4 second resets (Good reference: https://tushev.org/articles/arduino/5/arduino-and-watchdog-timer)
 wdt_enable(WDTO_4S);
#endif

}

/* 1ms Timer ISR */

//Executes every ~1ms.
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)|| defined (ARDUINO_AVR_PRO) || defined(ARDUINO_AVR_ATmega324) //AVR chips use the ISR for this
//******************************************************************
//  Timer2 Interrupt Service is invoked by hardware Timer 2 every 1 ms = 1000 Hz
//  16Mhz / 128 / 125 = 1000 Hz
ISR(TIMER2_COMPA_vect, ISR_NOBLOCK)
#elif defined (CORE_TEENSY) || defined(MCU_STM32F103C8)
void oneMSInterval() //Most ARM chips can simply call a function
#endif
{


  /* Increment Loop Counter, counts up to the highest timer value. 
  * Using the MOD (%) operator to check if the 1ms counter is divisible by the desired loop period
  * The remainder is checked against ==1, ==2, etc to offset the loops so that we try to avoid chaining
  * all the tasks into the same CPU loop which if not completed in 5ms would overrun the next 5ms task 
  * or at least cause variable loop timing.
  */
    
 
  //200Hz loop, 5ms
  if ((loop1ms % 5) == 0)
  {
    if (BIT_CHECK(TIMR_LoopTmrsBits, BIT_TIMER_5MS)) { BIT_SET(TIMR_LoopDlyWarnBits, BIT_TIMER_5MS); } // The loop did not finish (or even start) before we tried to run again.
    BIT_SET(TIMR_LoopTmrsBits, BIT_TIMER_5MS);
  }

  //50Hz loop, 20ms
  if ((loop1ms % 20) == 1)
  {
    if (BIT_CHECK(TIMR_LoopTmrsBits, BIT_TIMER_20MS)) { BIT_SET(TIMR_LoopDlyWarnBits, BIT_TIMER_20MS); } // The loop did not finish (or even start) before we tried to run again.
    BIT_SET(TIMR_LoopTmrsBits, BIT_TIMER_20MS);
  }

  //20Hz loop, 50ms
  if ((loop1ms % 50) == 2)
  {
    if (BIT_CHECK(TIMR_LoopTmrsBits, BIT_TIMER_50MS)) { BIT_SET(TIMR_LoopDlyWarnBits, BIT_TIMER_50MS); } // The loop did not finish (or even start) before we tried to run again.
    BIT_SET(TIMR_LoopTmrsBits, BIT_TIMER_50MS);
  }
  
  //13.33Hz loop, 75ms
  if ((loop1ms % 75) == 2)
  {
    if (BIT_CHECK(TIMR_LoopTmrsBits, BIT_TIMER_75MS)) { BIT_SET(TIMR_LoopDlyWarnBits, BIT_TIMER_75MS); } // The loop did not finish (or even start) before we tried to run again.
    BIT_SET(TIMR_LoopTmrsBits, BIT_TIMER_75MS);
  }
  
  //10Hz loop, 100ms
  if ((loop1ms % 100) == 3)
  {
    if (BIT_CHECK(TIMR_LoopTmrsBits, BIT_TIMER_100MS)) { BIT_SET(TIMR_LoopDlyWarnBits, BIT_TIMER_100MS); } // The loop did not finish (or even start) before we tried to run again.
    BIT_SET(TIMR_LoopTmrsBits, BIT_TIMER_100MS);
  }

  //4Hz loop, 250ms
  if ((loop1ms % 250) == 4)
  {
    if (BIT_CHECK(TIMR_LoopTmrsBits, BIT_TIMER_250MS)) { BIT_SET(TIMR_LoopDlyWarnBits, BIT_TIMER_250MS); } // The loop did not finish (or even start) before we tried to run again.
    BIT_SET(TIMR_LoopTmrsBits, BIT_TIMER_250MS);
  }
  
  //2Hz loop, 500ms
  if ((loop1ms % 500) == 4)
  {
    if (BIT_CHECK(TIMR_LoopTmrsBits, BIT_TIMER_500MS)) { BIT_SET(TIMR_LoopDlyWarnBits, BIT_TIMER_500MS); } // The loop did not finish (or even start) before we tried to run again.
    BIT_SET(TIMR_LoopTmrsBits, BIT_TIMER_500MS);
  }

  //1Hz loop, 1000ms
  if ((loop1ms % 1000) == 4)
  {
    if (BIT_CHECK(TIMR_LoopTmrsBits, BIT_TIMER_1000MS)) { BIT_SET(TIMR_LoopDlyWarnBits, BIT_TIMER_1000MS); } // The loop did not finish (or even start) before we tried to run again.
    BIT_SET(TIMR_LoopTmrsBits, BIT_TIMER_1000MS);
    
    if (LoopDelayWarningCntr >= 2)
    {
      TIMR_LoopDlyWarnBits = 0; // reset warnings
      LoopDelayWarningCntr = 0;
    }
    else if (TIMR_LoopDlyWarnBits > 0) { LoopDelayWarningCntr++; }
    else { LoopDelayWarningCntr = 0; }
  
    //**************************************************************************************************************************************************
    //This records the number of main loops the system has completed in the last second
    currentStatus.loopsPerSecond = mainLoopCount;
    mainLoopCount = 0;
    //**************************************************************************************************************************************************
    //increament secl (secl is simply a counter that increments every second and is used to track whether the system has unexpectedly reset
    currentStatus.secl++;
    //**************************************************************************************************************************************************
  }
  if (loop1ms > 998) { loop1ms = 0; } //Reset counter for next loop. Note 0-999ms = 1sec. 
  else { loop1ms++; }
  
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)|| defined (ARDUINO_AVR_PRO) || defined(ARDUINO_AVR_ATmega324)  //AVR chips use the ISR for this
    //Reset Timer2 to trigger in another ~1ms
    //TCNT2 = 132;            //Preload timer2 with 132 cycles, leaving 124 till overflow.
    TIFR2  = 0x00;          //Timer2 INT Flag Reg: Clear Timer Overflow Flag
#endif
}
