- gcc linaro 7.2
- hard fp

CXX=aarch64-telechips-linux-g++  -march=armv8-a+simd -mcpu=cortex-a53 -mtune=cortex-a53 -mfix-cortex-a53-843419 -mfix-cortex-a53-835769
CC=aarch64-telechips-linux-gcc  -march=armv8-a+simd -mcpu=cortex-a53 -mtune=cortex-a53 -mfix-cortex-a53-843419 -mfix-cortex-a53-835769