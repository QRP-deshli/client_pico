# Client-server API (mbed)        
# Client side                    
# Version 0.5.1                   
# Bachelor's work project        
# Technical University of Kosice 
# 10.11.2024                     
# Nikita Kuropatkin

# Define the target name for the executable
set(TARGET_NAME client_pico)

# Add all .c files from the project folder, including client_mbed.c
file(GLOB SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c  
    ${CMAKE_CURRENT_SOURCE_DIR}/src/client/*.c  
    ${CMAKE_CURRENT_SOURCE_DIR}/client.c 
)

# Add the executable with the source files
add_executable(${TARGET_NAME} ${SRC_FILES})

# Include the current directory for header files (if any headers are in the project folder)
target_include_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include  # Include headers from the current directory
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include/client  # Include headers from the current directory
)

# Link libraries (adjust as necessary)
target_link_libraries(${TARGET_NAME} 
    pico_unique_id
    hardware_clocks
    hardware_timer
    hardware_sync
    pico_stdlib
    pico_stdio_uart
    hardware_spi
    hardware_dma
    ETHERNET_FILES
    IOLIBRARY_FILES
    TIMER_FILES
    DHCP_FILES
)

# Macros for defining communication type
pico_enable_stdio_usb(${TARGET_NAME} 1) # Set to 0 for UART communication
pico_enable_stdio_uart(${TARGET_NAME} 0) # Set to 1 for USB communication

# Set to 0 to use UART cable for communication, set to 1, to use USB
target_compile_definitions(${TARGET_NAME} PRIVATE PICO_STDIO_USB_ENABLE=1)

# Add extra outputs (like UF2 file for Raspberry Pi Pico)
pico_add_extra_outputs(${TARGET_NAME})
