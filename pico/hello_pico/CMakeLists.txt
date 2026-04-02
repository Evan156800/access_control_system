cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(hello_pico C CXX ASM)

pico_sdk_init()

add_executable(hello_pico
    main.c
    ws2812.c
)

target_link_libraries(hello_pico
    pico_stdlib
    hardware_uart
    hardware_pio
)

# USB printf
pico_enable_stdio_usb(hello_pico 1)
pico_enable_stdio_uart(hello_pico 0)

pico_generate_pio_header(hello_pico ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

pico_add_extra_outputs(hello_pico)
