// Client-server API(PICO)        //
// Netdata functions              //
// Version 0.9.0pi                //
// Bachelor's Work Project        //
// Technical University of Kosice //
// 23.02.2025                     //
// Nikita Kuropatkin              //
// Version for MCU                //
// W5100S-EVB-Pico                //

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "include/error.h" //All errors defined + function proto
#include "dhcp.h"
#include "include/parameters.h"
#include "include/addition.h"
#include "include/network_data.h"
#include "w5x00_spi.h"
#include "hardware/flash.h"
#include "pico/flash.h"

/*
Default settings of the chip for network connection (networking data).  
You can modify these values based on your needs.
*/
static wiz_NetInfo your_net_info =
{
  .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56}, // MAC address
  .ip = {192, 168, 11, 2},                     // IP address
  .sn = {255, 255, 255, 0},                    // Subnet Mask
  .gw = {192, 168, 11, 1},                     // Gateway
  .dns = {192, 168, 11, 1},                    // DNS server
  .dhcp = NETINFO_STATIC                       // DHCP OFF
};

/*
A pointer to the memory address of the specified flash page,  
offset from `XIP_BASE` (the base address for flash memory access).
Address of server/port configurations
Do not change this value!
*/
uint8_t *flash_target_net = (uint8_t *)(XIP_BASE + FLASH_PAGE_NET);

/*
Macro and global variable for DHCP initialization.  
Do not change these values!
*/
uint8_t g_ethernet_buf[ETHERNET_BUF_MAX_SIZE] = {0};

/*
A pointer to the memory address of the specified flash page,  
offset from `XIP_BASE` (the base address for flash memory access).
Address of server/port configurations
Do not change this value!
*/
uint8_t *flash_target_serport = (uint8_t *)(XIP_BASE + FLASH_PAGE_SERPORT);

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
static NetInfoType get_network_config_choice(void) {
  uint8_t input[ANS_SIZE];  
  printf("Select the network configuration type:\n");
  printf("1. Last used\n2. DHCP\n3. Manual\n");
  printf("Enter your choice (1-3): ");
  
  if (fgets(input, ANS_SIZE, stdin) == NULL) {
    exit_with_error(ERROR_GETTING_INPUT, "Error reading input");
  }

  int choice = atoi(input);

  switch (choice) {
    case 1: return NETDATA_LAST;
    case 2: return NETDATA_DHCP;
    case 3: return NETDATA_MANUAL;
    default:
      printf("Invalid choice. Defaulting to 'Default' network settings.\n");
      return NETDATA_DEFAULT;
  }
}
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
wiz_NetInfo choose_net_data(void) {
  NetInfoType netinfo_type = get_network_config_choice();
  configure_network(netinfo_type);

  if (netinfo_type != NETDATA_LAST) {
    store_net_flash(); // Save new settings
  }

  return your_net_info;
}
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
static void get_net_data(char *buffer) {
  if (fgets(buffer, ANS_SIZE, stdin) == NULL) {
    exit_with_error(ERROR_GETTING_INPUT, "Error reading input");
  }

  // Check for missing newline, indicating buffer overflow
  if (buffer[strlen(buffer) - 1] != '\n') {
    exit_with_error(WRONG_NETWORK_CONFIG, "Entered network data has wrong format");
  }
}
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
static void char_converter(char *input, uint8_t *output) {
  int octet;
  int i = 0;

  // Split input string into tokens using "." as the delimiter
  char *ptr = strtok(input, ".");  

  // Convert first octet to integer and store it
  octet = atoi(ptr);
  output[i] = octet;

  // Process remaining octets
  while (ptr != NULL) {
    i++;
    ptr = strtok(NULL, ".");
    if (ptr == NULL) break; // Stop if there are no more tokens

    octet = atoi(ptr); // Convert octet to integer
    output[i] = octet; 
  }
}
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
static void mac_converter(char *input, uint8_t *output) {
  int mac_part;
  int i = 0;

  // Split input string into tokens using ":" as the delimiter
  char *ptr = strtok(input, ":");  
  if (ptr == NULL) return; // Ensure input is valid

  // Convert first MAC part from hex to integer and store it
  mac_part = (int)strtol(ptr, NULL, 16);
  output[i] = mac_part;

  // Process remaining MAC parts
  while (ptr != NULL) {
    i++;
    ptr = strtok(NULL, ":");
    if (ptr == NULL) break; // Stop if there are no more tokens

    mac_part = (int)strtol(ptr, NULL, 16); // Convert to integer
    output[i] = mac_part; 
  }
}
///////////////////////////
///////////////////////////

///////////////////////////
/// Checking Input IP   ///
///////////////////////////
/*
The purpose of this function is to check whether the  
user has entered a valid IP address in the correct format.  
If the entered IP address is not valid (not correctly formatted),  
the program exits.  
It takes the following parameter:  
- `ip` - contains the IP address to be validated.  

Valid format examples:  
- Dotted decimal format: 192.168.0.1  
- Each octet must have a maximum value of 255: 255.255.255.255  
  (the "maximum" IP address).
*/
void ip_check (char *ip)
{
 int test;
 int dot_count = 0;

 /*Checking dotted decimal form:*/
 for (int i = 0; i<(int)strlen(ip); i++) { //Checking amount of dots in ipv4
    if (ip[i] == '.') dot_count++;
 }
 /*If amount of dots is less than 3 - exits*/
 if (dot_count != 3) exit_with_error(ERROR_IP_INPUT,"Invalid IP"); 

 /*Checking maximal value of octets:*/
 char *ptr = strtok(ip, "."); //Divide string to tokens
 test = atoi(ptr); //One quarter of ipv4 to a number

 /*Checking first octet of IP*/
 if (test<IPSTART || test>IPEND) exit_with_error(ERROR_IP_INPUT,"Invalid IP"); 
 while (ptr != NULL) {
    ptr = strtok(NULL, ".");
    test = atoi(ptr); //Convert one octet of ipv4 to a decimal form
    /*Checking other octets of IP*/
    if (test<IPSTART || test>IPEND) exit_with_error(ERROR_IP_INPUT,"Invalid IP"); 
 }
}
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
static void mac_check(char *mac) {
  int test;
  int semi_count = 0;

  // Count colons in the MAC address
  for (int i = 0; i < (int)strlen(mac); i++) {
    if (mac[i] == ':') semi_count++;
  }

  // A valid MAC should have exactly 5 colons
  if (semi_count != 5) {
    exit_with_error(ERROR_IP_INPUT, "Invalid MAC");
  }

  // Validate each MAC segment
  char *ptr = strtok(mac, ":");
  test = (int)strtol(ptr, NULL, 16);

  // Check the first MAC segment
  if (test < IPSTART || test > IPEND) {
    exit_with_error(ERROR_IP_INPUT, "Invalid MAC");
  }

  while (ptr != NULL) {
    ptr = strtok(NULL, ":");
    test = (int)strtol(ptr, NULL, 16);
    
    // Validate remaining MAC segments
    if (test < IPSTART || test > IPEND) {
      exit_with_error(ERROR_IP_INPUT, "Invalid MAC");
    }
  }
}
///////////////////////////
///////////////////////////

/////////////////////////////
/// Manual Network Setup  ///
/////////////////////////////
/*
Prompts the user to manually enter network settings, 
validates them, and converts them to numeric format.
*/
static void manual_net(void) {
  char buffer[ANS_SIZE];
  char copy_buffer[ANS_SIZE];

  // Get and process MAC address
  printf("Enter your MAC: ");
  get_net_data(buffer);
  memcpy(copy_buffer, buffer, MAC_ADDRESS_SIZE);
  mac_check(copy_buffer); // Validate MAC address format
  mac_converter(buffer, your_net_info.mac); // Convert MAC to numeric form

  // Get and process IP address
  printf("Enter your IP: ");
  get_net_data(buffer);
  memcpy(copy_buffer, buffer, IPSZ);
  ip_check(copy_buffer); // Validate IP format
  char_converter(buffer, your_net_info.ip); // Convert IP to numeric form

  // Get and process subnet mask
  printf("Enter subnet mask: ");
  get_net_data(buffer);
  memcpy(copy_buffer, buffer, IPSZ);
  ip_check(copy_buffer);
  char_converter(buffer, your_net_info.sn);

  // Get and process default gateway
  printf("Enter default gateway IP: ");
  get_net_data(buffer);
  memcpy(copy_buffer, buffer, IPSZ);
  ip_check(copy_buffer);
  char_converter(buffer, your_net_info.gw);

  // Get and process DNS server address
  printf("Enter DNS IP: ");
  get_net_data(buffer);
  memcpy(copy_buffer, buffer, IPSZ);
  ip_check(copy_buffer);
  char_converter(buffer, your_net_info.dns);
}
///////////////////////////
///////////////////////////

/////////////////////////////
/// Load Last Used Config ///
/////////////////////////////
/*
Loads previously stored network configuration from flash memory.
*/
static void last_used(void) {
  // Load IP address
  read_net_flash(IP_OFFSET, your_net_info.ip, NET_DATA_SIZE);   
  // Load default gateway
  read_net_flash(GW_OFFSET, your_net_info.gw, NET_DATA_SIZE);  
  // Load subnet mask 
  read_net_flash(SN_OFFSET, your_net_info.sn, NET_DATA_SIZE);  
  // Load DNS server 
  read_net_flash(DNS_OFFSET, your_net_info.dns, NET_DATA_SIZE); 
  // Load MAC address
  read_net_flash(MAC_OFFSET, your_net_info.mac, MAC_SIZE); 
}
///////////////////////////
///////////////////////////

////////////////////////////
/// DHCP Configuration   ///
////////////////////////////
/*
Attempts to obtain an IP address dynamically using DHCP.
Retries a limited number of times before failing.
*/
static void dhcp_usage(void) {
  int retval;
  uint8_t dhcp_retry = 0;

  while (1) {
    printf("!");

    // Check if DHCP is enabled
    if (your_net_info.dhcp == NETINFO_DHCP) {
      retval = DHCP_run();
      
      // Successful DHCP lease acquired
      if (retval == DHCP_IP_LEASED) {
        printf("DHCP success\n");
        return; // Exit function after obtaining an IP lease
      }

      // DHCP lease attempt failed, increment retry counter
      else if (retval == DHCP_FAILED) {
        dhcp_retry++;

        if (dhcp_retry <= DHCP_RETRY_COUNT) {
          printf("DHCP timeout occurred, retry %d\n", dhcp_retry);
        }
      }

      // Stop DHCP process after exceeding retry limit
      if (dhcp_retry > DHCP_RETRY_COUNT) {
        DHCP_stop();
        exit_with_error(DHCP_ERROR, "DHCP failed");
      }

      wiz_delay_ms(1000); // Wait before retrying
    }
  }
}
///////////////////////////
///////////////////////////

//////////////////////////////
/// DHCP Callback - Assign ///
//////////////////////////////
/*
Retrieves the assigned network parameters from DHCP 
and updates the global network info structure.
*/
static void wizchip_dhcp_assign(void) {
  getIPfromDHCP(your_net_info.ip);
  getGWfromDHCP(your_net_info.gw);
  getSNfromDHCP(your_net_info.sn);
  getDNSfromDHCP(your_net_info.dns);

  your_net_info.dhcp = NETINFO_DHCP;

  printf("\nDHCP leased time: %ld seconds\n", getDHCPLeasetime());
}
///////////////////////////
///////////////////////////

////////////////////////////////
/// DHCP Callback - Conflict ///
////////////////////////////////
/*
Handles IP conflicts detected by DHCP and exits with an error.
*/
static void wizchip_dhcp_conflict(void) {
  exit_with_error(CONFLICT_DHCP, "Conflict IP from DHCP");
}
///////////////////////////
///////////////////////////

///////////////////////
/// Initialize DHCP ///
///////////////////////
/*
Initializes the DHCP client and registers necessary callback functions.
*/
static void wizchip_dhcp_init(void) {
  DHCP_init(SOCKET_DHCP, g_ethernet_buf);

  // Register callback functions for DHCP events
  reg_dhcp_cbfunc(wizchip_dhcp_assign, wizchip_dhcp_assign, wizchip_dhcp_conflict);
}
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
static void configure_network(const NetInfoType netinfo_type) {
  switch (netinfo_type) {
    case NETDATA_LAST:
      printf("Configuring with last used network settings...\n");
      last_used(); // Use last known network settings
      break;

    case NETDATA_DHCP:
      printf("Configuring with DHCP...\n");
      your_net_info.dhcp = NETINFO_DHCP; // Mark as DHCP
      wizchip_dhcp_init(); // Initialize DHCP
      dhcp_usage(); // Attempt to obtain network settings via DHCP
      break;

    case NETDATA_MANUAL:
      printf("Configuring with manual network settings...\n");
      manual_net(); // Prompt for manual network settings
      break;

    default:
      printf("Invalid network configuration type.\n");
      break;
  }
}
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
static void read_net_flash(const int offset, uint8_t *buffer, const int size) {
  for (int i = offset, j = 0; i < offset + size; i++, j++) {
    buffer[j] = flash_target_net[i]; // Copy flash data to buffer
  }
}
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
static void call_flash_range_erase(void *param) {
  uint32_t offset = (uint32_t)param; // Get the offset from parameters
  flash_range_erase(offset, FLASH_SECTOR_SIZE); // Erase flash sector
}
////////////////////////////////
////////////////////////////////

////////////////////////////////
///// Flash Program Callback ///
////////////////////////////////
/*
This function is executed to write data to flash memory when it's safe to 
do so. It takes the offset and data to be written.

Takes as input parameters:
  - params: A pointer to an array containing the offset and data to be written.
*/
static void call_flash_range_program(void *params) {
  uint32_t offset = ((uintptr_t*)params)[0]; // Get offset from parameters
  // Get pointer to data from parameters
  const uint8_t *data = (const uint8_t *)((uintptr_t*)params)[1]; 
  flash_range_program(offset, data, FLASH_PAGE_SIZE); // Write data to flash
}
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
static void store_net_flash(void) {
  // Initialize buffer for padded data
  uint8_t padded_data[FLASH_PAGE_SIZE] = {0}; 

  int i, j = 0;
  // Copy IP into padded_data
  for (j = IP_OFFSET, i = 0; j < 4; j++, i++) {
    padded_data[j] = your_net_info.ip[i]; // Copy each byte of IP
  }
  // Copy Gateway into padded_data
  for (j = GW_OFFSET, i = 0; j < 8; j++, i++) {
    padded_data[j] = your_net_info.gw[i]; // Copy each byte of Gateway
  }
  // Copy Subnet Mask into padded_data
  for (j = SN_OFFSET, i = 0; j < 12; j++, i++) {
    padded_data[j] = your_net_info.sn[i]; // Copy each byte of Subnet Mask
  }
  // Copy DNS into padded_data
  for (j = DNS_OFFSET, i = 0; j < 16; j++, i++) {
    padded_data[j] = your_net_info.dns[i]; // Copy each byte of DNS
  }
  // Copy MAC address into padded_data
  for (j = MAC_OFFSET, i = 0; j < 22; j++, i++) {
    padded_data[j] = your_net_info.mac[i]; // Copy each byte of MAC address
  }

  // Safely execute the flash erase and program operations
  if (flash_safe_execute(call_flash_range_erase, (void*)FLASH_PAGE_NET, UINT32_MAX) != PICO_OK) {
    exit_with_error(ERROR_FLASH, "Error erasing flash");
  }
  uintptr_t params[] = { FLASH_PAGE_NET, (uintptr_t)padded_data };
  if (flash_safe_execute(call_flash_range_program, params, UINT32_MAX) != PICO_OK) {
    exit_with_error(ERROR_FLASH, "Error programming flash");
  }

  printf("Network data stored to flash successful!\n"); // Notify success
}
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
static void manual_serport(uint8_t *ip, int *port) {
 // Buffer to store user input for port number
 char port_buff[PINSZ];
 // Buffer to store the IP address entered by the user as a string
 char buffer[IPSZ];
 // Copy of the IP address buffer to pass for validation
 char copy_buffer[IPSZ];
 // Buffer to store the user's answer to the question (yes/no)
 char ans[ANS_SIZE];

 // Prompt user for IP address input
 printf("Enter IP of server: ");
 if (fgets(buffer, IPSZ, stdin) == NULL) {
    exit_with_error(ERROR_GETTING_INPUT, "Error reading input");
 }

 // Buffer overflow check
 if (buffer[strlen(buffer) - 1] != '\n') {
    exit_with_error(WRONG_NETWORK_CONFIG,"Entered network data has wrong format");
 }
        
 // Copy the entered IP address to another buffer for validation
 memcpy(copy_buffer, buffer, IPSZ);
 ip_check(copy_buffer); // Validate the entered IP address

 // Convert the IP address to uint8_t array
 char_converter(buffer, ip);

 // Prompt user for port number input
 printf("Enter number of port:");
 if (fgets(port_buff, PINSZ, stdin) == NULL) {
    exit_with_error(ERROR_GETTING_INPUT, "Error reading input");
 }

 // Buffer overflow check
 if (port_buff[strlen(port_buff) - 1] != '\n') {
    exit_with_error(WRONG_NETWORK_CONFIG,"Entered network data has wrong format");
 }

 // Convert the entered port number from string to integer
 *port = atoi(port_buff);

 // Check if the entered port number is within the valid range
 if (!(*port > PORT_START && *port <= PORT_END)) {
    exit_with_error(ERROR_PORT_INPUT,"Invalid port"); 
 }
}
///////////////////////////////
///////////////////////////////

///////////////////////////////////
//// SERVER/PORT  Configuration ///
///////////////////////////////////
/*
Configures the server/port based on the selected server/port configuration type.
It handles 2 types of server/port setups: Last used or Manual.

Takes as input parameter:
  - servport_type: The server/port configuration type (Last used, Manual).
*/
static void configure_server_port(const ServerPortType servport_type, uint8_t *ip, int *port) 
{
  switch (servport_type) {
    case SERPORT_LAST:
      printf("Configuring with last used server/port settings...\n");
      last_serport(ip, port); // Use last known server/port settings
      break;

    case SERPORT_MANUAL:
      printf("Configuring with manual server/port settings...\n");
      manual_serport(ip, port); // Prompt for manual server/port settings
      break;

    default:
      printf("Invalid server/port configuration type.\n");
      break;
  }
}
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
static ServerPortType get_serport_config_choice(void) {
  uint8_t input[ANS_SIZE];  
  printf("Select the server/port configuration type:\n");
  printf("1. Last used\n2. Manual\n");
  printf("Enter your choice (1-2): ");
  
  if (fgets(input, ANS_SIZE, stdin) == NULL) {
    exit_with_error(ERROR_GETTING_INPUT, "Error reading input");
  }

  int choice = atoi(input);

  switch (choice) {
    case 1: return SERPORT_LAST;
    case 2: return SERPORT_MANUAL;
    default:
      printf("Invalid choice. Defaulting to 'Default' network settings.\n");
      return SERPORT_DEFAULT;
  }
}
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
void choose_server_port(uint8_t *ip, int *port) {

  ServerPortType server_port_type = get_serport_config_choice();
  configure_server_port(server_port_type, ip, port);

  if (server_port_type != SERPORT_LAST) {
    store_serport_flash(ip, port); // Save new settings
  }

}
///////////////////////////
///////////////////////////

////////////////////////////////////
///// Store SERVER/PORT to Flash ///
////////////////////////////////////
/*
Stores the server/port settings into 
flash memory after padding to match the flash page size.
This function prepares the network data for storage in flash memory.
Check FLASH_PAGE_SERPORT macro in parameters.  
*/
static void store_serport_flash(uint8_t *ip, int *port) {
  // Initialize buffer for padded data
  uint8_t padded_data[FLASH_PAGE_SIZE] = {0}; 

  int i, j = 0;
  // Copy IP into padded_data
  for (j = SERVER_IP_OFFSET, i = 0; j < 4; j++, i++) {
    padded_data[j] = ip[i]; // Copy each byte of SERVER IP
  }
  padded_data[PORT_OFFSET] = (uint8_t)((*port >> 8) & 0xFF); // Store high byte
  padded_data[PORT_OFFSET + 1] = (uint8_t)((*port) & 0xFF);  // Store low byte

  // Safely execute the flash erase and program operations
  if (flash_safe_execute(call_flash_range_erase, (void*)FLASH_PAGE_SERPORT, UINT32_MAX) != PICO_OK) {
    exit_with_error(ERROR_FLASH, "Error erasing flash");
  }
  uintptr_t params[] = { FLASH_PAGE_SERPORT, (uintptr_t)padded_data };
  if (flash_safe_execute(call_flash_range_program, params, UINT32_MAX) != PICO_OK) {
    exit_with_error(ERROR_FLASH, "Error programming flash");
  }

  printf("Server/port data stored to flash successful!\n"); // Notify success
}
////////////////////////////////
////////////////////////////////

/////////////////////////////
/// Load Last Used Config ///
/////////////////////////////
/*
Loads previously stored server/port configuration from flash memory.
*/
static void last_serport(uint8_t *ip, int *port) {
  for (int i = SERVER_IP_OFFSET, j = 0; i < SERVER_IP_OFFSET + NET_DATA_SIZE; i++, j++) {
    ip[j] = flash_target_serport[i]; // Copy flash data to buffer
  }
  *port = (flash_target_serport[PORT_OFFSET] << 8) | flash_target_serport[PORT_OFFSET + 1];
}
///////////////////////////
///////////////////////////