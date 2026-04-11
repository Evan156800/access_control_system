#include <string.h>
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "uart.h"

#define UART_ID uart0
#define BAUD_RATE 115200

#define UART_TX_PIN 0
#define UART_RX_PIN 1

static char rx_buffer[128];
static int rx_index = 0;

void uart_custom_init() {
    uart_init(UART_ID, BAUD_RATE);

    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
}

/**
 * Non-blocking UART line reader
 * Return:
 * 1 → 收到完整一行（\n）
 * 0 → 尚未完成
 */
int uart_read_line(char *buffer, int max_len) {
    while (uart_is_readable(UART_ID)) {
        char c = uart_getc(UART_ID);

        if (c == '\n') {
            rx_buffer[rx_index] = '\0';
            strncpy(buffer, rx_buffer, max_len);
            rx_index = 0;
            return 1;
        } else {
            if (rx_index < (int)sizeof(rx_buffer) - 1) {
                rx_buffer[rx_index++] = c;
            }
        }
    }

    return 0;
}
