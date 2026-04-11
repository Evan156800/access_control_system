#ifndef UART_H
#define UART_H

#include <stdint.h>

void uart_custom_init();
int uart_read_line(char *buffer, int max_len);

#endif
