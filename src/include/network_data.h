// Client-server API(PICO)        //
// Netdata functions              //
// Version 0.9.0pi                //
// Bachelor's Work Project        //
// Technical University of Kosice //
// 23.02.2025                     //
// Nikita Kuropatkin              //
// Version for MCU                //
// W5100S-EVB-Pico                //

/* 
This header file declares prototypes of functions to configure 
networking information of the module, such as DHCP, 
Manual, and LAST used settings. 
Function bodies are in network_data.c.
*/
#ifndef NETWORK_DATA_H
#define NETWORK_DATA_H

// Enum to select the type of network information (netinfo)
typedef enum {
  NETDATA_LAST,     // Last used network settings
  NETDATA_DHCP,     // Dynamic Host Configuration Protocol (DHCP)
  NETDATA_MANUAL,    // Manual network settings
  NETDATA_DEFAULT    // Default network settings
} NetInfoType;

// Enum to select the type of server/port information
typedef enum {
  SERPORT_LAST,     // Last used server/port settings
  SERPORT_MANUAL,    // Manual server/port settings
  SERPORT_DEFAULT    // Default server/port settings
} ServerPortType;

/////////////////////////////
/// Network Configuration ///
/////////////////////////////
/*
Prompts the user to select a network configuration type.
1 - Last used settings
2 - DHCP
3 - Manual configuration
Returns the selected NetInfoType.
*/
static NetInfoType get_network_config_choice(void);
///////////////////////////
///////////////////////////

///////////////////////////
/// Apply Configuration ///
///////////////////////////
/*
Determines network settings based on user choice.
Calls configuration function and stores settings in Flash if needed.
Returns updated network info.
*/
wiz_NetInfo choose_net_data(void);
///////////////////////////
///////////////////////////

///////////////////////////
/// Read Network Input ///
///////////////////////////
/*
Reads user input for network parameters (IP, Gateway, etc.).
Ensures input is correctly formatted.

Takes as input parameter:
  - buffer: The pointer to the buffer where the user input will be stored.
*/
static void get_net_data(char *buffer);
///////////////////////////
///////////////////////////

/////////////////////////////
/// Convert IPv4 Address ///
/////////////////////////////
/*
Processes an IPv4 address string and converts it to a numeric format.
Each octet is extracted and stored in the output array.

Takes as input parameters:
  - input: The string containing the IPv4 address.
  - output: The array where the converted numeric values will be stored.

Outputs:
  - The function writes the IP address into the output array in numeric form.
*/
static void char_converter(char *input, uint8_t *output);
///////////////////////////
///////////////////////////

/////////////////////////////
/// Convert MAC Address ///
/////////////////////////////
/*
Processes a MAC address string and converts it to a numeric format.
Each hex pair is extracted and stored in the output array.

Takes as input parameters:
  - input: The string containing the MAC address.
  - output: The array where the converted numeric values will be stored.

Outputs:
  - The function writes the MAC address into the output array in numeric form.
*/
static void mac_converter(char *input, uint8_t *output);
///////////////////////////
///////////////////////////

/////////////////////////////
/// Validate MAC Address ///
/////////////////////////////
/*
Checks if the given MAC address is correctly formatted.
- Ensures it contains exactly 5 colons.
- Verifies that each part is within the valid range.

Takes as input parameter:
  - mac: The string containing the MAC address to be validated.
*/
static void mac_check(char *mac);
///////////////////////////
///////////////////////////

/////////////////////////////
/// Manual Network Setup  ///
/////////////////////////////
/*
Prompts the user to manually enter network settings, 
validates them, and converts them to numeric format.
*/
static void manual_net(void);
///////////////////////////
///////////////////////////

/////////////////////////////
/// Load Last Used Config ///
/////////////////////////////
/*
Loads previously stored network configuration from flash memory.
*/
static void last_used(void);
///////////////////////////
///////////////////////////

////////////////////////////
/// DHCP Configuration   ///
////////////////////////////
/*
Attempts to obtain an IP address dynamically using DHCP.
Retries a limited number of times before failing.
*/
static void dhcp_usage(void);
///////////////////////////
///////////////////////////

//////////////////////////////
/// DHCP Callback - Assign ///
//////////////////////////////
/*
Retrieves the assigned network parameters from DHCP 
and updates the global network info structure.
*/
static void wizchip_dhcp_assign(void);
///////////////////////////
///////////////////////////

////////////////////////////////
/// DHCP Callback - Conflict ///
////////////////////////////////
/*
Handles IP conflicts detected by DHCP and exits with an error.
*/
static void wizchip_dhcp_conflict(void);
///////////////////////////
///////////////////////////

///////////////////////
/// Initialize DHCP ///
///////////////////////
/*
Initializes the DHCP client and registers necessary callback functions.
*/
static void wizchip_dhcp_init(void);
///////////////////////////
///////////////////////////

///////////////////////////////
///// Network Configuration ///
///////////////////////////////
/*
Configures the network based on the selected network configuration type.
It handles three types of network setups: Last used, DHCP, and Manual.

Takes as input parameter:
  - netinfo_type: The network configuration type (Last used, DHCP, or Manual).
*/
static void configure_network(const NetInfoType netinfo_type);
////////////////////////////////
////////////////////////////////

/////////////////////////////////
///// Read Network from Flash ///
/////////////////////////////////
/*
Reads network data (IP, Gateway, Subnet Mask, DNS, MAC) from flash memory 
and stores it in the provided buffer.

Takes as input parameters:
  - offset: The starting position in the flash memory to read from.
  - buffer: The pointer to the buffer where the data will be stored.
  - size: The size of the data to read from flash.

Outputs:
  - The buffer is filled with the data from flash memory.
*/
static void read_net_flash(const int offset, uint8_t *buffer, const int size);
////////////////////////////////
////////////////////////////////

//////////////////////////////
///// Flash Erase Callback ///
//////////////////////////////
/*
This function is executed to erase a specified range in flash memory when 
it is safe to do so.

Takes as input parameter:
  - param: The offset address of the flash sector to be erased.
*/
static void call_flash_range_erase(void *param);
////////////////////////////////
////////////////////////////////

///////////////////////////////
///// Flash Program Callback ///
////////////////////////////////
/*
This function is executed to write data to flash memory when it's safe to 
do so. It takes the offset and data to be written.

Takes as input parameters:
  - params: A pointer to an array containing the offset and data to be written.
*/
static void call_flash_range_program(void *params);
////////////////////////////////
////////////////////////////////

////////////////////////////////
///// Store Network to Flash ///
////////////////////////////////
/*
Stores the network settings (IP, Gateway, Subnet Mask, DNS, MAC) into 
flash memory after padding to match the flash page size.
This function prepares the network data for storage in flash memory.
Check FLASH_PAGE_NET macro in parameters.  
*/
static void store_net_flash(void);
////////////////////////////////
////////////////////////////////

//////////////////////////////////
///  Manual IP/PORT configure  ///
//////////////////////////////////
/*
The purpose of this function is to prompt the user for 
the IP address and port number, 
and then store them in the variables ip and port.
It takes the following parameters:
- `ip` - a uint8_t array to store the server's IP address.
- `port` - pointer to an integer to store the server's port number.
*/
static void manual_serport(uint8_t *ip, int *port);
///////////////////////////////
///////////////////////////////
 
///////////////////////////////////
///// SERVER/PORT  Configuration ///
///////////////////////////////////
/*
Configures the server/port based on the selected server/port configuration type.
It handles 2 types of server/port setups: Last used or Manual.
 
Takes as input parameter:
  - servport_type: The server/port configuration type (Last used, Manual).
*/
static void configure_server_port(const ServerPortType servport_type, uint8_t *ip, int *port);
////////////////////////////////
////////////////////////////////
 
//////////////////////////////////
/// SERVER/PORT  Configuration ///
//////////////////////////////////
/*
Prompts the user to select a server/port configuration type.
1 - Last used settings
2 - Manual configuration
*/
static ServerPortType get_serport_config_choice(void);
///////////////////////////
///////////////////////////
 
/////////////////////////////////
/// SERVER/PORT Configuration ///
/////////////////////////////////
/*
Determines network settings based on user choice.
Calls configuration function and stores settings in Flash if needed.
Returns updated network info.
*/
void choose_server_port(uint8_t *ip, int *port);
///////////////////////////
///////////////////////////
 
////////////////////////////////////
//// Store SERVER/PORT to Flash  ///
////////////////////////////////////
/*
Stores the server/port settings into 
flash memory after padding to match the flash page size.
This function prepares the network data for storage in flash memory.
Check FLASH_PAGE_SERPORT macro in parameters.  
*/
static void store_serport_flash(uint8_t *ip, int *port);
////////////////////////////////
////////////////////////////////
 
/////////////////////////////
/// Load Last Used Config ///
/////////////////////////////
/*
Loads previously stored server/port configuration from flash memory.
*/
static void last_serport(uint8_t *ip, int *port);
///////////////////////////
///////////////////////////

#endif