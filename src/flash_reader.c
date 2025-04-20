// Client-server API(PICO)        //
// Flash Memory Reader            //
// Version 0.9.0pi                //
// Bachelor's Work Project        //
// Technical University of Kosice //
// 23.02.2025                     //
// Nikita Kuropatkin              //
// Version for MCU                //
// W5100S-EVB-Pico                //

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "include/error.h" //All errors defined + function proto
#include "include/parameters.h"

/*
A pointer to the memory address of the specified flash page,  
offset from `XIP_BASE` (the base address for flash memory access).
Address of key/salt values
Do not change this value!
*/
uint8_t *flash_target = (uint8_t *)(XIP_BASE + FLASH_PAGE);

////////////////////////
/// Flash Mem Reader ///
////////////////////////
/*
Copies data from flash memory into the provided `uint8_t` buffer.

Parameters:  
- `offset` - the starting position in flash memory for the data.  
  For example, the key is stored first, followed by the salt, 
  so the offset helps locate the data.  
- `buffer` - an array to store the read data (e.g., key, salt, pin).  
- `size` - the number of bytes to read.  
*/
void read_from_flash(const int offset, uint8_t *buffer, const int size) {
 for (int i = offset, j = 0; i < offset + size; i++, j++) {
  buffer[j] = flash_target[i];
 }
}
///////////////////////
///////////////////////
