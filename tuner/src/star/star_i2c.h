#ifndef H_STAR_I2C

#define I2C_FILENAME "/dev/i2c-4"

int star_i2c_open(void);
int star_i2c_close(void);
int star_i2c_write(unsigned char chip_addr, unsigned char *wr_buf, unsigned short wr_size);
int star_i2c_read(unsigned char chip_addr, unsigned char *wr_buf, unsigned short wr_size, unsigned char *rd_buf, unsigned short rd_size);
int star_i2c_read_byte_only(int chip_addr, unsigned char *rd_buf, unsigned short rd_size);

#endif // H_STAR_I2C
