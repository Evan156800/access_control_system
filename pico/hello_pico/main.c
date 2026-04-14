#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/pio.h"

#include "drivers/ws2812/ws2812.h"
#include "drivers/sg90/sg90.h"
#include "drivers/buzzer/buzzer.h"

// ================= PIN =================
#define WS2812_PIN 2
#define SG90_PIN 15
#define BUZZER_PIN 16

// ================= UART =================
#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

// ================= UART BUFFER =================
#define BUF_SIZE 128
static char uart_buf[BUF_SIZE];
static int uart_idx = 0;

// ================= UART INIT =================
void uart_init_custom() {
    uart_init(UART_ID, BAUD_RATE);

    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_fifo_enabled(UART_ID, true);
}

// ================= UART PARSER =================
void uart_read_nonblock() {

    while (uart_is_readable(UART_ID)) {

        char c = uart_getc(UART_ID);

        if (c == '\r') continue;

        if (c == '\n') {

            uart_buf[uart_idx] = '\0';

            if (uart_idx > 0) {

                printf("[UART] %s\n", uart_buf);

                // ===== 動作控制 =====
		/*if (strstr(uart_buf, "detect")) {
    		    printf("ACTION: DETECT\n");
		    sleep_ms(300);
		    uart_puts(UART_ID,"pass\n");   
		}*/
                if (strstr(uart_buf, "pass")) {

                    printf("ACTION: PASS\n");
		    ws2812_set_green();
                    sg90_open(SG90_PIN);
                    buzzer_beep(200);
		    sleep_ms(3000);
		    sg90_close(SG90_PIN);
		    ws2812_set_off();
                }
                else if (strstr(uart_buf, "fail")) {

                    printf("ACTION: FAIL\n");
		    ws2812_set_red();
                    buzzer_beep(100);
                    sleep_ms(100);
                    buzzer_beep(100);
                    sg90_close(SG90_PIN);
		    ws2812_set_off();
                }
            }

            uart_idx = 0;
            memset(uart_buf, 0, BUF_SIZE);
        }
        else {
            if (uart_idx < BUF_SIZE - 1) {
                uart_buf[uart_idx++] = c;
            }
        }
    }
}

// ================= MAIN =================
int main() {

    stdio_init_all();
    sleep_ms(1500);

    printf("System Start (Pico)\n");

    // init drivers
    ws2812_init(pio0, 0, WS2812_PIN);
    sg90_init(SG90_PIN);
    buzzer_init(BUZZER_PIN);
    uart_init_custom();

    // 預設關門
    sg90_close(SG90_PIN);

    while (true) {

        uart_read_nonblock();

        sleep_ms(10);
    }
}
