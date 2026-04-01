//測試用
#include <stdio.h>
#include <unistd.h>
#include "uart.h"

int main() {
    int fd = uart_init("/dev/serial0");

    char buffer[100];

    while (1) {
        int n = uart_read(fd, buffer, sizeof(buffer)-1);

        if (n > 0) {
            buffer[n] = '\0';
            printf("Receive: %s\n", buffer);

            // 回傳結果給 Pico
            uart_send(fd, "OK\n");
        }

        usleep(100000);
    }

    return 0;
}
