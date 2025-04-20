// Client-server API(PICO)        //
// Network functions              //
// Version 0.9.0pi                //
// Bachelor's Work Project        //
// Technical University of Kosice //
// 23.02.2025                     //
// Nikita Kuropatkin              //
// Version for MCU                //
// W5100S-EVB-Pico                //

/* 
This header file declares network TCP/IP functions
for a client-server application. Function bodies
are in network.c. 
*/
#ifndef NETWORK_H
#define NETWORK_H
#include <stdint.h>

//////////////////////////////////////////
/// Data Receiver ///
//////////////////////////////////////////
/*
The purpose of this function is to receive data over open sockets 
on Raspberry Pi Pico.
It takes the following parameters:  
1. `sockfd` - the ID of the socket where the data will be received.  
2. `msg` - a buffer where the received message will be written.  
3. `size` - the size of the message.  
The program exits in case of an error.
*/
void read_pico(const int sockfd, uint8_t *msg, const unsigned int size);
//////////////////////////////////////////
//////////////////////////////////////////

//////////////////////////////////////////
/// Data Sender ///
//////////////////////////////////////////
/*
The purpose of this function is to send data over open sockets 
on Raspberry Pi Pico.  
It takes the following parameters:  
1. `sockfd` - the ID of the socket from which the data will be sent.  
2. `msg` - a buffer containing the message to be sent.  
3. `size` - the size of the message.  
The program exits in case of an error.
*/
void write_pico(const int sockfd, uint8_t *msg, const unsigned int size);
//////////////////////////////////////////
//////////////////////////////////////////

//////////////////////////////////////////
/// Socket Closer ///
//////////////////////////////////////////
/*
The purpose of this function is to close existing sockets 
on Raspberry Pi Pico.  
It takes the following parameter:  
- `sockfd` - the ID of the socket to be closed.  
The program exits in case of an error.
*/
void sockct_cls(const int sockfd);
///////////////////////////////////////////////////
///////////////////////////////////////////////////

///////////////////////////////
/// Socket Creation Checker ///
///////////////////////////////
/*
The purpose of this function is to check if a socket was successfully created.  
It takes the following parameter:  
- `sockfd` - a variable containing the ID of the socket to be checked.  
The program exits in case of an error.
*/
void sock_check(const int sockfd);
//////////////////////////////
//////////////////////////////

#endif
