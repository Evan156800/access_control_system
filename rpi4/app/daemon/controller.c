#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include "camera/camera.h"

#define UART_DEV "/dev/serial0"

// ================= PATH =================
#define RESULT_FILE "/home/pi/access_control_system/rpi4/app/web/result.txt"

// ================= STATE =================
typedef enum {
    STATE_IDLE,
    STATE_DETECT,
    STATE_RUN_AI,
    STATE_PASS,
    STATE_FAIL,
    STATE_COOLDOWN
} state_t;

// ================= TIME =================
long get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

// ================= PIR =================
int read_pir() {
    int fd = open("/dev/pir", O_RDONLY);
    if (fd < 0) {
        perror("open pir failed");
        return 0;
    }

    char buf[8] = {0};
    int n = read(fd, buf, sizeof(buf) - 1);
    close(fd);

    if (n <= 0) return 0;
    return atoi(buf);
}

// ================= SCREEN =================
void screen_on() {
    system("wlr-randr --output HDMI-A-1 --on");
    printf("SCREEN ON\n");
}

void screen_off() {
    system("wlr-randr --output HDMI-A-1 --off");
    printf("SCREEN OFF\n");
}

// ================= UART =================
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

void uart_send(int fd, const char *msg) {
    write(fd, msg, strlen(msg));
    write(fd, "\n", 1);
    printf("UART SEND: %s\n", msg);
}

// ================= READ RESULT =================
void read_face_result(char *out) {
    FILE *fp = fopen(RESULT_FILE, "r");
    if (!fp) {
        strcpy(out, "NONE");
        return;
    }

    fgets(out, 32, fp);
    fclose(fp);

    out[strcspn(out, "\n")] = 0;
}

// ================= ACTION =================
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

// ================= MAIN =================
int main() {
    printf("System Start (RPI4)\n");

    int uart_fd = uart_init();
    if (uart_fd < 0) return -1;

    state_t state = STATE_IDLE;
    state_t last_state = -1;

    long state_time = 0;
    long last_motion_time = 0;

    int display_on = 0;
    int last_pir = 0;

    while (1) {

        int pir_value = read_pir();

        if (pir_value == 1)
            last_motion_time = get_time_ms();

        // ===== timeout idle =====
        if (state != STATE_IDLE &&
            get_time_ms() - last_motion_time > 10000) {
            state = STATE_IDLE;
        }

        // ===== screen control =====
        /*if (state == STATE_IDLE) {
            if (display_on) {
                screen_off();
                display_on = 0;
            }
        } else {
            if (!display_on) {
                screen_on();
                display_on = 1;
            }
        }*/

        // ===== state change =====
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

                case STATE_PASS:
                    handle_pass(uart_fd);
                    state_time = get_time_ms();
                    break;

                case STATE_FAIL:
                    handle_fail(uart_fd);
                    state_time = get_time_ms();
                    break;

                case STATE_COOLDOWN:
                    printf("STATE: COOLDOWN\n");
                    state_time = get_time_ms();
                    break;
            }
        }

        // ===== FSM =====
        switch (state) {

            case STATE_IDLE:
                if (pir_value == 1 && last_pir == 0) {
                    state = STATE_DETECT;
                    printf("pir detected\n");
                }
                break;

            case STATE_DETECT:
                if (get_time_ms() - state_time > 1000) {
                    state = STATE_RUN_AI;
                }
                break;

            case STATE_RUN_AI:
            {
                printf("STATE: RUN_AI\n");

                system("/home/pi/access_control_system/rpi4/venv/bin/python "
                       "/home/pi/access_control_system/rpi4/ai/face_lbph/face_auth.py");

                char result[32];
                read_face_result(result);

                printf("FACE RESULT: %s\n", result);

                if (strcmp(result, "PASS") == 0)
                    state = STATE_PASS;
                else
                    state = STATE_FAIL;

                state_time = get_time_ms();
            }
            break;

            case STATE_PASS:
            case STATE_FAIL:
                if (get_time_ms() - state_time > 2000)
                    state = STATE_COOLDOWN;
                break;

            case STATE_COOLDOWN:
                if (get_time_ms() - state_time > 1000)
                    state = STATE_IDLE;
                break;
        }

        last_pir = pir_value;
        usleep(20000);
    }

    close(uart_fd);
    return 0;
}
