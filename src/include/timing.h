// Client-server API(PICO)        //
// System clock configuration     //
// Version 0.9.0pi                //
// Bachelor's Work Project        //
// Technical University of Kosice //
// 23.02.2025                     //
// Nikita Kuropatkin              //
// Version for MCU                //
// W5100S-EVB-Pico                //

/* 
This header file declares functions for configuring the timing system 
and managing timers in the application. These functions are used to 
set up the system's operating frequency. 
Function definitions are in the timing.c file. 
*/

#ifndef TIMING_H
#define TIMING_H
#include "time.h"

/*
This function's purpose is to configure the system clock and 
peripheral clock to operate at PLL_SYS_KHZ frequency.
Easier explanation: we are setting up these clocks 
to configure the chip`s operating frequency as PLL_SYS_KHZ.
*/
void set_clock_khz(void);

/*
Function that repeatedly increments the counter every millisecond.
*/
void repeating_timer_callback(void);

/*
Returns the millisecond counter value.
It means the number of milliseconds that 
have passed from the start of the program.
*/
time_t millis(void);

#endif
