#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "uart.h"
#include <stdlib.h>
#include <time.h>

int main() {
    int fd = uart_init("/dev/serial0");

    char buffer[100];
    int idx = 0;
    srand(time(NULL));
    while (1) {
        char c;

        int n = read(fd, &c, 1);

        if (n > 0) {
            if (c == '\n') {
                buffer[idx] = '\0';
                idx = 0;

                printf("Recv: %s\n", buffer);

                if (strcmp(buffer, "EVT:MOTION") == 0) {

                    // 回 PROCESSING
                    write(fd, "RES:PROCESSING\n", 15);

                    printf("Processing...\n");
                    sleep(2);

                    // 模擬辨識結果（先假資料）
                    int result = rand() % 2;

                    if (result == 0) {
                        write(fd, "RES:SUCCESS\n", 12);
                        printf("Send SUCCESS\n");
                    } else {
                        write(fd, "RES:FAIL\n", 9);
                        printf("Send FAIL\n");
                    }
                }

            } else {
                buffer[idx++] = c;
            }
        }
    }

    return 0;
}
