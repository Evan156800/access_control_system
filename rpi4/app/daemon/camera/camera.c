#include <stdio.h>
#include <stdlib.h>

static int running = 0;

void camera_start()
{
    if (running) return;

    printf("Camera ON\n");
    system("DISPLAY=:0 ffplay -f v4l2 /dev/video0 &");
    running = 1;
}

void camera_stop()
{
    if (!running) return;

    printf("Camera OFF\n");
    system("pkill ffplay");
    running = 0;
}
