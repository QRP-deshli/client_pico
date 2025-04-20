// Client-server API(PICO)        //
// Chip Initialization and UART   //
// Version 0.9.0pi                //
// Bachelor's Work Project        //
// Technical University of Kosice //
// 23.02.2025                     //
// Nikita Kuropatkin              //
// Version for MCU                //
// W5100S-EVB-Pico                //

/* 
This header file declares functions for initializing 
hardware components required for the client-server application, 
including UART and the Wiznet W5100S Ethernet chip.
Function definitions are in chip_init.c. 
*/
#ifndef CHIP_INIT_H
#define CHIP_INIT_H

/*
Initializes the UART interface on the Raspberry Pi Pico, 
including hardware flow control using RTS and CTS and FIFO.
*/
void chip_uart_init(void);

/* 
Waits for user input and repeatedly displays a welcome message 
until the user interacts with the terminal.
This function is used when the user chooses 
USB for communication with the board. 
*/
void user_await_usb(void);

/* 
Waits for user input and repeatedly displays a welcome message 
until the user interacts with the terminal.
This function is used when the user chooses 
UART for communication with the board. 
*/
void user_await_uart(void);

/*
Initializes the Wiznet W5100S Ethernet chip. Configures SPI, resets the chip, 
performs initialization, and verifies proper operation. This function does 
not contain the main `wizchip_initialize()` because it requires an active PHY 
status. It is designed to inform the user of this requirement.
*/
void wiznet_chip_init_start(void);

/*
Initializes chip settings and checks the PHY (physical layer) status.
If the MCU is not connected to the server via an Ethernet cable, 
this function will block the program from running until the connection is made.
*/
void wiznet_chip_init_end(void);

#endif
