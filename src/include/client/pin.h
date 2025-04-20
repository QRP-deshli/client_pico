// Client-server API(PICO)        //
// PIN functions                  //
// Version 0.8.7pi                //
// Bachelor's Work Project        //
// Technical University of Kosice //
// 10.02.2025                     //
// Nikita Kuropatkin              //
// Version for MCU                //
// W5100S-EVB-Pico                //

/* 
This header file declares functions for securing authentication 
key by PIN for a client-server application. Function bodies
are in pin.c. Also this header is in use by pin_changer.c program, 
that helps to generate keys secured by PIN 
*/
#ifndef PIN_H
#define PIN_H
#include <stdint.h>

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
static void xor_with_key(uint8_t *result_key, const uint8_t *working_key, const uint8_t *hashed_pin);
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
static void hashing_pin(uint8_t *pin, uint8_t *hashed_pin, uint8_t *salt);
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
void pin_checker(uint8_t *plain_key);
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

#endif