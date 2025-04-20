// Client-server API (PICO)       //
// Random number generator        //
// Version 0.9.0pi                //
// Bachelor's Work Project        //
// Technical University of Kosice //
// 23.02.2025                     //
// Nikita Kuropatkin              //
// Version for MCU                //
// W5100S-EVB-Pico                //

#include <stdint.h>
#include "include/error.h" // All errors are defined + function prototypes
#include "include/random.h"
#include "random_entropy.h"
#include "include/xdrbg.h"
#include "include/parameters.h"
#include "include/monocypher.h"

//////////////////////////////////////////
/// Random Numbers Generator        ///
//////////////////////////////////////////

/*
Working state for xdrbg random bit generator
Do not change this value!
*/
static struct lc_xdrbg256_drng_state xdrbg256_ctx = { 0 };

/*
Function: random_num
Purpose: Generates random bytes using XDRBG and fills the provided array 
with them.
Input:
    - number: Pointer to the array that will be filled with random bytes.
    - size: The number of random bytes to generate and store in the `number` 
      array.
*/
void random_num(uint8_t *number,const int size) {
 if (lc_xdrbg256_drng_generate(&xdrbg256_ctx, number, size) != OK) {
   exit_with_error(ERROR_GENERATING_RANDOM, "Error generating random bits");
 }
}

/*
Function: random_init
Purpose: Initializes the random number generator by seeding the 
XDRBG with entropy.
This function generates entropy and seeds the XDRBG.
*/
void random_init(void) {
 // Entropy that we will use for XDRBG seeding
 uint8_t entropy[SEED_SIZE];
 random_entropy(entropy, SEED_SIZE);

 if (lc_xdrbg256_drng_seed(&xdrbg256_ctx, entropy, sizeof(entropy)) != OK) 
 {
   exit_with_error(ERROR_SEEDING_XDRBG, "Error seeding XDRBG");
 }

 crypto_wipe(entropy,SEED_SIZE); //Wiping entropy after seeding the XDRBG
}

/*
Function: random_entropy
Purpose: Fills the provided `entropy` array with random data.
Uses default pico_rand function to collect entropy from 3 different sourses
Input:
    - entropy: Pointer to the array to be filled with random entropy data.
    - size: The number of random entropy bytes to generate.
*/
static void random_entropy(uint8_t *entropy, const int size) {
 uint64_t temp_entropy = 0;
 for (int i = 0; i < size; i++) {
    if(i%8 == 0)temp_entropy = get_rand_64();
    entropy[i] = (uint8_t) (temp_entropy & 0xFF); 
    temp_entropy >>= 8;
 }
}
