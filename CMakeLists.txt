cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)



project(PICO_MSC C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)


pico_sdk_init()

add_executable(PICO_MSC)

target_sources(PICO_MSC PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        ${CMAKE_CURRENT_LIST_DIR}/usb_descriptors.c
        ${CMAKE_CURRENT_LIST_DIR}/msc_disk.c
        )

# Make sure TinyUSB can find tusb_config.h
target_include_directories(PICO_MSC PUBLIC
        ${CMAKE_CURRENT_LIST_DIR})

# In addition to pico_stdlib required for common PicoSDK functionality, add dependency on tinyusb_device
# for TinyUSB device support and tinyusb_board for the additional board support library used by the example
target_link_libraries(PICO_MSC PUBLIC pico_stdlib tinyusb_device tinyusb_board hardware_flash)

# Uncomment this line to enable fix for Errata RP2040-E5 (the fix requires use of GPIO 15)
#target_compile_definitions(PICO_MSC PUBLIC PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1)

pico_add_extra_outputs(PICO_MSC)

pico_enable_stdio_usb(PICO_MSC 0)
pico_enable_stdio_uart(PICO_MSC 1)


set(PICO_COPY_TO_RAM,1)
#add_compile_definitions(PICO_COPY_TO_RAM=1)
#add_definitions(-DPICO_COPY_TO_RAM=1)