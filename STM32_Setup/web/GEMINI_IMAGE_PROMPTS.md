# Gemini 이미지 생성 프롬프트 — TechBlips STM32 학습 가이드

> **스타일 공통**: 미니멀 테크 일러스트, 보라(#6c5ce7) 포인트, 깨끗한 배경
> **워터마크 회피**: 모든 프롬프트에 하단 우측 여백 지시 포함
> **프롬프트 언어**: English (Gemini 최적)

---

## 1. `stm32-hero.png` — 히어로 커버 (1200×630) ⭐필수

```
Create a wide banner illustration (1200x630 aspect ratio) for a tech blog article about STM32 microcontroller robot board development. Show a sleek, modern STM32 microcontroller chip rendered in an isometric 3D style at the center-left, with glowing purple (#6c5ce7) circuit traces emanating from it. On the right side, depict a stylized bipedal robot exoskeleton (walking assist device) with clean geometric lines. Connect the chip and robot with flowing data streams in purple and indigo (#818cf8) gradients. The background should be a subtle dark navy (#0b1120) with faint grid lines suggesting a PCB layout. Add small floating icons: gears, signal waves, and code brackets in semi-transparent purple. The overall mood is futuristic, professional, and educational. All visual elements must be placed in the center and upper 80% of the image. Leave the bottom-right corner (approximately 15% width x 10% height) completely empty with the same dark navy background color, containing no elements, text, or details whatsoever, to allow clean cropping.
```

---

## 2. `cortex-m7-block.png` — Cortex-M7 블록 다이어그램 (1장)

```
Create a clean, technical block diagram illustration of an ARM Cortex-M7 processor core architecture. Show the main functional blocks as rounded rectangles with purple (#6c5ce7) borders and light purple (#ede9fe) fills: "6-Stage Superscalar Pipeline" at the top center, "FPU (SP+DP)" and "DSP/SIMD" as twin blocks below it, "I-Cache 16KB" and "D-Cache 16KB" as paired blocks, "MPU (16 Regions)" below, and "AXI + AHB Bus Interface" at the bottom connecting everything. Use thin purple arrows to show data flow between blocks. Label each block with clean sans-serif text. The style should be flat, minimal, and modern — like a textbook diagram but with vibrant purple accents. Background should be pure white or very light gray (#fafbfc). Include a small "480 MHz" badge in the top-right area with a purple pill-shaped background. All visual elements must be centered and placed in the upper 85% of the image. Leave the bottom-right corner (15% x 12%) completely blank with matching background color for clean cropping.
```

---

## 3. `memory-map.png` — 메모리 맵 시각화 (1장)

```
Create a vertical memory map diagram for the STM32H743 microcontroller showing address ranges as stacked rectangular blocks. From top to bottom: "0xFFFF_FFFF — Cortex-M7 Internal" in gray, "0x6000_0000–0x9FFF_FFFF — External Memory (QSPI/FMC)" in light blue, "0x5000_0000 — APB/AHB Peripherals" in orange, "0x4000_0000 — APB Peripherals" in yellow, "0x3000_0000 — SRAM (D2: SRAM1/2/3, D3: SRAM4)" in green with sub-sections, "0x2400_0000 — AXI SRAM 512KB (D1)" in bright purple (#6c5ce7), "0x2000_0000 — DTCM-RAM 128KB" in indigo (#818cf8), "0x0800_0000 — Flash 2MB" in deep purple, "0x0000_0000 — ITCM-RAM 64KB" in lavender. Each block should have its address range on the left side and size on the right. Use a flat, clean style with subtle shadows between blocks. Background white. Add small icons: a lightning bolt next to TCM sections, a circular arrow next to SRAM. All content positioned in center and upper 85%. Leave bottom-right corner (15% x 12%) completely empty with white background for clean cropping.
```

---

## 4. `bus-domains.png` — D1/D2/D3 버스 도메인 (1장)

```
Create a technical architecture diagram showing the three power/clock domains of the STM32H7 series. Arrange three large rounded rectangles horizontally: "D1 Domain" on the left in deep purple (#6c5ce7) border with "Cortex-M7 Core", "AXI SRAM 512KB", "AXI Bus (64-bit)" labels inside; "D2 Domain" in the center with indigo (#818cf8) border containing "DMA1/DMA2", "SRAM1/2/3", "AHB Bus", "Most Peripherals"; "D3 Domain" on the right in lighter purple (#a29bfe) border with "BDMA", "SRAM4 64KB", "LPUART/I2C4/SPI6". Show interconnecting bridges between domains as double-headed arrows labeled "AHB Bridge". Below each domain, show clock frequencies: "D1: 240 MHz AHB", "D2: 120 MHz AHB", "D3: 120 MHz AHB". Use a flat minimal style with consistent spacing and clean sans-serif labels. Light gray (#fafbfc) background. All elements in center and top 85% of image. Bottom-right corner (15% x 12%) must be completely empty with matching background for clean cropping.
```

---

## 5. `lqfp100-pinout.png` — LQFP-100 핀아웃 ⭐필수

```
Create a top-down view technical pinout diagram of an LQFP-100 (14x14mm) microcontroller package. Draw a square chip body in dark gray at the center with 25 pins on each side (100 total), arranged as thin metallic lines extending outward from the chip body. Color-code pin groups: purple (#6c5ce7) for power pins (VDD/VSS), indigo (#818cf8) for Port A (PA0-PA15), teal for Port B (PB0-PB15), green for Port C (PC0-PC13), orange for Port D (PD0-PD2), and red for BOOT0/NRST. Label pin 1 with a small dot marker on the top-left corner. Show pin numbers (1-100) in tiny text along the outside. Add a small legend box in the top-right showing the color coding. The style should be precise and technical like a datasheet diagram but with modern clean aesthetics. White background. Mark key pins used for a robot board: CAN (PA11/PA12), UART, SPI with small star markers. All content in center and upper 85%. Bottom-right corner (15% x 12%) completely empty with white background for cropping.
```

---

## 6. `af-mux.png` — AF 멀티플렉서 개념도 (2장)

```
Create a conceptual diagram showing the STM32 GPIO Alternate Function (AF) multiplexer system. On the left, draw a single GPIO pin (e.g., "PA6") as a large circle with purple (#6c5ce7) border. From this circle, draw 16 branching lines going right, each connecting to a different function block: "AF0: TIM13_CH1" through "AF15: EVENTOUT", arranged as a vertical stack of small rounded rectangles. Highlight AF1 (TIM3_CH1), AF5 (SPI1_MISO), and AF9 (TIM12_CH1) in bright purple as "active selections" while others are in light gray. Add a multiplexer symbol (trapezoid shape) between the pin and the AF options, labeled "AFR Register (4 bits)". Show binary "0101" above the mux to indicate AF5 is selected, with an arrow pointing to the SPI1_MISO option. Clean flat style, white background, sans-serif labels. All visual elements centered and in upper 85%. Bottom-right corner (15% x 12%) must remain completely empty with white background for clean cropping.
```

---

## 7. `clock-tree.png` — 클럭 트리 플로차트 (3장)

```
Create a horizontal flowchart diagram showing the STM32H743 clock tree from oscillator sources to system buses. Start on the left with three source blocks: "HSE 8 MHz" (crystal icon, purple), "HSI 64 MHz" (chip icon, gray), "LSE 32.768 kHz" (small crystal, gray). HSE feeds into a large "PLL1" block (bright purple #6c5ce7 fill) which outputs three channels: "P: 480 MHz → SYSCLK", "Q: 48 MHz → USB/RNG", "R: reserved". SYSCLK arrow flows right through a divider "÷2" to "AHB Bus 240 MHz" (indigo #818cf8), then through another "÷2" to "APB1/APB2 120 MHz" and "APB3/APB4 120 MHz". Add "PLL2" and "PLL3" as smaller gray blocks below PLL1 for peripheral clocks. Show "HPRE", "D1PPRE", "D2PPRE" divider labels on the connecting arrows. Use rounded rectangle blocks with drop shadows, directional arrows, and clean typography. Light background (#fafbfc). All content in center and upper 85%. Bottom-right corner (15% x 12%) completely blank with matching background for cropping.
```

---

## 8. `gpio-modes.png` — GPIO 4모드 비교 (4장)

```
Create a 2x2 grid comparison diagram showing the four GPIO modes of STM32. Each quadrant contains a simplified circuit schematic: Top-left "Input Mode" shows a pin connected through Schmitt trigger to input data register with optional pull-up/pull-down resistors; Top-right "Output Mode" shows output data register driving through a push-pull or open-drain transistor pair to the pin; Bottom-left "Alternate Function" shows the AF mux selecting between peripheral outputs; Bottom-right "Analog Mode" shows direct connection to ADC/DAC with digital logic disconnected (shown as X mark). Each quadrant has a purple (#6c5ce7) header bar with white text for the mode name. Use simple transistor symbols (MOSFET), resistor symbols, and clean wiring. Add small labels: "VDD", "VSS", "IDR", "ODR", "AFR". Gray background (#f8f9fa) with white quadrant fills. Clean technical illustration style. All elements in center and upper 85%. Leave bottom-right corner (15% x 12%) completely empty with background color for clean cropping.
```

---

## 9. `push-pull-od.png` — Push-Pull vs Open-Drain 회로도 (4장)

```
Create a side-by-side comparison diagram of Push-Pull vs Open-Drain GPIO output configurations. Left side labeled "Push-Pull": show a PMOS transistor connected between VDD and the output pin, and an NMOS transistor between the output pin and VSS. When output=1, PMOS ON (highlighted purple #6c5ce7), NMOS OFF. When output=0, PMOS OFF, NMOS ON. Show voltage output swinging between VDD and VSS. Right side labeled "Open-Drain": show only NMOS transistor between output pin and VSS, with the PMOS replaced by an external pull-up resistor to VDD (or external voltage). When output=0, NMOS ON pulls low. When output=1 (high-Z), external pull-up pulls high. Include small waveform diagrams below each showing the output voltage. Use clean schematic style with purple highlights for active components. White background. Add a small note "Open-Drain allows level shifting to 5V!" in an indigo (#818cf8) callout box. All elements centered in upper 85%. Bottom-right corner (15% x 12%) empty with white background for cropping.
```

---

## 10. `can-topology.png` — CAN 버스 토폴로지 (5장)

```
Create a technical network topology diagram for a CAN (Controller Area Network) bus system used in a robot. Show a horizontal backbone bus line (thick purple #6c5ce7 line labeled "CAN-H" and a parallel indigo #818cf8 line labeled "CAN-L"). Connected to this bus via vertical stub lines, place 4-5 node blocks: "STM32 Main Board" (large, central, highlighted with purple glow), "Motor Driver L" (left), "Motor Driver R" (right), "Sensor Hub" (left-center), "BLE Module" (right-center). Each node shows a small CAN transceiver chip (SN65HVD230) symbol. At both ends of the bus, show 120Ω termination resistors as standard resistor symbols with "120Ω" labels. Add small data frame packets (represented as colored rectangles) traveling along the bus. Include bit rate label "500 Kbps" near the bus. Clean flat technical style, white background with subtle grid. All elements in center and upper 85%. Bottom-right corner (15% x 12%) completely empty with white background for clean cropping.
```

---

## 11. `cubemx-pinout.png` — CubeMX 핀 설정 화면 (6장)

```
Create an illustration that resembles the STM32CubeMX pin configuration interface. Show a top-down view of an LQFP-100 chip package at the center with pins arranged on all four sides. Each pin should be a small colored rectangle: green for GPIO configured pins, yellow for analog, blue for communication peripherals (UART/SPI/CAN), purple (#6c5ce7) for timers/PWM, gray for unused pins. On the left side, show a panel listing configured peripherals: "FDCAN1: PA11/PA12 ✓", "USART2: PD5/PD6 ✓", "SPI1: PA5/PB4/PB5 ✓", "TIM1: PE9/PE11/PE13/PE14 ✓", "ADC1: PC0/PA4/PC3/PC2 ✓". Use a modern UI design style with rounded corners, subtle shadows, and a light gray toolbar at the top. The overall look should mimic a professional IDE/tool interface but as a clean illustration. White/light gray background. All content in center and upper 85%. Bottom-right corner (15% x 12%) completely empty for clean cropping.
```

---

## 12. `cubemx-clock.png` — CubeMX 클럭 설정 화면 (6장)

```
Create an illustration resembling the STM32CubeMX Clock Configuration tab. Show a horizontal flow diagram from left to right: "HSE 8 MHz" source selector (radio button style, selected) → PLL configuration panel showing multipliers and dividers ("×120 /2" = 480 MHz) with input fields → Output frequencies branching to "SYSCLK: 480 MHz", "HCLK: 240 MHz", "APB1: 120 MHz", "APB2: 120 MHz". Each frequency node should be in a rounded input-field style box. Highlight the PLL path in purple (#6c5ce7) while inactive paths (HSI, CSI) are grayed out. Add warning icons (orange triangles) near any constraint violations. Include a top toolbar with "Clock Configuration" tab highlighted. The style should look like a modern software interface mockup — clean, precise, with subtle depth. Light background. All elements in center and upper 85%. Bottom-right corner (15% x 12%) must be completely empty with matching background color for cropping.
```

---

## 13. `pin-mapping.png` — Teensy→STM32 핀 매핑 비교 ⭐필수

```
Create a side-by-side comparison infographic showing pin mapping from Teensy 4.1 to STM32H743. On the left, draw a simplified Teensy 4.1 board (rectangular, green PCB style, with USB port at top) showing labeled pins: "Pin 22/23: CAN", "Pin 16: UART RX4", "Pin 11/10: SPI MOSI/CS", "Pin A16/A6: ADC", "Pin 28/29: Motor EN", "Pin 14/25/24: Status LED". On the right, draw a simplified STM32 LQFP-100 chip (square, dark gray) with corresponding mapped pins: "PA11/PA12: FDCAN1", "PD6: USART2 RX", "PB5/PA4: SPI1", "PC0/PA4: ADC1", "PE7/PE8: Motor EN", "PB0/PB1/PB2: Status LED". Connect matching pairs with curved purple (#6c5ce7) arrow lines flowing from left to right. Use a gradient from green (Teensy) to purple (STM32) on the arrows to show the migration direction. Add a title "Migration: Teensy 4.1 → STM32H743" at top. Clean flat infographic style, white background. All elements in center and upper 85%. Bottom-right corner (15% x 12%) completely empty with white background for clean cropping.
```

---

## 14. `bringup-flow.png` — 보드 브링업 9단계 플로차트 (8장)

```
Create a vertical flowchart showing 9 steps for board bring-up of an STM32 robot board. Each step is a rounded rectangle node connected by downward arrows. The steps from top to bottom: "1. Power Supply Check (3.3V/5V)" in red, "2. BOOT0 + SWD Connection" in orange, "3. Flash LED Blink Test" in yellow, "4. UART Debug Printf" in light green, "5. System Clock 480MHz Verify" in green, "6. CAN Bus Loopback Test" in teal, "7. SPI Sensor Communication" in blue, "8. PWM Motor Drive Test" in indigo (#818cf8), "9. Full Integration Test" in purple (#6c5ce7). Each node has a small icon on the left (power plug, probe, LED, terminal, clock, bus, chip, motor, robot). Use a progress bar or gradient effect getting more purple as steps advance, suggesting progression from basic to complete. Add small checkmark circles beside each step for a checklist feel. Clean modern flowchart style, white background with very subtle dot grid. All content positioned in center and upper 85%. Bottom-right corner (15% x 12%) completely blank with white background for clean cropping.
```

---

## 사용 방법

1. 위 프롬프트를 Gemini에 복사-붙여넣기
2. 생성된 이미지를 다운로드
3. 하단 우측 워터마크 영역 크롭 (프롬프트에서 이미 여백 확보됨)
4. `public/images/study/stm32/` 폴더에 해당 파일명으로 저장
5. 이미지 크기 최적화: 웹용 PNG, 폭 1200px 이하 권장

## 이미지 크기 가이드

| 파일명 | 권장 크기 | 비율 |
|--------|----------|------|
| stm32-hero.png | 1200×630 | 약 2:1 |
| 나머지 다이어그램 | 1000×700~800 | 약 4:3 |
| 핀아웃 (lqfp100) | 1000×1000 | 1:1 |
