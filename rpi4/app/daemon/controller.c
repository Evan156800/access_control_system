#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

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

// ===== 時間 =====
long get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

// ===== PIR =====
int read_pir() {
    int fd = open("/dev/pir", O_RDONLY);
    if (fd < 0) {
        perror("open pir failed");
        return 0;
    }

    char buf[8] = {0};
    read(fd, buf, sizeof(buf));
    close(fd);

    return atoi(buf);

    /*
    FILE *fp = fopen(PIR_PATH, "r");
    if (!fp) return 0;
    char buf[8];
    fgets(buf, sizeof(buf), fp);
    fclose(fp);
    return atoi(buf);
    */
}

// ===== UART INIT =====
int uart_init() {
    int fd = open(UART_DEV, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        perror("UART open failed");
        return -1;
    }

    struct termios options;
    tcgetattr(fd, &options);

    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);

    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    options.c_lflag = 0;
    options.c_iflag = 0;
    options.c_oflag = 0;

    tcsetattr(fd, TCSANOW, &options);

    return fd;
}

// ===== UART SEND =====
void uart_send(int fd, const char *msg) {
    write(fd, msg, strlen(msg));
    write(fd, "\n", 1);
    printf("UART SEND: %s\n", msg);
}

// ===== UART RECEIVE（非阻塞）=====
int uart_receive(int fd, char *buffer) {
    static int idx = 0;
    char c;

    int n = read(fd, &c, 1);
    if (n > 0) {
        if (c == '\r') return 0;

        if (c == '\n') {
            buffer[idx] = '\0';
            idx = 0;
            return 1;
        }

        if (idx < 127) {
            buffer[idx++] = c;
        }
    }

    return 0;
}

// ===== 動作 =====
void handle_idle() {
    printf("STATE: IDLE\n");
}

void handle_detect(int uart_fd) {
    printf("STATE: DETECT\n");
    uart_send(uart_fd, "detect");
}

void handle_pass(int uart_fd) {
    printf("STATE: PASS\n");
    uart_send(uart_fd, "pass");
}

void handle_fail(int uart_fd) {
    printf("STATE: FAIL\n");
    uart_send(uart_fd, "fail");
}

// ===== MAIN =====
int main() {
    printf("System Start (RPI4)\n");

    int uart_fd = uart_init();
    if (uart_fd < 0) return -1;

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

        // ===== STATE MACHINE =====
        switch (state) {
            case STATE_IDLE:
                //state = STATE_WAIT_UART;
                if (pir_value == 1 && last_pir == 0) {
                    state = STATE_DETECT;
                    printf("pir detected\n");
                }
                break;

            case STATE_DETECT:
                if (get_time_ms() - state_time > 1000) {
                    state = STATE_WAIT_UART;
                }
                break;

            case STATE_WAIT_UART:
                if (uart_receive(uart_fd, uart_buffer)) {
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
        usleep(20000);
    }

    close(uart_fd);
    return 0;
}
