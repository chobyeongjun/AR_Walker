# STM32_Study

STM32 기반 보행 보조기(Walker) 제어 시스템 개발 및 학습 저장소입니다.

## 프로젝트 목록

| 프로젝트 | MCU | 설명 | 상태 |
|----------|-----|------|------|
| H-Walker_STM32_Test | STM32H743VITx | H-Walker 모터 제어 테스트 | 개발 중 |
| AR_Walker_STM32 | STM32H743VITx | AR_Walker 메인 제어 | 예정 |

## 개발 환경

| 항목 | 버전 |
|------|------|
| IDE | STM32CubeIDE 2.1.x |
| MCU | STM32H743VITx (LQFP-100) |
| 디버거 | ST-Link V2 / V3 |
| Git | 2.x 이상 |

## 빠른 시작

```bash
# 1. 클론
git clone https://github.com/chobyeongjun/STM32_Study.git
cd STM32_Study
```

**STM32CubeIDE에서 Import:**
1. **File** → **Import** → **General** → **Existing Projects into Workspace**
2. **Select root directory**: 프로젝트 하위 폴더 선택 (예: `H-Walker_STM32_Test/`)
3. **"Copy projects into workspace"** → **체크 해제** ⚠️
4. **Finish**

**빌드:**
1. **Project** → **Clean...** → **Clean all projects** → **Clean**
2. **Project** → **Build All** (또는 `Ctrl+B`)

## 멀티 PC 환경 설정

여러 PC에서 작업하는 방법은 [멀티 PC 설정 가이드](Documentation/Guides/MultiPC_Setup.md)를 참고하세요.

## 디렉토리 구조

```
STM32_Study/
├── Documentation/          ← 가이드, 핀맵, 회로도
├── H-Walker_STM32_Test/    ← STM32CubeIDE 프로젝트 #1
├── AR_Walker_STM32/        ← STM32CubeIDE 프로젝트 #2 (향후)
└── Shared/                 ← 공유 코드 (선택)
```

## 기여자

- [조병준](https://github.com/chobyeongjun)

## 라이선스

[라이선스 유형을 여기에 기입]
