

/*
NOTE - This file and it's associated functions need a CLEARER NAME

//Purpose
We're implementing a lower frequency interrupt loop to perform calculations that are needed less often, some of which depend on time having passed (delta/time) to be meaningful.


//Technical
Timer2 is only 8bit so we are setting the prescaler to 128 to get the most out of it. This means that the counter increments every 0.008ms and the overflow at 256 will be after 2.048ms
Max Period = (Prescale)*(1/Frequency)*(2^8)
(See http://arduinomega.blogspot.com.au/2011/05/timer2-and-overflow-interrupt-lets-get.html)

We're after a 1ms interval so we'll need 131 intervals to reach this ( 1ms / 0.008ms per tick = 125).
Hence we will preload the timer with 131 cycles to leave 125 until overflow (1ms).

*/
#ifndef TIMERS_H
#define TIMERS_H

#include "globals.h"
#if defined(AVR_WDT)
  #include <avr/wdt.h>
#endif

#if defined(CORE_AVR)
//#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)|| defined (ARDUINO_AVR_PRO) 
  #include <avr/interrupt.h>
  #include <avr/io.h>
  
  // Prescaler values for timers 1-3-4-5. Refer to www.instructables.com/files/orig/F3T/TIKL/H3WSA4V7/F3TTIKLH3WSA4V7.jpg
  #define TIMER_PRESCALER_OFF  ((0<<CS12)|(0<<CS11)|(0<<CS10))
  #define TIMER_PRESCALER_1    ((0<<CS12)|(0<<CS11)|(1<<CS10))
  #define TIMER_PRESCALER_8    ((0<<CS12)|(1<<CS11)|(0<<CS10))
  #define TIMER_PRESCALER_64   ((0<<CS12)|(1<<CS11)|(1<<CS10))
  #define TIMER_PRESCALER_256  ((1<<CS12)|(0<<CS11)|(0<<CS10))
  #define TIMER_PRESCALER_1024 ((1<<CS12)|(0<<CS11)|(1<<CS10))

  #define TIMER_MODE_NORMAL    ((0<<WGM01)|(0<<WGM00))
  #define TIMER_MODE_PWM       ((0<<WGM01)|(1<<WGM00))
  #define TIMER_MODE_CTC       ((1<<WGM01)|(0<<WGM00))
  #define TIMER_MODE_FASTPWM   ((1<<WGM01)|(1<<WGM00))

#endif


/*Variables Shared between Functions should have [XXXX_] naming convention where XXXX is the source module identifier*/
extern volatile uint8_t TIMR_LoopTmrsBits;
extern volatile uint8_t TIMR_LoopDlyWarnBits;

/* Functions */

  
#if defined (CORE_TEENSY)
  IntervalTimer lowResTimer;
  void oneMSInterval();
#elif defined(CORE_STM32)
  void oneMSInterval();
#endif  


void INIT_timers();

#endif // TIMERS_H
