#ifndef PIR_H
#define PIR_H

#include "pico/stdlib.h"

void pir_init(uint pin);
int pir_read();

#endif
