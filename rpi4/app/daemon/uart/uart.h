#ifndef UART_H
#define UART_H

int uart_init(const char *device);
int uart_send(int fd, const char *msg);
int uart_read(int fd, char *buffer, int size);

#endif
