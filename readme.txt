1) PROCESSOR, MACHINE 정의는 PERI와 HAL에서만 필요함.
2) --enalbe-pulseaudio 정의는 HAL의 dev_audio.c에서만 사용.
3) --enable-s0tuner --enable-x0tuner --enable-m0tuner 정의는 HAL의 tcradio_hal_config.c, tcradio_hal.c 에서 사용.
4) --enable-aarch64는 PERI만 제외하고 모두 사용 하지만, so 소스 위치만 참조 함. (lib or lib64, bin or bin64)
