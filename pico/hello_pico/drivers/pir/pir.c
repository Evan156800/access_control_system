#include "pir.h"

static uint pir_pin;

void pir_init(uint pin) {
    pir_pin = pin;
    gpio_init(pir_pin);
    gpio_set_dir(pir_pin, GPIO_IN);
}

int pir_read() {
    return gpio_get(pir_pin);
}
