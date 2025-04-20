// Client-server API(PICO)        //
// Compression functions          //
// Version 0.9.0pi                //
// Bachelor's Work Project        //
// Technical University of Kosice //
// 23.02.2025                     //
// Nikita Kuropatkin              //
// Version for MCU                //
// W5100S-EVB-Pico                //

/* 
This header file declares functions needed for 
compression/decompression using LZRW3a algorithm of users text for a 
client-server application. Function bodies
are in compress_decompress.c. 
LZRW3a: http://www.ross.net/compression/lzrw3a.html
*/
#ifndef COMPRESS_DECOMPRESS_H
#define COMPRESS_DECOMPRESS_H
#include <stdint.h>

/////////////////////////////////////////////
/// uint32_t to uint8_t Array Converter   ///
/////////////////////////////////////////////
/*
The function converts the `uint32_t` number to a byte array and writes 
it in Big Endian format.  
This function is needed to send the size of the compressed text to the 
other side.
This function takes the following parameters:  
- `number` - a `uint32_t` number that will be converted.  
- `byte_array` - a pointer to a `uint8_t` array (byte array) where the 
  converted value will be stored.  
*/
void to_byte_array(const uint32_t number, uint8_t* byte_array);
/////////////////////////////////////////////
/////////////////////////////////////////////

/////////////////////////////////////////////
/// uint8_t Array to uint32_t Converter   ///
/////////////////////////////////////////////
/*
This function takes the following parameters:  
The function converts the Big Endian byte array to a `uint32_t` number.  
This function is needed to send the size of the compressed text 
to another side.
- `number` - a `uint32_t` variable where the converted value will be stored.  
- `byte_array` - a pointer to a `uint8_t` array (byte array) 
  that is in Big Endian format.  
*/
uint32_t from_byte_array( const uint8_t* byte_array, uint32_t number);
/////////////////////////////////////////////
/////////////////////////////////////////////

/////////////////////////
/// Text Compressors  ///
/////////////////////////
/*
The function compresses `input_txt` using the LZRW3a algorithm,  
and the compressed text is written to `output_txt`. 
This function takes the following parameters:  
- `input_txt` - the uncompressed text to be compressed.  
- `max_size` - the maximum size that the compressed text can have.  
- `output_txt` - a pointer to the buffer where the compressed text 
  will be stored.  
- `output_size` - a pointer to the size of the compressed text.  

If the compressed text exceeds the size of `output_txt`, the program 
will exit. This is highly unlikely, as the buffer is 100 characters larger 
than `input_txt`. According to the LZRW3a documentation, the compressed text 
should not be drastically larger than the input text. Based on experiments, 
text with extremely high entropy (45 unique characters) expanded by less 
than 10%, with the output text being 49 characters long.  
For more information on LZRW3a, see:  
LZRW3-A: http://www.ross.net/compression/lzrw3a.html
*/
void compress_text(unsigned char *input_txt, const uint32_t max_size, unsigned  char *output_txt, uint32_t *output_size);
/////////////////////////
/////////////////////////

///////////////////////////
/// Text Decompressors  ///
///////////////////////////
/*
The function decompresses `input_txt` using the LZRW3a algorithm,  
and the decompressed text is written to `output_txt`. 
This function takes the following parameters:  
- `input_txt` - the compressed text to be decompressed.  
- `max_size` - the maximum size that the decompressed text can have.  
- `input_size` - the size of the compressed text.  
- `output_txt` - a pointer to the buffer where the decompressed 
  text will be stored.  
 
For more information on LZRW3a, see:  
LZRW3-A: http://www.ross.net/compression/lzrw3a.html
*/
void decompress_text(unsigned char *input_txt, const uint32_t max_size, unsigned char *output_txt, const uint32_t input_size);
///////////////////////////
///////////////////////////

#endif