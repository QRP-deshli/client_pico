// Client-server API(PICO)        //
// Cryptographic functions        //
// Version 0.9.0pi                //
// Bachelor's Work Project        //
// Technical University of Kosice //
// 23.02.2025                     //
// Nikita Kuropatkin              //
// Version for MCU                //
// W5100S-EVB-Pico                //

#include <string.h>
#include <stdio.h>
#include <math.h>
#include "include/crypto.h" //Crypto primitievs
#include "include/monocypher.h"
#include "include/random.h" //CSPRNG
#include "include/error.h"
#include "include/parameters.h" //Macros are defined here

/*
Macros for PADME:
- L16, L24, L32: Sizes of messages (compared to input size).
- S16, S24, S32: Computation of the S element for the PADME 
  algorithm for different sizes.
- E16, E24, E32: Computation of the E element for the PADME 
  algorithm for different sizes.
These macros ensure that the log2 calculations are executed at compile time, 
making the code more lightweight for embedded platforms.
*/
#define L16 16
#define E16 (int)log2(L16)
#define S16 (int)log2(E16) + 1

#define L24 24
#define E24 (int)log2(L24)
#define S24 (int)log2(E24) + 1

#define L32 32
#define E32 (int)log2(L32)
#define S32 (int)log2(E32) + 1

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
int padme_size(const int L) {
 //modified PADME for padding of the key + nonce
 int E;
 int S;
 if (L == L16) {
   E = E16;
   S = S16;
 }
 else if (L == L24) {
   E = E24;
   S = S24;
 }
 else if (L == L32) {
   E = E32;
   S = S32;
 }
 else exit_with_error(UNSUPPORTED_SIZE, "Unsupported size for padding"); 
 int lastBits = E - S;
 int bitMask = (1 << lastBits) - 1;
 return (L + bitMask);
}

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
void pad_array(const uint8_t* array, uint8_t* pad_array, const int og_size, const int new_size) 
{
    memcpy(pad_array, array, og_size);
    if (new_size > og_size) {
        random_num(&pad_array[og_size], new_size - og_size);
    }
}

/*
Reverse of pad_array function. This function copies the original 
(unpadded) data from the padded array back to the original array size.
Takes as input:
- `array`: The original array where the data will be copied.
- `pad_array`: The padded array from which data will be copied.
- `og_size`: The original size of the array.
*/
void unpad_array(uint8_t* array, const uint8_t* pad_array, const int og_size) 
{
    memcpy(array, pad_array, og_size);
}

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
void kdf(uint8_t *shared_key, const uint8_t *your_sk, const uint8_t *your_pk, const uint8_t *their_pk, const int keysz)
{
 uint8_t shared_secret[keysz]; // Raw shared key
 
 // Compute shared secret
 crypto_x25519(shared_secret, your_sk, their_pk);

 // KDF with Blake2(only clients algorithm)
 crypto_blake2b_ctx ctx;
 crypto_blake2b_init(&ctx, keysz);
 crypto_blake2b_update(&ctx, shared_secret, keysz);
 crypto_blake2b_update(&ctx, their_pk, keysz);
 crypto_blake2b_update(&ctx, your_pk, keysz);
 crypto_blake2b_final(&ctx, shared_key); // Shared key for encryption
        
 // Cleaning raw shared secret
 crypto_wipe(shared_secret, keysz);
}
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
void key_hidden(uint8_t *your_sk, uint8_t *your_pk, uint8_t *hidden, const int keysz) {
 uint8_t tweak; // Tweak for elligator`s inverse map
 random_num(&tweak, 1); // Tweak generation

 /*
  Cycle for creating SK and computing PK, 
  then inverse mapping it to a scalar
 */
 while (1) {
 random_num(your_sk, keysz);
 crypto_x25519_dirty_fast(your_pk, your_sk);
 if (crypto_elligator_rev(hidden, your_pk, tweak) == OK)
    break;
 }
}
/////////////////////////////////////////
/////////////////////////////////////////