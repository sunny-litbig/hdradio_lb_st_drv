#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "star_i2c.h"

int fd_star_i2c;

int star_i2c_open(void)
{
    if (fd_star_i2c > 0)
    {
        printf("[%s] I2C device is alread opend. fd = %d\n", __func__, fd_star_i2c);
        return -1;
    }

    fd_star_i2c = -1;

    fd_star_i2c = open(I2C_FILENAME, O_RDWR);

    if (fd_star_i2c < 0)
    {
        printf("[%s] Failed to open I2C device. fd = %d\n", __func__, fd_star_i2c);
        return -1;
    }

    return 0;
}

int star_i2c_close(void)
{
    int ret = 0;

    ret = close(fd_star_i2c);

    if (ret < 0)
    {
        printf("[%s] Failed to close I2C device. fd = %d\n", __func__, fd_star_i2c);
        return -1;
    }

    fd_star_i2c = -1;

    return 0;
}

int star_i2c_write(unsigned char chip_addr, unsigned char *wr_buf, unsigned short wr_size)
{
    struct i2c_rdwr_ioctl_data data;
    struct i2c_msg messages[1];
    int status;

    messages[0].addr = chip_addr;
    messages[0].flags = 0;
    messages[0].len = wr_size;
    messages[0].buf = wr_buf;

    data.msgs = messages;
    data.nmsgs = 1;

    status = ioctl(fd_star_i2c, I2C_RDWR,  &data);

    if ( status < 0 )
    {
        printf("errno = %d\n", errno);
        perror("i2c write eror occurred");
        printf("[%s] i2c fail (%d).\n",__func__, status);
        return -1;
    }

    return 0;
}

int star_i2c_read(unsigned char chip_addr, unsigned char *wr_buf, unsigned short wr_size, unsigned char *rd_buf, unsigned short rd_size)
{
    struct i2c_rdwr_ioctl_data data;
    struct i2c_msg messages[2];
    int status;

    messages[0].addr = chip_addr;
    messages[0].flags = 0;
    messages[0].len = wr_size;
    messages[0].buf = wr_buf;

    messages[1].addr = chip_addr;
    messages[1].flags = I2C_M_RD;
    messages[1].len = rd_size;
    messages[1].buf = rd_buf;

    data.msgs = messages;
    data.nmsgs = 2;

    status = ioctl(fd_star_i2c, I2C_RDWR,  &data);

    if ( status < 0 )
    {
        printf("[%s] i2c fail (%d).\n",__func__, status);
        return -1;
    }

    return 0;
}

int star_i2c_read_byte_only(int chip_addr, unsigned char *rd_buf, unsigned short rd_size)
{
    struct i2c_rdwr_ioctl_data data;
    struct i2c_msg messages[1];
    int status;

    messages[0].addr = chip_addr;
    messages[0].flags = I2C_M_RD;
    messages[0].len = rd_size;
    messages[0].buf = rd_buf;

    data.msgs = messages;
    data.nmsgs = 1;

    status = ioctl(fd_star_i2c, I2C_RDWR,  &data);

    if ( status < 0 )
    {
        printf("[%s] i2c fail (%d).\n",__func__, status);
        return -1;
    }

    return 0;
}
