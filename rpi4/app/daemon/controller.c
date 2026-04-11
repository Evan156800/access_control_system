#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <fcntl.h>

// ===== UART =====
#define UART_DEV "/dev/serial0"

// ===== PIR sysfs =====
#define PIR_PATH "/sys/bus/platform/devices/pir@17/pir"

// ===== 狀態 =====
typedef enum {
    STATE_IDLE,
    STATE_DETECT,
    STATE_WAIT_UART,
    STATE_PASS,
    STATE_FAIL
} state_t;

// ===== 時間工具 =====
long get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

// ===== 讀 PIR =====
int read_pir() {
    FILE *fp = fopen(PIR_PATH, "r");
    if (!fp) return 0;

    char buf[8];
    fgets(buf, sizeof(buf), fp);
    fclose(fp);

    return atoi(buf);
}

// ===== UART send =====
void uart_send(int fd, const char *msg) {
    write(fd, msg, strlen(msg));
    write(fd, "\n", 1);
    printf("UART SEND: %s\n", msg);
}

// ===== Fake UART receive（模擬辨識結果）=====
int fake_uart_receive(char *buffer) {
    static long start = 0;
    static int waiting = 0;

    if (!waiting) {
        start = get_time_ms();
        waiting = 1;
    }

    if (get_time_ms() - start > 2000) {
        sprintf(buffer, "{\"event\":\"result\",\"value\":\"pass\"}");
        waiting = 0;
        return 1;
    }

    return 0;
}

// ===== 動作 =====
void handle_idle() {
    printf("STATE: IDLE\n");
}

void handle_detect(int uart_fd) {
    printf("STATE: DETECT\n");
    uart_send(uart_fd, "DETECT");
}

void handle_pass(int uart_fd) {
    printf("STATE: PASS\n");
    uart_send(uart_fd, "PASS");
}

void handle_fail(int uart_fd) {
    printf("STATE: FAIL\n");
    uart_send(uart_fd, "FAIL");
}

// ===== 主程式 =====
int main() {
    printf("System Start (RPI4)\n");

    int uart_fd = open(UART_DEV, O_RDWR | O_NOCTTY);
    if (uart_fd < 0) {
        perror("UART open failed");
        return -1;
    }

    state_t state = STATE_IDLE;
    state_t last_state = -1;

    int last_pir = 0;
    long state_time = 0;

    char uart_buffer[128];

    while (1) {

        int pir_value = read_pir();

        // ===== 狀態變化 =====
        if (state != last_state) {
            last_state = state;

            switch (state) {
                case STATE_IDLE:
                    handle_idle();
                    break;

                case STATE_DETECT:
                    handle_detect(uart_fd);
                    state_time = get_time_ms();
                    break;

                case STATE_WAIT_UART:
                    printf("STATE: WAIT_UART\n");
                    break;

                case STATE_PASS:
                    handle_pass(uart_fd);
                    state_time = get_time_ms();
                    break;

                case STATE_FAIL:
                    handle_fail(uart_fd);
                    state_time = get_time_ms();
                    break;
            }
        }

        // ===== State Machine =====
        switch (state) {

            case STATE_IDLE:
                if (pir_value == 1 && last_pir == 0) {
                    state = STATE_DETECT;
                }
                break;

            case STATE_DETECT:
                if (get_time_ms() - state_time > 1000) {
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
                if (get_time_ms() - state_time > 3000) {
                    state = STATE_IDLE;
                }
                break;

            case STATE_FAIL:
                if (get_time_ms() - state_time > 2000) {
                    state = STATE_IDLE;
                }
                break;
        }

        last_pir = pir_value;
        usleep(50000);
    }

    close(uart_fd);
    return 0;
}
