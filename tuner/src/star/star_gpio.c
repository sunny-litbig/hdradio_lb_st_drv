#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define GPIO_CTRL_HIGH  1
#define GPIO_CTRL_LOW   0
#define GPIO_RSTN       227
#define TMP_BUF_MAX     128 

int gpio_export(void)
{
    int fd, len;
    char buf[TMP_BUF_MAX];

    fd = open("/sys/class/gpio/export", O_WRONLY);

    if (fd < 0)
    {
        printf("Can't open gpio export dir.\n");
        return -1;
    }

    len = snprintf(buf, sizeof(buf), "%d", GPIO_RSTN);
    write(fd, buf, len);
    close(fd);

    return 0;
}

int gpio_unexport(void)
{
    int fd, len;
    char buf[TMP_BUF_MAX];

    fd = open("/sys/class/gpio/unexport", O_WRONLY);

    if (fd < 0)
    {
        printf("Can't open gpio unexport dir.\n");
        return -1;
    }

    len = snprintf(buf, sizeof(buf), "%d", GPIO_RSTN);
    write(fd, buf, len);
    close(fd);

    return 0;
}

int gpio_set_out(void)
{
    int fd;
    char buf[TMP_BUF_MAX];

    snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/direction", GPIO_RSTN);

    fd = open(buf, O_WRONLY);

    if (fd < 0)
    {
        printf("Can't open gpio direction dir.\n");
        return -1;
    }

    write(fd, "out", 4);
    close(fd);

    return 0;
}

int gpio_set_value(int val)
{
    int fd, len;
    char buf[TMP_BUF_MAX];

    len = snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", GPIO_RSTN);

    fd = open(buf, O_WRONLY);

    if (fd < 0)
    {
        printf("Can't open gpio value dir.\n");
        return -1;
    }

    len = snprintf(buf, sizeof(buf), "%d", val);
    write(fd, buf, len);
    close(fd);

    return 0;
}

void star_tuner_reset(void)
{
    gpio_export();
    gpio_set_out();

    gpio_set_value(GPIO_CTRL_HIGH);
    usleep(1 * 1000);  // Keep boot configuration 1ms

    gpio_set_value(GPIO_CTRL_LOW);
    usleep(1 * 1000); // 1 ms  (T1, Treset)

    gpio_set_value(GPIO_CTRL_HIGH);
    usleep(2 * 1000);  // 2 ms  > Tlatch (10us) + Tboot(1ms)

    gpio_unexport();
}

