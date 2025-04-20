// Client-server API(PICO)        //
// Random number generator        //
// Version 0.9.0pi                //
// Bachelor's Work Project        //
// Technical University of Kosice //
// 23.02.2025                     //
// Nikita Kuropatkin              //
// Version for MCU                //
// W5100S-EVB-Pico                //

/* 
This header file declares CSPRNG(random number generator) function
for a client-server application. Function definition
are in random.c. 
*/
#ifndef RANDOM_H
#define RANDOM_H
#include <stdint.h>

/*
Function: random_num
Purpose: Generates random bytes using XDRBG and fills the provided array 
with them.
Input:
    - number: Pointer to the array that will be filled with random bytes.
    - size: The number of random bytes to generate and store in the `number` 
      array.
*/
void random_num(uint8_t *number,const int size);

/*
Function: random_init
Purpose: Initializes the random number generator by seeding the 
XDRBG with entropy.
This function generates entropy and seeds the XDRBG.
*/
void random_init(void);

/*
Function: random_entropy
Purpose: Fills the provided `entropy` array with random data.
Input:
    - entropy: Pointer to the array to be filled with random entropy data.
    - size: The number of random entropy bytes to generate.
*/
static void random_entropy(uint8_t *entropy, const int size);

#endif