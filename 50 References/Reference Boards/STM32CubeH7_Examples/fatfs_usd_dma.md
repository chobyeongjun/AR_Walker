# FatFs uSD DMA Example (STM32H743)

**출처:** https://github.com/STMicroelectronics/STM32CubeH7/tree/master/Projects/STM32H743I-EVAL/Applications/FatFs/FatFs_uSD_DMA_Standalone

## 왜 보는가
- STM32H743 SDMMC 4-bit + DMA 실제 배선 확인
- SDMMC1 핀맵 (PC8~PC12, PD2) 레퍼런스
- FatFs 초기화 순서, SDMMC 클럭 설정

## 핵심 파일
- `Src/main.c` — SDMMC 초기화 흐름
- `Src/bsp_driver_sd.c` — 하드웨어 드라이버 레이어
- `Inc/stm32h7xx_hal_conf.h` — HAL 설정

## 관련
- [[Storage]] schematic block
- AN5200_SDMMC.pdf
