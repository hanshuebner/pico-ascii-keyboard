cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(pico-ascii-keyboard C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()
set(FAMILY rp2040)
set(BOARD pico_sdk)
set(TINYUSB_FAMILY_PROJECT_NAME_PREFIX "tinyusb_host_")

# Function to set up targets
function(set_up_target target_name)
    add_executable(${target_name} ${ARGN})

    pico_enable_stdio_usb(${target_name} 0)
    pico_enable_stdio_uart(${target_name} 1)
    pico_add_extra_outputs(${target_name})
    target_link_libraries(${target_name} pico_stdlib tinyusb_host tinyusb_board)
    target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_LIST_DIR})
endfunction()

# Shared source files
set(SHARED_SOURCES hid-to-ascii.c hid-to-ascii.h)

# Main application
set_up_target(pico-ascii-keyboard pico-ascii-keyboard.c ${SHARED_SOURCES})
