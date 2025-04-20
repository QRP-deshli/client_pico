// Client-server API(PICO)        //
// Chip Initialization and UART   //
// Version 0.9.0pi                //
// Bachelor's Work Project        //
// Technical University of Kosice //
// 23.02.2025                     //
// Nikita Kuropatkin              //
// Version for MCU                //
// W5100S-EVB-Pico                //

#include "wizchip_conf.h"
#include "port_common.h"
#include "w5x00_spi.h"
#include "include/chip_init.h"
#include "include/timing.h"
#if PICO_STDIO_USB_ENABLE
    #include "tusb.h"
#else
    #include "pico/stdio_uart.h"
#endif
#include "pico/stdlib.h"

/*
Initializes the UART interface on the Raspberry Pi Pico, 
including hardware flow control using RTS and CTS and FIFO.
*/
void chip_uart_init(void) {
 stdio_uart_init();
  
 // Configure GPIO pins for RTS and CTS hardware flow control
 gpio_set_function(2, GPIO_FUNC_UART); // CTS (Clear to Send)
 gpio_set_function(3, GPIO_FUNC_UART); // RTS (Request to Send)

 // Enable UART FIFO and hardware flow control
 uart_set_fifo_enabled(uart0, true);
 uart_set_hw_flow(uart0, true, true);  // Enable RTS/CTS flow control
}


/* 
Waits for user input and repeatedly displays a welcome message 
until the user interacts with the terminal.
This function is used when the user chooses 
USB for communication with the board. 
*/
#if PICO_STDIO_USB_ENABLE
void user_await_usb(void) {
 // Initialize TinyUSB, as we cannot use getchar() due to its blocking behavior 
 tusb_init();
   
 while (1) {
    // Send a welcome message over USB
    tud_cdc_write_str("Welcome to PICO-encryptor, enter something to start!\n");
    tud_cdc_write_flush();  // Make sure data is transmitted
    sleep_ms(500);  // Wait for 500 ms
   
    // Check if there's any data available from USB
    if (tud_cdc_available()) {
        sleep_ms(1000);  // Wait 1s if user enters more than one char
               
        // Clear the input buffer 
        while (tud_cdc_available()) {
           tud_cdc_read_char();  // Read and discard characters
        }
   
        tud_cdc_write_str("\nStarting communication...\n");
        tud_cdc_write_str("Please connect the server to the encryptor with Ethernet.\n");
        tud_cdc_write_str("The MCU will not proceed with executing the program until this is done.\n");
        tud_cdc_write_flush();  // Ensure data is transmitted
   
        break;  // Exit the loop if the user enters something
    }
 }
}
#endif
   
/* 
Waits for user input and repeatedly displays a welcome message 
until the user interacts with the terminal.
This function is used when the user chooses 
UART for communication with the board. 
*/
void user_await_uart(void) {
 while (1) {
    // Send a welcome message over UART
    uart_puts(uart0, "Welcome to PICO-encryptor, enter something to start!\n");
    sleep_ms(500);  // Wait for 500 ms
           
    // Check if the user has entered any input
    if (uart_is_readable(uart0)) {
        sleep_ms(1000);// Waiting 1s if user more than one char 
        // Clear the input buffer
        while (uart_is_readable(uart0)) {
            uart_getc(uart0);
        }
        uart_puts(uart0, "\nStarting communication...\n");
        uart_puts(uart0, "Please connect the server to the encryptor with Ethernet.\n");
        uart_puts(uart0, "The MCU will not proceed with executing the program until this is done.\n");
        break;  // Exit the loop if the user enters something
    }
 }
}

/*
Initializes the Wiznet W5100S Ethernet chip. Configures SPI, resets the chip, 
performs initialization, and verifies proper operation. This function does 
not contain the main `wizchip_initialize()` because it requires an active PHY 
status. It is designed to inform the user of this requirement.
*/
void wiznet_chip_init_start(void) {
 wizchip_spi_initialize();  // Initialize the SPI interface for the chip
 wizchip_cris_initialize(); // Initialize critical section management
 wizchip_reset();           // Perform a hardware reset on the Wiznet chip
}

/*
Initializes chip settings and checks the PHY (physical layer) status.
If the MCU is not connected to the server via an Ethernet cable, 
this function will block the program from running until the connection is made.
*/
void wiznet_chip_init_end(void)  {
 wizchip_initialize(); // Initialize the chip       
 wizchip_check();      // Verify proper initialization and configuration
}
