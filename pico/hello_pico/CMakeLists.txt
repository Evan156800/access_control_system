cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(hello_pico C CXX ASM)

pico_sdk_init()

add_executable(hello_pico
    main.c
    drivers/ws2812/ws2812.c
    drivers/sg90/sg90.c
    drivers/buzzer/buzzer.c
    drivers/pir/pir.c
)

target_link_libraries(hello_pico
    pico_stdlib
    hardware_uart
    hardware_pwm
    hardware_pio
)

target_include_directories(hello_pico PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/drivers/ws2812
)

# USB printf
pico_enable_stdio_usb(hello_pico 1)
pico_enable_stdio_uart(hello_pico 0)

pico_generate_pio_header(hello_pico ${CMAKE_CURRENT_LIST_DIR}/drivers/ws2812/ws2812.pio)

pico_add_extra_outputs(hello_pico)
