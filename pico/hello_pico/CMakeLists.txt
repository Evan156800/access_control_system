cmake_minimum_required(VERSION 3.13)

include(../pico-sdk/external/pico_sdk_import.cmake)

project(hello_pico)

pico_sdk_init()

add_executable(hello_pico
    main.c
)

target_link_libraries(hello_pico pico_stdlib)

# ⭐ 讓 printf 出現在 USB（PuTTY）
pico_enable_stdio_usb(hello_pico 1)
pico_enable_stdio_uart(hello_pico 0)

pico_add_extra_outputs(hello_pico)
