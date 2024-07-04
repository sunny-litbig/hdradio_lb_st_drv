- gcc linaro 7.2
- soft fp

CXX=arm-telechips-linux-gnueabi-g++  -march=armv7-a -mfloat-abi=softfp -mfpu=neon -mtune=cortex-a7
CC=arm-telechips-linux-gnueabi-gcc  -march=armv7-a -mfloat-abi=softfp -mfpu=neon -mtune=cortex-a7