// Client-server API(PICO)        //
// Flash Memory Reader            //
// Version 0.9.0pi                //
// Bachelor's Work Project        //
// Technical University of Kosice //
// 23.02.2025                     //
// Nikita Kuropatkin              //
// Version for MCU                //
// W5100S-EVB-Pico                //

/* 
This header file declares a function for reading data 
from flash memory in a client-server application. 
The function's implementation is in flash_reader.c. 
*/
#ifndef FLASH_READER_H
#define FLASH_READER_H
#include <stdint.h>

////////////////////////
/// Flash Mem Reader ///
////////////////////////
/*
This function reads important data(`uint8_t` arrays) 
from flash memory for use in the client-server application.  
Parameters:  
- `offset` - the offset within the flash memory page where the desired 
  data (e.g., key, salt, and network configurations) is stored.  
- `buffer` - an array where the read data (e.g., key, salt, pin arrays) 
  will be stored.  
- `size` - the size of the data to read (number of bytes).    
*/
void read_from_flash(const int offset, uint8_t *buffer, const int size);
///////////////////////
///////////////////////

#endif
