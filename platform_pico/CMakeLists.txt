cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico2_w)

# Include Pico SDK configuration
include(pico_sdk_import.cmake)

project(platypus C CXX ASM)
pico_sdk_init()

# ====================================================================
# 1. GENERATE SDIO INLINE COMPILATION STUB
# ====================================================================
set(SDIO_STUB_FILE "${CMAKE_CURRENT_BINARY_DIR}/sdio_hardware_stub.c")
file(WRITE ${SDIO_STUB_FILE} "
#include <stddef.h>
void *sd_sdio_ctor(char const *pcName) { 
    (void)pcName; 
    return NULL; 
}
")

# ====================================================================
# 2. EXECUTABLE TARGET AND SOURCE FILES
# ====================================================================
add_executable(platypus
    main.c
    hid.c
    ../common/parser.c
    hw_config.c 
    usb_descriptors.c                        
    ${SDIO_STUB_FILE}                        

    # --- FatFS Pure Logic Engine ---
    lib/no-OS-FatFS/src/ff15/source/ff.c
    lib/no-OS-FatFS/src/ff15/source/ffunicode.c
    lib/no-OS-FatFS/src/ff15/source/ffsystem.c

    # --- Carlk3 Platform Glue & Infrastructure Tools ---
    lib/no-OS-FatFS/src/src/glue.c
    lib/no-OS-FatFS/src/src/f_util.c
    lib/no-OS-FatFS/src/src/my_rtc.c
    lib/no-OS-FatFS/src/src/my_debug.c        
    lib/no-OS-FatFS/src/src/util.c            
    lib/no-OS-FatFS/src/src/crc.c             

    # --- Carlk3 Physical SD/SPI Hardware Drivers ---
    lib/no-OS-FatFS/src/sd_driver/sd_card.c
    lib/no-OS-FatFS/src/sd_driver/sd_timeouts.c
    lib/no-OS-FatFS/src/sd_driver/SPI/my_spi.c
    lib/no-OS-FatFS/src/sd_driver/SPI/sd_card_spi.c
    lib/no-OS-FatFS/src/sd_driver/SPI/sd_spi.c
)

# ====================================================================
# 3. INCLUDE DIRECTORIES
# ====================================================================
target_include_directories(platypus PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/no-OS-FatFS/src
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/no-OS-FatFS/src/src
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/no-OS-FatFS/src/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/no-OS-FatFS/src/ff15/source
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/no-OS-FatFS/src/sd_driver
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/no-OS-FatFS/src/sd_driver/SPI
)

# ====================================================================
# 4. LINK LIBRARIES
# ====================================================================
target_link_libraries(platypus PRIVATE
    pico_stdlib
    pico_cyw43_arch_none     
    pico_aon_timer
    hardware_spi
    hardware_gpio
    hardware_pio             
    hardware_dma             
    hardware_pwm
    tinyusb_device
)

# ====================================================================
# 5. STDIO INTERFACE CONSTRAINTS (Moved safely below target definition)
# ====================================================================
pico_enable_stdio_usb(platypus 0)
pico_enable_stdio_uart(platypus 1)

pico_add_extra_outputs(platypus)
