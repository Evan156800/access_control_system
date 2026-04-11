#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/pio.h"

#include "drivers/ws2812/ws2812.h"
#include "drivers/sg90/sg90.h"
#include "drivers/buzzer/buzzer.h"
#include "drivers/pir/pir.h"

// ================= PIN =================
#define WS2812_PIN 2
#define PIR_PIN 14
#define SG90_PIN 15
#define BUZZER_PIN 16

// ================= UART =================
#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

// ================= STATE =================
typedef enum {
    STATE_IDLE,
    STATE_DETECT,
    STATE_WAIT_UART,
    STATE_PASS,
    STATE_FAIL
} state_t;

static state_t state = STATE_IDLE;
static state_t last_state = -1;
static absolute_time_t state_time;

// ================= UART BUFFER =================
#define BUF_SIZE 128
static char uart_buf[BUF_SIZE];
static int uart_idx = 0;

// ================= UART INIT =================
void uart_init_custom() {
    uart_init(UART_ID, BAUD_RATE);

    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_fifo_enabled(UART_ID, true);
}

// ================= UART PARSER =================
void uart_read_nonblock() {

    while (uart_is_readable(UART_ID)) {

        char c = uart_getc(UART_ID);
	printf("BYTE=%02X\n",(uint8_t)c);
        // ignore CR
        if (c == '\r') continue;

        // line end
        if (c == '\n') {

            uart_buf[uart_idx] = '\0';
	    
	    if(uart_idx > 0){
            	
		printf("[UART] %s\n", uart_buf);

            	// ===== parsing logic =====
            	if (strstr(uart_buf, "pass")) {
               	    state = STATE_PASS;
            	}
            	else if (strstr(uart_buf, "fail")) {
            	    state = STATE_FAIL;
            	}
	    }
            uart_idx = 0;
            memset(uart_buf, 0, BUF_SIZE);
        }
        else {
            if (uart_idx < BUF_SIZE - 1) {
                uart_buf[uart_idx++] = c;
            }
        }
    }
}

// ================= MAIN =================
int main() {

    stdio_init_all();
    sleep_ms(1500);

    setvbuf(stdout, NULL, _IONBF, 0);

    printf("System Start\n");

    // init drivers
    pir_init(PIR_PIN);
    ws2812_init(pio0, 0, WS2812_PIN);
    sg90_init(SG90_PIN);
    buzzer_init(BUZZER_PIN);
    uart_init_custom();

    int pir_last = 0;

    while (true) {

        int pir = pir_read();

        // ================= STATE CHANGE =================
        if (state != last_state) {
            last_state = state;

            switch (state) {

                case STATE_IDLE:
                    printf("STATE: IDLE\n");
                    sg90_close(SG90_PIN);
                    break;

                case STATE_DETECT:
                    printf("STATE: DETECT\n");
                    state_time = get_absolute_time();
                    break;

                case STATE_WAIT_UART:
                    printf("STATE: WAIT UART\n");
                    break;

                case STATE_PASS:
                    printf("STATE: PASS\n");
                    sg90_open(SG90_PIN);
                    buzzer_beep(200);
                    state_time = get_absolute_time();
                    break;

                case STATE_FAIL:
                    printf("STATE: FAIL\n");
                    buzzer_beep(100);
                    sleep_ms(100);
                    buzzer_beep(100);
                    state_time = get_absolute_time();
                    break;
            }
        }

        // ================= STATE LOGIC =================
        switch (state) {

            case STATE_IDLE:
                //if (pir == 1 && pir_last == 0) {
                    state = STATE_WAIT_UART;
		    //state = STATE_DETECT;
		//}
                break;

            case STATE_DETECT:
                if (absolute_time_diff_us(state_time, get_absolute_time()) > 1000000) {
                    state = STATE_WAIT_UART;
                }
                break;

            case STATE_WAIT_UART:
                uart_read_nonblock();
                break;

            case STATE_PASS:
                if (absolute_time_diff_us(state_time, get_absolute_time()) > 3000000) {
                    state = STATE_IDLE;
                }
                break;

            case STATE_FAIL:
                if (absolute_time_diff_us(state_time, get_absolute_time()) > 2000000) {
                    state = STATE_IDLE;
                }
                break;
        }

        pir_last = pir;
        sleep_ms(20);
    }
}
