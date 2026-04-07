#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "drivers/ws2812/ws2812.h"
#include "drivers/sg90/sg90.h"
#include "drivers/buzzer/buzzer.h"
#include "drivers/pir/pir.h"

#define WS2812_PIN 2
#define PIR_PIN 14
#define SG90_PIN 15
#define BUZZER_PIN 16

// ===== 狀態定義 =====
typedef enum {
    STATE_IDLE,
    STATE_DETECT,
    STATE_WAIT_UART,
    STATE_PASS,
    STATE_FAIL
} state_t;

// ===== Fake UART（模擬 RPi4 回傳）=====
int fake_uart_receive(char *buffer) {
    static absolute_time_t start;
    static int waiting = 0;

    if (!waiting) {
        start = get_absolute_time();
        waiting = 1;
    }

    // 模擬 2 秒後回傳
    if (absolute_time_diff_us(start, get_absolute_time()) > 2000000) {

        // 可以改成 "fail" 測試
        sprintf(buffer, "{\"event\":\"result\",\"value\":\"pass\"}");

        waiting = 0;
        return 1;
    }

    return 0;
}

// ===== 動作 =====
void handle_idle() {
    ws2812_handle_event("{\"device\":\"pico\",\"event\":\"pir\",\"value\":0}");
    sg90_close(SG90_PIN);
    printf("STATE: IDLE\n");
}

void handle_detect() {
    ws2812_handle_event("{\"device\":\"pico\",\"event\":\"pir\",\"value\":1}");
    printf("STATE: DETECT\n");

    // 模擬送 UART
    printf("UART SEND: detect\n");
}

void handle_pass() {
    ws2812_handle_event("{\"device\":\"pico\",\"event\":\"pir\",\"value\":2}");
    buzzer_beep(200);
    sg90_open(SG90_PIN);
    printf("STATE: PASS\n");
}

void handle_fail() {
    ws2812_handle_event("{\"device\":\"pico\",\"event\":\"pir\",\"value\":3}");

    buzzer_beep(100);
    sleep_ms(100);
    buzzer_beep(100);

    printf("STATE: FAIL\n");
}

int main() {
    stdio_init_all();

    PIO pio = pio0;
    uint sm = 0;

    pir_init(PIR_PIN);
    ws2812_init(pio, sm, WS2812_PIN);
    sg90_init(SG90_PIN);
    buzzer_init(BUZZER_PIN);

    printf("System Start\n");

    state_t state = STATE_IDLE;
    state_t last_state = -1;

    int last_pir = 0;
    absolute_time_t state_time;

    char uart_buffer[128];

    while (true) {

        int pir_value = pir_read();

        // 狀態改變 → 執行動作
        if (state != last_state) {
            last_state = state;

            switch (state) {
                case STATE_IDLE:
                    handle_idle();
                    break;

                case STATE_DETECT:
                    handle_detect();
                    state_time = get_absolute_time();
                    break;

                case STATE_WAIT_UART:
                    printf("STATE: WAIT_UART\n");
                    break;

                case STATE_PASS:
                    handle_pass();
                    state_time = get_absolute_time();
                    break;

                case STATE_FAIL:
                    handle_fail();
                    state_time = get_absolute_time();
                    break;
            }
        }

        // State Machine
        switch (state) {

            case STATE_IDLE:
                if (pir_value == 1 && last_pir == 0) {
                    state = STATE_DETECT;
                }
                break;

            case STATE_DETECT:
                // 等 1 秒後送出 UART → 進 WAIT
                if (absolute_time_diff_us(state_time, get_absolute_time()) > 1000000) {
                    state = STATE_WAIT_UART;
                }
                break;

            case STATE_WAIT_UART:
                if (fake_uart_receive(uart_buffer)) {

                    printf("UART RECV: %s\n", uart_buffer);

                    if (strstr(uart_buffer, "pass")) {
                        state = STATE_PASS;
                    } else {
                        state = STATE_FAIL;
                    }
                }
                break;

            case STATE_PASS:
                if (absolute_time_diff_us(state_time, get_absolute_time()) > 3000000) {
                    sg90_close(SG90_PIN);
                    state = STATE_IDLE;
                }
                break;

            case STATE_FAIL:
                if (absolute_time_diff_us(state_time, get_absolute_time()) > 2000000) {
                    state = STATE_IDLE;
                }
                break;
        }

        last_pir = pir_value;
        sleep_ms(50);
    }
}
