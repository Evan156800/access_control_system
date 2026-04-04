//API for main

#ifndef WS2812_H
#define WS2812_H

#include "hardware/pio.h"
#include <stdbool.h>

void ws2812_program_init(PIO pio, uint sm, uint offset, uint pin, float freq, bool rgbw);//初始化 用哪個 PIO（pio0 / pio1）、state machine（0~3)、GPIO 腳位、LED timing 頻率
//void ws2812_put_pixel(PIO pio, uint sm, uint32_t pixel_grb);

void ws2812_init(PIO pio, uint sm, uint pin);
void ws2812_set_green();
void ws2812_set_red();
void ws2812_set_orange();
void ws2812_set_off();
// JSON-like 處理
void ws2812_handle_event(const char *json);

#endif
