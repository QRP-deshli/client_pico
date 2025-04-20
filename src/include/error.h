// Client-server API(PICO)        //
// Error handling                 //
// Version 0.9.0pi                //
// Bachelor's Work Project        //
// Technical University of Kosice //
// 23.02.2025                     //
// Nikita Kuropatkin              //
// Version for MCU                //
// W5100S-EVB-Pico                //

/*
This header file declares macros for returning values
of error, also there is a function in config.c,
that ends program and returns error code value
(can be programmed to print error message)
*/
#ifndef ERROR_H
#define ERROR_H

//Defining errors with macros to use them in codes
#define OK 0
#define ERROR_SOCKET_CREATION 1
#define ERROR_GETTING_INPUT 2
#define ERROR_RECEIVING_DATA 3  
#define ERROR_SENDING_DATA 4
#define ERROR_GENERATING_RANDOM 5
#define ERROR_CLIENT_CONNECTION 6
#define ERROR_PORT_INPUT 7
#define ERROR_IP_INPUT 8
#define MESSAGE_ALTERED 9
#define UNEQUAL_MAC 10
#define WRONG_PIN 11
#define WRONG_PIN_FORMAT 12
#define ALLOCATION_ERROR 13
#define TEXT_OVERFLOW 14
#define WRONG_TXT_FORMAT 15
#define UNSUPPORTED_SIZE 16
#define WRONG_NETWORK_CONFIG 17
#define ANS_TEXT_OVERFLOW 18
#define ERROR_SEEDING_XDRBG 19

#define DHCP_ERROR 20
#define CONFLICT_DHCP 21
#define ERROR_FLASH 22
#define ERROR_MAC_INPUT 23

///////////////////////////////////////
/// Error Printing and System Reset ///
///////////////////////////////////////
/*
The purpose of this function is to handle errors by printing error details 
and resetting the system using the watchdog timer. This is the recommended 
software reset method for the W5100S-EVB-Pico board.

#About watchdog on Pico MCU:
- Link about resetting the board with a watchdog: 
  https://forums.raspberrypi.com/viewtopic.php?t=326097

#Parameters:
- `error`: An error code defined in `error.h` that describes the type of error.
- `err_string`: A string that contains details about the error.

#Behavior:
1. If the `DEBUG` macro is set to `YES`:
   - The function prints the error details to the terminal.
   - Prompts the user to press Enter before resetting.
2. If the `DEBUG` macro is set to `NO`:
   - The function skips printing error details and resets chip in 5 seconds.
3. The system is reseted using the watchdog timer with a 5-second delay.

#Notes:
- This function is specific to the W5100S-EVB-Pico board and uses the 
  watchdog timer for resets.
*/
void exit_with_error(const int error, const char *err_string);
///////////////////////////////////////
/////////////////////////////////////// 

#endif