#include "ws2812.h"
#include "ws2812.pio.h"
#include "hardware/clocks.h"
#include <stdio.h>
#include <string.h>

void ws2812_program_init(PIO pio, uint sm, uint offset, uint pin, float freq, bool rgbw) {
    pio_gpio_init(pio, pin);
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true);

    pio_sm_config c = ws2812_program_get_default_config(offset);

    sm_config_set_sideset_pins(&c, pin);
    sm_config_set_out_shift(&c, false, true, rgbw ? 32 : 24);

    float div = (float)clock_get_hz(clk_sys) / (freq * 10.0f);
    sm_config_set_clkdiv(&c, div);

    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
}

/*void ws2812_put_pixel(PIO pio, uint sm, uint32_t pixel_grb) {
    pio_sm_put_blocking(pio, sm, pixel_grb);
}*/

static PIO ws_pio;
static uint ws_sm;

// ===== 基本 function =====
static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(ws_pio, ws_sm, pixel_grb << 8u);
}

static uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(g) << 16) |
           ((uint32_t)(r) << 8)  |
           (uint32_t)(b);
}

// ===== init =====
void ws2812_init(PIO pio, uint sm, uint pin) {
    ws_pio = pio;
    ws_sm = sm;

    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, pin, 800000, false);
}

// ===== LED 控制 =====
void ws2812_set_green() {
    put_pixel(urgb_u32(0, 255, 0));
}

void ws2812_set_red() {
    put_pixel(urgb_u32(255, 0, 0));
}

void ws2812_set_orange() {
    put_pixel(urgb_u32(255, 165, 0));
}

void ws2812_set_off() {
    put_pixel(urgb_u32(0, 0, 0));
}

// ===== JSON-like parsing（核心）=====
void ws2812_handle_event(const char *json) {

    // 判斷 device
    if (strstr(json, "\"device\":\"pico\"") == NULL) {
        printf("Not pico device\n");
        return;
    }

    // 判斷事件
    if (strstr(json, "\"event\":\"pir\"") != NULL) {

        if (strstr(json, "\"value\":0") != NULL) {
            printf("Camera off -> none\n");
            ws2812_set_off();
        } 
        else if (strstr(json, "\"value\":1") != NULL) {
            printf("Camera detected -> Orange\n");
            ws2812_set_orange();
        }
        else if (strstr(json, "\"value\":2") != NULL) {
            printf("Pass -> Green\n");
            ws2812_set_green();
        }	
        else if (strstr(json, "\"value\":3") != NULL) {
            printf("Fail -> Red\n");
            ws2812_set_red();
        }	
        else {
            printf("Unknown PIR value\n");
            ws2812_set_off();
        }

    } else {
        printf("Unknown event\n");
    }
}
