// Client-server API(PICO)        //
// PIN functions                  //
// Version 0.9.0pi                //
// Bachelor's Work Project        //
// Technical University of Kosice //
// 23.02.2025                     //
// Nikita Kuropatkin              //
// Version for MCU                //
// W5100S-EVB-Pico                //

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/client/pin.h"
#include "../include/monocypher.h"
#include "../include/parameters.h" //Macros are defined here
#include "../include/error.h"
#include "../include/flash_reader.h"

/*
This function takes an input key and a hashed PIN, and performs an 
XOR operation on each corresponding element of both arrays. 
Each byte of the `working_key` is XORed with the corresponding byte of the 
`hashed_pin` to generate the `result_key`.
The function can be used to:
- Transform a plain key into a key secured by a PIN.
- Convert a key secured by a PIN back to its original plain form.
Parameters:
- `result_key`: A pointer to the buffer where the result of the 
   XOR operation will be stored.
- `working_key`: A pointer to the working key that will be XORed with 
  the hashed PIN.
- `hashed_pin`: A pointer to the hashed PIN that will be used for the 
  XOR operation.
*/
static void xor_with_key(uint8_t *result_key, const uint8_t *working_key, const uint8_t *hashed_pin)
{
 for (size_t i = 0; i < HASHSZ; i++) {
    result_key[i] = working_key[i] ^ hashed_pin[i];
 }
}
/////////////////////////////////////////////////
/////////////////////////////////////////////////

/*
This function takes an input PIN and hashes it using the Argon2i algorithm. 
The Argon2i parameters used in this function are predefined in the code and 
can be modified, but changes should be made with caution to ensure security 
and proper functioning.
The function uses Argon2i to hash the `pin` with the provided `salt` and 
stores the result in the `hashed_pin`.
Parameters:
- `pin`: A pointer to the input PIN that will be hashed.
- `hashed_pin`: A pointer to the buffer where the resulting hashed PIN will 
  be stored.
- `salt`: A pointer to the salt that will be used in the hashing process.
*/
static void hashing_pin(uint8_t *pin, uint8_t *hashed_pin, uint8_t *salt) {
 /*Configuring Argon2 for PIN hashing*/
 crypto_argon2_config config = {
    .algorithm = CRYPTO_ARGON2_I, /* Variant of Argon*/
    .nb_blocks = BLOCK_AMOUNT,    /* The number of blocks for work area*/
    .nb_passes = ITERATIONS,               /*iterations*/
    .nb_lanes  = LANSES                /* Single-threaded*/
 };
 crypto_argon2_inputs inputs = {
    .pass      = pin,                   /* User PIN*/
    .salt      = salt,                  /* Salt for the PIN*/
    .pass_size = PINSZ,                 /* PIN length*/
    .salt_size = SALTSZ                 /* salt length*/
 };       
 crypto_argon2_extras extras = {0};   /* Extra parameters unused */

 /*
  Working memory for Argon2i hashing.
  The allocation type is determined based on the ALLOCATION flag.
  Check parameters.h for more info about ALLOCATION macro
 */
 #if ALLOCATION == DYNAMIC
   void *work_area = ALLOCATE_WORK_AREA((size_t)BLOCK_AMOUNT * 1024);
   if (work_area == NULL) {
    crypto_wipe(pin, PINSZ);//wiping PIN, cause it`s not longer needed
    crypto_wipe(salt, SALTSZ); //wiping salt, cause it`s not longer needed
    exit_with_error(ALLOCATION_ERROR,"Memory allocation failed");
   }
 #elif ALLOCATION == STATIC_BSS
   /* Static memory allocation on BSS segment */
   static uint8_t work_area[BLOCK_AMOUNT * 1024] __attribute__((aligned(8)));
 #else
   /* Static memory allocation on STACK segment */
   uint8_t work_area[BLOCK_AMOUNT * 1024] __attribute__((aligned(8)));
 #endif
 crypto_argon2(hashed_pin, HASHSZ, work_area,config, inputs, extras);
 crypto_wipe(pin, PINSZ); //wiping PIN, cause it`s not longer needed
 crypto_wipe(salt, SALTSZ); //wiping salt, cause it`s not longer needed
 FREE_WORK_AREA(work_area); //free memory after usage of Argon
}
/////////////////////////////////////////////////
/////////////////////////////////////////////////

/*
This function prompts the user to enter their PIN. 
The entered PIN is then XORed with key material stored in `client/secret.h`.
If the PIN is correct, the client side will be authenticated. 
If the PIN is incorrect, the communication will end.
The function ensures that only users with the correct PIN can authenticate 
successfully, allowing further communication to proceed.
Parameters:
- `plain_key`: A pointer to the key material that will be XORed with 
the entered PIN. This key is used for authentication.
*/
void pin_checker(uint8_t *plain_key) {
 char pin[NONSZ]; // bigger size for checking
 uint8_t hashed_pin[HASHSZ]; //hashed value of pin
 /*Key secured by PIN that contained in src/client/key.txt*/
 uint8_t secured_key[KEYSZ];
 /*Salt for PIN hashing(contained in src/client/salt.txt)*/
 uint8_t salt[SALTSZ];

 printf("Enter PIN: ");
 fgets(pin, NONSZ, stdin); // getting pin

/*
 Checking if user entered digits for PIN(not other characters)
*/
 for (int i = 0; i < PINSZ; i++) {
    if (pin[i] < '0' || pin[i] > '9') {
       exit_with_error(WRONG_PIN_FORMAT,"PIN must be digits(0-9)!");
    }
 }

 int len = strlen(pin)-1; // length of PIN client entered

 // Buffer overflow or PIN longer that it suppose to be, abort
 if (pin[len] != '\n' || len != PINSZ) {
    exit_with_error(WRONG_PIN,"You entered wrong PIN");
 }

 read_from_flash(KEY_OFFSET, secured_key, KEYSZ); //Reading key from flash

 read_from_flash(SALT_OFFSET, salt, SALTSZ); //Reading salt from flash
 
 hashing_pin((uint8_t*)pin, hashed_pin, salt); // hashing with ARGON2
 
 /*XORing resulted hash with key-material*/
 xor_with_key(plain_key, secured_key, (uint8_t*)hashed_pin); 
}
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

