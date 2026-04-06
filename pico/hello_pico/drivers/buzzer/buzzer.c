#include "buzzer.h"

static uint buzzer_pin;

void buzzer_init(uint pin) {
    buzzer_pin = pin;
    gpio_init(buzzer_pin);
    gpio_set_dir(buzzer_pin, GPIO_OUT);
}

void buzzer_on() {
    gpio_put(buzzer_pin, 1);
}

void buzzer_off() {
    gpio_put(buzzer_pin, 0);
}

void buzzer_beep(int duration_ms) {
    buzzer_on();
    sleep_ms(duration_ms);
    buzzer_off();
}
