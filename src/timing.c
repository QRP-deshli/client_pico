// Client-server API(PICO)        //
// System clock configuration     //
// Version 0.9.0pi                //
// Bachelor's Work Project        //
// Technical University of Kosice //
// 23.02.2025                     //
// Nikita Kuropatkin              //
// Version for MCU                //
// W5100S-EVB-Pico                //

#include "include/timing.h"
#include "include/parameters.h"
#include "port_common.h"

/* 
Initial value for a global millisecond counter.
Tracks milliseconds since the program started.
*/
static volatile uint32_t g_msec_cnt = 0;

/*
This function's purpose is to configure the system clock and 
peripheral clock to operate at PLL_SYS_KHZ frequency.
Easier explanation: we are setting up these clocks 
to configure the chip`s operating frequency  
*/
void set_clock_khz(void) {
 // Set the system clock (clk_sys) frequency in kilohertz (kHz).
 set_sys_clock_khz(PLL_SYS_KHZ, true);

 /*
 Set the peripheral clock (clk_peri) to use the System PLL (PLL_SYS).
 */
 clock_configure(
    /* The peripheral clock to configure. */
    clk_peri,   
    /* No glitchless multiplexer. */                                     
    0,                                              
    /* Use the System PLL as the clock source. */
    CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, 
    /* Input clock frequency in Hz, thats why we need to multiply by 1000 */
    PLL_SYS_KHZ * 1000,                             
    /* Output clock frequency in Hz. */
    PLL_SYS_KHZ * 1000                             
 );
}

/*
Function that repeatedly increments the counter every millisecond.
*/
void repeating_timer_callback(void) {
 g_msec_cnt++;  
}

/*
Returns the millisecond counter value.
It means the number of milliseconds that 
have passed from the start of the program.
*/
time_t millis(void) {
 return g_msec_cnt;  
}

