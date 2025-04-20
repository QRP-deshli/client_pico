// Client-server API(PICO)        //
// Error handling                 //
// Version 0.9.0pi                //
// Bachelor's Work Project        //
// Technical University of Kosice //
// 23.02.2025                     //
// Nikita Kuropatkin              //
// Version for MCU                //
// W5100S-EVB-Pico                //

#include <stdio.h>
#include <stdlib.h>
#include "include/error.h"
#include "hardware/watchdog.h"
#include "include/parameters.h" // Macros are defined here

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
void exit_with_error(const int error, const char *err_string) {
 if (DEBUG == YES) {
   if (error != OK) {
      printf("\n!Error occurred!\n");
   }
   // Print error message
   printf("%s.\n", err_string);

   // Prompt user before reset
   printf("Press Enter to reset chip in 5 seconds:\n");

   // Wait for user to press Enter
   getchar();
 }

 // Trigger system reset with a 5-second delay using watchdog
 watchdog_reboot(0, 0, 5000);

 // Infinite loop to wait for the watchdog timer to reset the system
 while (1);
}
