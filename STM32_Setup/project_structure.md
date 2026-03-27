# 프로젝트 구조 권장안

## 모노레포 디렉토리 구조

여러 STM32 프로젝트를 하나의 GitHub 레포지토리에서 관리하는 구조입니다.

```
STM32_Study/
├── .gitignore                          ← 루트 .gitignore (공통)
├── README.md                           ← 레포 전체 소개
│
├── Documentation/                      ← 프로젝트 공통 문서
│   ├── Guides/
│   │   ├── MultiPC_Setup.md            ← 멀티 PC 환경 구축 가이드
│   │   └── Daily_Workflow.md           ← 일상 작업 워크플로우
│   ├── Hardware/
│   │   ├── H-Walker_Pinmap.md          ← H-Walker 핀맵
│   │   └── AR_Walker_Pinmap.md         ← AR_Walker 핀맵
│   └── Figures/                        ← 다이어그램, 회로도 이미지
│
├── H-Walker_STM32_Test/                ← STM32CubeIDE 프로젝트 #1
│   ├── .cproject                       ← [Git 추적] CDT 빌드 설정
│   ├── .project                        ← [Git 추적] Eclipse 프로젝트 설정
│   ├── H-Walker_STM32_Test.ioc         ← [Git 추적] CubeMX 설정
│   ├── STM32H743VITX_FLASH.ld          ← [Git 추적] 링커 스크립트
│   ├── Core/
│   │   ├── Inc/                        ← 헤더 파일 (main.h, 유저 헤더)
│   │   └── Src/                        ← 소스 파일 (main.c, 유저 소스)
│   ├── Drivers/                        ← HAL, CMSIS (자동 생성)
│   │   ├── CMSIS/
│   │   └── STM32H7xx_HAL_Driver/
│   └── Middlewares/                    ← FreeRTOS, USB 등 (사용 시)
│
├── AR_Walker_STM32/                    ← STM32CubeIDE 프로젝트 #2 (향후)
│   ├── .cproject
│   ├── .project
│   ├── AR_Walker_STM32.ioc
│   └── ...
│
└── Shared/                             ← 프로젝트 간 공유 코드 (선택)
    ├── Inc/
    └── Src/
```

## 핵심 규칙

### 1. STM32CubeIDE Import 시 주의
- **Import 대상**: 레포 루트가 아니라 **프로젝트 하위 폴더** (예: `H-Walker_STM32_Test/`)를 선택
- **"Copy projects into workspace"**: 반드시 **체크 해제**

### 2. 반드시 Git 추적해야 하는 파일
| 파일 | 이유 |
|------|------|
| `.cproject` | CDT 빌드 설정 (컴파일러 옵션, 인클루드 경로) |
| `.project` | Eclipse 프로젝트 인식에 필수 |
| `*.ioc` | CubeMX 핀/클럭/페리페럴 설정 |
| `STM32*.ld` | 링커 스크립트 (메모리 맵) |

### 3. 절대 Git에 넣지 말아야 하는 것
| 항목 | 이유 |
|------|------|
| `Debug/`, `Release/` | 빌드 산출물 (PC마다 다름) |
| `.settings/` | 개인 IDE 설정 |
| `*.elf`, `*.bin`, `*.hex` | 바이너리 (재빌드 가능) |

### 4. Shared 코드 연결 방법
STM32CubeIDE에서 공유 코드를 사용하려면:
1. 프로젝트 우클릭 → **Properties** → **C/C++ General** → **Paths and Symbols**
2. **Includes** 탭에서 `../../Shared/Inc` 추가 (상대 경로)
3. **Source Location** 탭에서 **Link Folder** → `../../Shared/Src` 연결

> **주의**: 절대 경로 대신 **상대 경로**를 사용해야 다른 PC에서도 동작합니다.
