#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>

int uart_init(const char *device) {
    int fd = open(device, O_RDWR | O_NOCTTY);

    struct termios options;
    tcgetattr(fd, &options);

    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);

    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    tcsetattr(fd, TCSANOW, &options);

    return fd;
}

int uart_send(int fd, const char *msg) {
    return write(fd, msg, strlen(msg));
}

int uart_read(int fd, char *buffer, int size) {
    return read(fd, buffer, size);
}
