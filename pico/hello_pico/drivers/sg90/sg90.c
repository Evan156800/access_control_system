#include "sg90.h"
#include "hardware/pwm.h"

static uint slice_num;

void sg90_init(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);

    slice_num = pwm_gpio_to_slice_num(pin);

    pwm_config config = pwm_get_default_config();

    // 設定 PWM 頻率為 50Hz
    pwm_config_set_clkdiv(&config, 125.0f); 
    pwm_config_set_wrap(&config, 20000);    // 20ms

    pwm_init(slice_num, &config, true);
}

void sg90_set_angle(uint pin, float angle) {
    // 0° → 500
    // 180° → 2500
    float duty = 500 + (angle / 180.0f) * 2000;

    pwm_set_gpio_level(pin, (uint16_t)duty);
}

void sg90_open(uint pin) {
    sg90_set_angle(pin, 90);  // 開門角度
}

void sg90_close(uint pin) {
    sg90_set_angle(pin, 0);   // 關門
}
