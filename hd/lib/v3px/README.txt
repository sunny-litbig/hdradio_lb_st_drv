- gcc linaro 7.2
- hard fp

CXX=arm-telechips-linux-gnueabi-g++  -mfpu=neon-fp-armv8 -mfloat-abi=hard -mcpu=cortex-a53 -mtune=cortex-a53
CC=arm-telechips-linux-gnueabi-gcc  -mfpu=neon-fp-armv8 -mfloat-abi=hard -mcpu=cortex-a53 -mtune=cortex-a53