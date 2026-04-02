#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "ws2812.h"
#include "ws2812.pio.h"

#define WS2812_PIN 2
int main() {
    stdio_init_all();

    PIO pio = pio0;
    uint sm = 0;

    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, false);

    while (true) {
	printf("running\n");

        // 發送顏色 (GRB 格式)
        ws2812_put_pixel(pio, sm, 0x00FF00 << 8); // 綠色

        sleep_ms(500);

        ws2812_put_pixel(pio, sm, 0xFF0000 << 8); // 紅色

        sleep_ms(500);
    }
}
