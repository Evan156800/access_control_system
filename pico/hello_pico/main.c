#include "pico/stdlib.h"
#include "hardware/uart.h"

#define UART_ID uart0
#define BAUD_RATE 115200

#define UART_TX_PIN 0
#define UART_RX_PIN 1

int main() {
    stdio_init_all();

    uart_init(UART_ID, BAUD_RATE);

    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    while (1) {
        // 傳送訊息給 RPI4
        uart_puts(UART_ID, "MOTION\n");
	printf("Send MOTION\n");
        sleep_ms(2000);

        // 接收 RPI4 回應
        while (uart_is_readable(UART_ID)) {
            char c = uart_getc(UART_ID);
            printf("%c", c);
        }
    }
}
