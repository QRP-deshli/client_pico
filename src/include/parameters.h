// Client-server API(PICO)        //
// Parameters                     //
// Version 0.9.0pi                //
// Bachelor's Work Project        //
// Technical University of Kosice //
// 23.02.2025                     //
// Nikita Kuropatkin              //
// Version for MCU                //
// W5100S-EVB-Pico                //

/* 
This header file contains macros for the operation 
of the client program.
Read the comments next to each macro for an explanation of its purpose.
*/
#ifndef PARAMETERS_H
#define PARAMETERS_H
#include "hardware/flash.h"
#include "wizchip_conf.h"
#include "xdrbg.h"

/////////////////////
/* Can be modified */
/////////////////////

/*
Macros and variables that are in this block can be modified by the user. 
In some cases, conditions need to be met, while other macros 
have no specific rules for modification.
*/

/*
In use: client.c.
Defines the message size. You can modify this value as desired.
This size represents the maximum number of characters read from 
stdin to send. Ideally, it should be 10% smaller than the macro BUFF_MAX 
(minimal difference should be 5% for sizes greater than 100 characters).
*/
#define TEXT_MAX 400 

/*
In use: client.c, compress_decompress.c.
Defines the maximum size for buffers containing compressed/encrypted text. 
It is larger than TEXT_MAX since LZRW3-A can expand the input text, though 
this is highly unlikely. You can modify this value, but it should ideally 
be 10% larger than TEXT_MAX (minimal difference should be 5% for sizes 
greater than 100 characters).
*/
#define BUFF_MAX 500

/*
In use: client.c.
Defines the size of a uint8_t array containing a uint32_t number. 
You can modify this, but it is unnecessary since the message size 
is unlikely to exceed the 2^33-1 boundary.
*/
#define BYTE_ARRAY_SZ 4

/*
In use: client.c; affects pin.c, crypto.c.
These values define sizes for keys, nonces, and MACs. 
Modifying them could create security risks or instability.
If changes are necessary, review the code and consider using 
different Monocypher functions.
*/
#define KEYSZ 32   // SK, PK, Hidden PK sizes
#define NONSZ 24   // Nonce size
#define MACSZ 16   // MAC size

/*
In use: client.c.
Defines the default port number. If the user does not specify a 
port while running the program, this value is used. Can be modified, 
but it should remain within the range of 0-65535. Staying in the 
classic range (1024-65535) is recommended since ports below 1024 are 
used by well-known services like DHCP, DNS, or SSH.
*/
#define PORT 8087 

/*
In use: client.c.
Defines the range of ports that the user can choose from. You can 
modify these values, but they must meet the following conditions:
# PORT_START <= PORT_END
# 0 <= PORT_START <= 65535
# 0 <= PORT_END <= 65535
*/
#define PORT_START 1024 // First available port
#define PORT_END 65535  // Last available port

/*
In use: client.c.
Specifies the default server IP address (loopback). If the user does 
not provide an address while running the program, this address is used. 
Can be modified, but it must be a valid IP in dotted decimal format.
*/
#define IP {192, 168, 137, 1}

/*
In use: client.c
Specifies the socket number, the client will be using while communicating
with server(Raspberry pi PICO)
*/
#define SOCKET_NUM 0    /* Socket number */

/*
In use: addition.c.
Defines a stop-word that terminates a conversation. You can modify 
this value. If the stop-word differs on one side, the other side will 
interpret it as an error.
*/
#define EXIT "exit"

/*
In use: compress_decompress.c, pin.c.
Defines the memory allocation type for PIN hashing and text compression. 
Options: STATIC_BSS, STATIC_STACK, DYNAMIC. Adjusting the BLOCK_AMOUNT 
macro is necessary when using STATIC_STACK due to stack size limits. 
*/
#define ALLOCATION DYNAMIC  

/*
In use: config.c.
Enables or disables error printing. Set to NO to disable error 
printing, or YES to enable it.
*/
#define DEBUG YES 

/*
In use: pin.c.
Defines the number of memory blocks for Argon2i. The default value for Pico is 
10 KB with 200 iterations. Adjusting this requires considering system RAM 
and Argon2 input parameters.
*/
#define BLOCK_AMOUNT 10

/*
In use: pin.c.
Defines the number of iterations for Argon2i. The default is 200 iterations 
with 10 KB of memory. Adjusting this requires considering system RAM 
and Argon2 input parameters.
*/
#define ITERATIONS 200

/*
In use: pin.c.
Defines the PIN size. Changing this requires reapplying a new PIN to 
the long-term shared key. Update the PIN in the corresponding .txt file.
*/
#define PINSZ 6 

/*
In use: pin.c.
Defines the salt size for PIN hashing. Changing this requires reapplying 
the PIN to the long-term shared key and updating the salt in the .txt file.
*/
#define SALTSZ 16 // Size of the salt

/*
In use by: timing.c.  
Macro defining the system clock frequency in kHz.  
Sets the system's operating frequency.  
This value can be modified, 
but not all values will ensure optimal program performance.  
*/
#define PLL_SYS_KHZ (125 * 1000)

/*
Used in: random.c  
This macro sets the size of the entropy data array used to seed XDRBG (in bytes).   
The optimal value for our implementation is 64 bytes. If you want to change it,  
it is strongly advised to stay below 110 bytes due to our custom optimization  
of XDRBG. The array containing entropy is 136 bytes in size, but as I counted,  
4 additional bytes are included. Theoretically, you could use up to 132 bytes,  
but it is not recommended.
*/
#define SEED_SIZE 64

/*
Used in: client.c  
This macro sets the communication type between the host computer 
and the encryptor.
Changing this macro will switch the communication type. 
Do not forget to also update the CMakeLists.txt file with the following lines:
    pico_enable_stdio_usb(${TARGET_NAME} 1)
    pico_enable_stdio_uart(${TARGET_NAME} 0)

Set the macro to USB for USB communication or to UART for UART communication.
*/
#define COMMUNICATION USB

/*
In use: network_data.c.  
Macro that defines how many times the module will try to retrieve an address  
from the DHCP server. If the limit is exceeded, execution will stop.  
You can modify this value, but only consider an absolute value > 0.
*/
#define DHCP_RETRY_COUNT 5

/*
In use: network_data.c.  
Macro defining the socket for DHCP.  
You can change this value, but ensure it does not  
conflict with a value from SOCKET_NUM.
*/
#define SOCKET_DHCP 3

/*
In use: client.c.
This macro defines how many times the firmware will execute  
without a full system reboot (saving some time).  
Due to the design of some functions, 
you should not set this value greater than 6.  
The value must also be greater than 0.  
*/
#define LIVE_COUNT 6

/*
In use: network_data.c.  
Calculates the start of the second-to-last flash memory sector.  
Networking data values are stored in the second-to-last sector,  
so this macro provides access to them.  
You can change this macro, but be sure to correctly count flash addresses  
to access the right values from it.
(And do not choose last page(key & salt stored there)).
*/
#define FLASH_PAGE_NET (PICO_FLASH_SIZE_BYTES - 2 * FLASH_SECTOR_SIZE)

/*
In use: network_data.c.  
Calculates the start of the third-to-last flash memory sector.  
Server\Port data values are stored in the third-to-last sector,  
so this macro provides access to them.  
You can change this macro, but be sure to correctly count flash addresses  
to access the right values from it.
(And do not choose last page(key & salt stored there)).
*/
#define FLASH_PAGE_SERPORT (PICO_FLASH_SIZE_BYTES - 3 * FLASH_SECTOR_SIZE)


/////////////////// 
/* Do not modify */ 
///////////////////

/*
Macros and variables that are in this block either cannot 
be modified by the user, or there is no need to modify them, 
like macros that define options for other macros.
*/


/*
Defined in network_data.c.
A pointer to the memory address of the specified flash page,  
offset from `XIP_BASE` (the base address for flash memory access).  
This pointer references the memory where networking data  
(IP, gateway, etc.) from the last use is stored.  
Do not change this value!
*/
extern uint8_t *flash_target_net;

/*
In use: network_data.c
Macro defining offset of stored IP address in Flash memory
Do not change this value!
*/
#define IP_OFFSET 0

/*
In use: network_data.c
Macro defining offset of stored Gateway address in Flash memory
Do not change this value!
*/
#define GW_OFFSET 4

/*
In use: network_data.c
Macro defining offset of stored Subnet Mask in Flash memory
Do not change this value!
*/
#define SN_OFFSET 8

/*
In use: network_data.c
Macro defining offset of stored DNS address in Flash memory
Do not change this value!
*/
#define DNS_OFFSET 12

/*
In use: network_data.c
Macro defining offset of stored MAC address in Flash memory
Do not change this value!
*/
#define MAC_OFFSET 16

/*
In use: network_data.c
Macro defining sizes of IP, default gateway, subnet mask, DNS, 
that are stored in struct wiz_NetInfo.
Do not change this value!
*/
#define NET_DATA_SIZE 4

/*
In use: network_data.c
Macro defining the size of MAC address stored in struct wiz_NetInfo.
Do not change this value!
*/
#define MAC_SIZE 6

/*
In use: network_data.c.  
Macro and global variable for DHCP initialization.  
Do not change these values!
*/
#define ETHERNET_BUF_MAX_SIZE (1024 * 2)

/*
Defined in flash_reader.c.
*/
extern uint8_t g_ethernet_buf[ETHERNET_BUF_MAX_SIZE];

/*
In use: network_data.c
Macro defining offset of stored IP address of server in Flash memory
Do not change this value!
*/
#define SERVER_IP_OFFSET 0

/*
In use: network_data.c
Macro defining offset of stored port number in Flash memory
Do not change this value!
*/
#define PORT_OFFSET 4

/*
In use: addittion.c, network_data.c
Size of IP address.
Never change the value of this macro!
*/
#define IPSZ 16  

/*
In use: network_data.c. 
Size of MAC address.
Never change the value of this macro!
*/
#define MAC_ADDRESS_SIZE 17 

/*
In use: client.c
Macros defining the available communication options: USB or UART.
Do not change these values!
*/
#define UART 0
#define USB 1

/*
In use: addition.c
The next two macros define the allowed range 
of values for octets in IPv4 (0.0.0.0 - 255.255.255.255).
Never change the values of these macros!
*/ 
#define IPSTART 0
#define IPEND 255

/*
In use: here (upper segment -> macro ALLOCATION), also compress_decompress.c,
pin.c
Macros define types of memory allocation the user can use 
for PIN hashing and compressing and decompressing text.
You can change the values of the options, but I don't think you'll need to,
which is why they are here. (I do not recommend changing them.)
*/
#define STATIC_BSS 0    // Memory will be allocated statically in the BSS 
#define STATIC_STACK 1  // Memory will be allocated statically on the stack
#define DYNAMIC 2       // Memory will be allocated dynamically on the heap

/*
In use: here (upper segment -> macro DEBUG), config.c, client.c, addition.c
Two macros define DEBUG macro options. 
Also two macros define whether a certain side has ended communication
Again, you don’t need to change these, but you can. 
(I do not recommend changing them.)
*/
#define YES 1
#define NO 0

/*
This macro defines an error that can be returned by any function in this code.  
It is used almost everywhere. If you see this macro, it means the function  
execution ended with an error.
*/
#define RETURN_ERROR -1

/*
In use: pin.c, compress_decompress.c
Macro function to dynamically allocate memory for Argon2i and LZRW3-A.
Never change this value!
*/
#define ALLOCATE_WORK_AREA(size) malloc(size) 

/*
In use: pin.c, compress_decompress.c
Macro function to de-allocate work_area memory.
If the ALLOCATION macro is set to STATIC_BSS or STATIC_STACK, it does nothing.
If the ALLOCATION macro is set to DYNAMIC, it frees the allocated memory.
Never change the values of these macros!
*/
#if ALLOCATION == DYNAMIC
  #define FREE_WORK_AREA(ptr) free(ptr)
#else
  #define FREE_WORK_AREA(ptr)  // No action for STATIC allocation
#endif

/*
In use: pin.c
The produced value of the hash of a PIN should be equal to the key size, 
which is why the macro below is mapped to KEYSZ.
Do not change this value!
*/
#define HASHSZ KEYSZ 

/*
In use: pin.c.
Defines the number of threads (parallelism) for Argon2i.  
Since Monocypher uses a single-threaded approach, 
this value does not need to be changed.
*/
#define LANSES 1

/*
In use: pin.c.
Specifies the offset (starting location) of the secured key in flash memory.  
The key is always stored at the beginning of the flash page.  
Do not modify this value unless you have modified the `flash_key_salt` program 
to save it to a different location.
*/
#define KEY_OFFSET 0

/*
In use: pin.c.
Specifies the offset (starting location) of the salt in flash memory.  
The salt is stored after the key.  
Do not modify this value unless the `flash_key_salt` program has been modified 
to store it elsewhere.
*/
#define SALT_OFFSET KEYSZ

/*
In use: flash_reader.c.
Calculates the start of the last flash memory sector.  
The formula subtracts the size of two sectors from the total flash size.  
Key and salt are stored in the last sector, 
so this macro allows access to them.
Do not change this value!
*/  
#define FLASH_PAGE (PICO_FLASH_SIZE_BYTES - 1 * FLASH_SECTOR_SIZE)

/*
Defined in flash_reader.c.
A pointer to the memory address of the specified flash page,  
offset from `XIP_BASE` (the base address for flash memory access).
Address of key/salt values
Do not change this value!
*/
extern uint8_t *flash_target;

/*
Defined in flash_reader.c.
A pointer to the memory address of the specified flash page,  
offset from `XIP_BASE` (the base address for flash memory access).
Address of server/port configurations
Do not change this value!
*/
extern uint8_t *flash_target_serport;

/*
In use: addition.c
Defines the size of the buffer used to store the user's answer 
(either 'y' or 'n'). The buffer size is larger to also handle answers 
like "yes" or "no" or deletions for usb.
Do not change this macro value!
*/
#define ANS_SIZE 30

//////////////////////// 
////////////////////////

#endif
