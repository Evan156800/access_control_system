#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define DEVICE "/dev/pir"

int main() {
    int fd;
    char value;

    // 1. 開啟 device
    fd = open(DEVICE, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open /dev/pir");
        return -1;
    }

    printf("PIR test start...\n");

    // 2. 持續讀取
    while (1) {
        int ret = read(fd, &value, 1);

        if (ret < 0) {
            perror("Read error");
            break;
        }

        if (value == '1') {
            printf("[PIR] Motion detected\n");
        } else if (value == '0') {
            printf("[PIR] No motion\n");
        } else {
            printf("[PIR] Unknown value: %c\n", value);
        }

        usleep(500000); // 0.5 秒
    }

    // 3. 關閉
    close(fd);
    return 0;
}
