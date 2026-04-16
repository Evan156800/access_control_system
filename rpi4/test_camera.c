#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main()
{
    int fd = open("/dev/pir", O_RDONLY);
    if (fd < 0)
    {
        perror("open pir");
        return 1;
    }

    int camera_on = 0;
    char buf[2];

    while (1)
    {
        lseek(fd, 0, SEEK_SET);
        read(fd, buf, 1);
	printf("PIR=%C\n",buf[0]);
        if (buf[0] == '1' && camera_on == 0)
        {
            printf("Motion detected → Camera ON\n");
            system("DISPLAY=:0 ffplay -f v4l2 /dev/video0 &");
            camera_on = 1;
        }
        else if (buf[0] == '0' && camera_on == 1)
        {
            printf("No motion → Camera OFF\n");
            system("pkill ffplay");
            camera_on = 0;
        }

        usleep(500000); // 0.5 秒
    }

    close(fd);
    return 0;
}
