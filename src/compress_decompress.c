// Client-server API(PICO)        //
// Compression functions          //
// Version 0.9.0pi                //
// Bachelor's Work Project        //
// Technical University of Kosice //
// 23.02.2025                     //
// Nikita Kuropatkin              //
// Version for MCU                //
// W5100S-EVB-Pico                //

#include <string.h>
#include <stdlib.h>
#include "include/lzrw.h"
#include "include/error.h"
#include "include/parameters.h" //Macros are defined here

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
void to_byte_array(const uint32_t number, uint8_t* byte_array) {
 byte_array[0] = (number >> 24) & 0xFF; 
 byte_array[1] = (number >> 16) & 0xFF;
 byte_array[2] = (number >> 8) & 0xFF;
 byte_array[3] = number & 0xFF; 
}
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
uint32_t from_byte_array( const uint8_t* byte_array, uint32_t number) {
 number |= (uint32_t)byte_array[0] << 24;
 number |= (uint32_t)byte_array[1] << 16;
 number |= (uint32_t)byte_array[2] << 8;
 number |= (uint32_t)byte_array[3];
 return number;
}
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
void compress_text(unsigned char *input_txt, const uint32_t max_size, unsigned  char *output_txt, uint32_t *output_size)
{
 /*
  Working memory for LZRW3a compression.
  The allocation type is determined based on the ALLOCATION flag.
 */
 #if ALLOCATION == DYNAMIC // Check parameters.h for more info about macros 
   /* Dynamic memory allocation */
   void *wrk_mem = ALLOCATE_WORK_AREA(lzrw3a_req_mem()); 
   if (wrk_mem == NULL) {
       exit_with_error(ALLOCATION_ERROR, "Memory allocation failed");
   }
 #elif ALLOCATION == STATIC_BSS
   static uint8_t wrk_mem[MEM_REQ]; // Static memory allocation on BSS segment
 #else
     uint8_t wrk_mem[MEM_REQ]; // Static memory allocation on STACK segment
 #endif

 /*LZRW3a compress*/ 
 lzrw3a_compress(COMPRESS_ACTION_COMPRESS,wrk_mem,input_txt, strlen((const char *)input_txt),output_txt,output_size); 

 FREE_WORK_AREA(wrk_mem);

 if (*output_size>max_size) {
	exit_with_error(TEXT_OVERFLOW, "Compressed text size is bigger than buffer");
 }
}
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
void decompress_text(unsigned char *input_txt, const uint32_t max_size, unsigned char *output_txt, const uint32_t input_size)
{
 /*Output size*/
 uint32_t output_size = 0; 
 /*
  Working memory for LZRW3a compression.
  The allocation type is determined based on the ALLOCATION flag.
 */
 #if ALLOCATION == DYNAMIC // Check parameters.h for more info about macros 
   /* Dynamic memory allocation */
   void *wrk_mem = ALLOCATE_WORK_AREA(lzrw3a_req_mem()); 
   if (wrk_mem == NULL) {
       exit_with_error(ALLOCATION_ERROR, "Memory allocation failed");
   }
 #elif ALLOCATION == STATIC_BSS
   static uint8_t wrk_mem[MEM_REQ]; // Static memory allocation on BSS segment
 #else
     uint8_t wrk_mem[MEM_REQ]; // Static memory allocation on STACK segment
 #endif

 /*LZRW3a decompress*/
 lzrw3a_compress(COMPRESS_ACTION_DECOMPRESS,wrk_mem,input_txt,input_size,output_txt,&output_size); 
 FREE_WORK_AREA(wrk_mem);

 if (output_size>max_size) {
   exit_with_error(TEXT_OVERFLOW, "Compressed text size is bigger than buffer");
 }
}
///////////////////////////
///////////////////////////