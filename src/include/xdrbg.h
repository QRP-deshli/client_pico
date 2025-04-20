// Client-server API (PICO)       //
// DRBG Algorithm                 //
// Version 0.9.0pi                //
// Bachelor's Work Project        //
// Technical University of Kosice //
// 23.02.2025                     //
// Nikita Kuropatkin              //
// Version for MCU                //
// W5100S-EVB-Pico                //

/* 
This header file declares macros, structures, and function prototypes 
needed to use XDRBG, as the original project only contained a .c file. 
I needed to create this header file.
*/

#ifndef XDBRG_H
#define XDBRG_H

#define LC_XDRBG256_DRNG_KEYSIZE 64

// Structure to hold the state of the XDRBG
// initially_seeded - a flag indicating if the DRBG has been seeded
// v - the internal state of the DRBG, with a size of LC_XDRBG256_DRNG_KEYSIZE
struct lc_xdrbg256_drng_state {
    uint8_t initially_seeded; // Flag to indicate if the state has been seeded
    uint8_t v[LC_XDRBG256_DRNG_KEYSIZE]; // Internal state of the DRBG
};

/*
Function: lc_xdrbg256_drng_seed
Purpose: Seeds the XDRBG with entropy from the system. The seed size should not 
         exceed 110 bytes. The `seed` parameter must be smaller than this value 
         due to modifications in xdrbg.c. Check the `SEED_SIZE` macro in 
         parameters.h for more details.
Input: 
    - state: Pointer to the lc_xdrbg256_drng_state structure, where the 
	  working state was stored.
    - seed: Pointer to the entropy data used for seeding.
    - seedlen: Length of the seed data in bytes.
Output: 
    - Returns an integer: 0 on success, or a non-zero value on failure(EINVAL).
*/
int lc_xdrbg256_drng_seed(struct lc_xdrbg256_drng_state *state,
                        uint8_t *seed,
                          size_t seedlen
                          );

/*
Function: lc_xdrbg256_drng_generate
Purpose: Generates random bits and fills the provided output buffer.
Input:
    - state: Pointer to the lc_xdrbg256_drng_state structure representing the 
	  current state of the DRBG.
    - out: Pointer to the output buffer where the random bits will be stored.
    - outlen: The size of the output buffer in bytes.
Output:
    - Returns an integer: 0 on success, or a non-zero value on failure(EINVAL).
*/
int lc_xdrbg256_drng_generate(struct lc_xdrbg256_drng_state *state,
                               uint8_t *out, size_t outlen
                             );

#endif
