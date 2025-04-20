// Client-server API(PICO)        //
// Cryptographic functions        //
// Version 0.9.0pi                //
// Bachelor's Work Project        //
// Technical University of Kosice //
// 23.02.2025                     //
// Nikita Kuropatkin              //
// Version for MCU                //
// W5100S-EVB-Pico                //

/* 
This header file declares cryptographic functions
for a client-server application. Function definitions
are in crypto.c. 
*/
#ifndef CRYPTO_H
#define CRYPTO_H
#include <stdint.h>

/////////////////
///   PADME   ///
/////////////////
/*
This function derives the size of the padded array from the original
message size. It implements the PADME algorithm without rounding
using a bitmask (PADME: https://lbarman.ch/blog/padme/).
Parameters:
- `L`: The size of the original message (L16, L24, or L32).
Returns:
- The size of the padded message.
*/
int padme_size(const int L);

/*
Padding of array (copying to an array of larger size 
and the additional space is filled with random data).
The function copies the original array to the new padded array and fills 
the remaining space with random data.
Takes as input: 
- `array`: The original array to be padded.
- `pad_array`: The array where the padded data will be stored.
- `og_size`: The original size of the array.
- `new_size`: The size of the padded array.
*/
void pad_array(const uint8_t* array, uint8_t* pad_array, const int og_size, const int new_size);

/*
Reverse of pad_array function. This function copies the original 
(unpadded) data from the padded array back to the original array size.
Takes as input:
- `array`: The original array where the data will be copied.
- `pad_array`: The padded array from which data will be copied.
- `og_size`: The original size of the array.
*/
void unpad_array(uint8_t* array, const uint8_t* pad_array, const int og_size);

/*
This function derives a shared key using the Blake2b KDF 
(Key Derivation Function) from the raw shared key and the public keys (PKs) 
of both sides. Firstly it will generate your PK from your SK, than 
it will use it, PK of other side and shared_secret to derive strong key
Parameters:
- `shared_key`: A pointer to the buffer where the derived shared key will be 
  stored.
- `your_sk`: A pointer to your private key.
- `their_pk`: A pointer to the other party's public key.
- `keysz`: The size of the keys.
*/
void kdf(uint8_t *shared_key, const uint8_t *your_sk, const uint8_t *your_pk, const uint8_t *their_pk, const int keysz);
///////////////////////////////
///////////////////////////////

/*
This function generates hidden public keys (PKs) using the Elligator 2 
algorithm. It takes two empty arrays to store the generated keys and 
performs the following steps:
1. Generates a tweak for Elligator.
2. In an infinite loop, it generates a private key (SK) and derives 
the corresponding public key (PK).
3. It checks if the generated PK can be mapped to a random string using the 
Elligator 2 cycle.
4. If the PK can be mapped to a random string, the cycle ends. 
If not, the function continues generating new SKs and PKs until a valid one 
is found.
Parameters:
- `your_sk`: A pointer to the buffer where the generated private key (SK) 
  will be stored.
- `your_pk`: A pointer to the buffer where the derived public key (PK) 
  will be stored.
- `hidden`: A pointer to the buffer where the hidden public key 
  will be stored.
- `keysz`: The size of the keys (both SK and PK).
*/
void key_hidden(uint8_t *your_sk, uint8_t *your_pk, uint8_t *hidden, const int keysz);
/////////////////////////////////////////
/////////////////////////////////////////


#endif