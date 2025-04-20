// Client-server API(PICO)        //
// Additional functions           //
// Version 0.9.0pi                //
// Bachelor's Work Project        //
// Technical University of Kosice //
// 23.02.2025                     //
// Nikita Kuropatkin              //
// Version for MCU                //
// W5100S-EVB-Pico                //

/* 
This header file declares additional functions, 
like printing help, ip validation, etc. for a 
client-server application. Function bodies
are in addition.c. 
*/
#ifndef ADDITION_H
#define ADDITION_H
#include <stdint.h>

////////////////////////
/// Clearing Input   ///
////////////////////////
/*
The purpose of this function is to clear the `stdin` buffer after  
an oversized message is entered (to handle `stdin` overflow).
*/
void clear (void);
////////////////////////
////////////////////////

///////////////////////////
/// Exit on Stop-Word   ///
///////////////////////////
/*
The purpose of this function is to terminate the program  
when a stop-word is detected in the message.  
The function returns a value of 1 if the stop-word is detected.  
After returning, the program (server or client) proceeds with  
closing the communication session as normal.
- The `side` variable is a modifier for the message that indicates  
  who ended the communication.  
- The stop-word can be configured by changing the `EXIT` macro.  
*/
int exiting (const char *side, const char *msg);
///////////////////////////
///////////////////////////


#endif