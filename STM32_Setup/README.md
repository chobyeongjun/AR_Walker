# STM32 멀티 PC 개발 환경 구축 가이드

> **대상**: STM32H743VITx / STM32CubeIDE 2.1.x / Mac·Windows·Linux
> **프로젝트명**: H-Walker_STM32_Test

---

## 목차

1. [Git 저장소 초기 세팅](#1-git-저장소-초기-세팅)
2. [다른 PC에서 프로젝트 가져오기](#2-다른-pc에서-프로젝트-가져오기)
3. [일상 작업 워크플로우](#3-일상-작업-워크플로우)
4. [프로젝트 구조 권장안](#4-프로젝트-구조-권장안)
5. [추가 설정](#5-추가-설정)

---

## 1. Git 저장소 초기 세팅

### 1.1 GitHub에서 레포지토리 생성

1. [github.com/new](https://github.com/new) 접속
2. Repository name: `STM32_Study`
3. Public 선택
4. "Add a README file" 체크 해제 (로컬에서 올릴 것이므로)
5. **Create repository** 클릭

### 1.2 로컬 프로젝트를 Git 저장소로 초기화

STM32CubeIDE에서 프로젝트를 이미 만든 경우:

```bash
# 프로젝트 상위 폴더(모노레포 루트)로 이동
cd ~/STM32CubeIDE/workspace_2.1.1    # 또는 프로젝트가 있는 경로

# 모노레포 루트 폴더 만들기
mkdir STM32_Study
mv H-Walker_STM32_Test STM32_Study/   # 기존 프로젝트를 안으로 이동

cd STM32_Study
git init
```

### 1.3 .gitignore 설정

루트에 `.gitignore` 파일을 만듭니다. 이 저장소의 `STM32_Setup/.gitignore` 파일을 복사하세요:

```bash
cp <이 파일의 경로>/STM32_Setup/.gitignore .gitignore
```

또는 직접 만들기:

```gitignore
# Build output
Debug/
Release/
build/

# Compiled objects & binaries
*.o
*.d
*.elf
*.bin
*.hex
*.map
*.list
*.srec

# IDE metadata (user-specific)
.settings/
*.launch
.metadata/
RemoteSystemsTempFiles/

# CubeMX backup
*.ioc.bak

# OS-specific
.DS_Store
._*
Thumbs.db
desktop.ini
*~

# VS Code (personal)
.vscode/
*.swp
*.bak
*.tmp
```

> ⚠️ **절대 .gitignore에 넣지 마세요**: `.cproject`, `.project`, `*.ioc`, `STM32*.ld`

### 1.4 첫 번째 커밋 & 푸시

```bash
git add .
git commit -m "Initial commit: H-Walker_STM32_Test project"

git remote add origin https://github.com/chobyeongjun/STM32_Study.git
git branch -M main
git push -u origin main
```

---

## 2. 다른 PC에서 프로젝트 가져오기

### 2.1 사전 준비 (모든 OS 공통)

- Git 설치 확인: `git --version`
- STM32CubeIDE 2.1.x 설치 완료
- ST-Link 드라이버 설치 (OS별 아래 참고)

### 2.2 Git Clone

```bash
git clone https://github.com/chobyeongjun/STM32_Study.git
cd STM32_Study
```

### 2.3 STM32CubeIDE에서 Import

1. STM32CubeIDE 실행
2. **File** → **Import...**
3. **General** → **Existing Projects into Workspace** → **Next**
4. **Select root directory** → **Browse...** → 클론한 폴더 내 **프로젝트 폴더** 선택
   - 예: `~/STM32_Study/H-Walker_STM32_Test`
   - ⚠️ 레포 루트(`STM32_Study/`)가 아닌 **프로젝트 하위 폴더**를 선택!
5. ✅ 프로젝트가 목록에 나타나는지 확인
6. ⚠️ **"Copy projects into workspace" → 반드시 체크 해제!**
   - 체크하면 workspace에 복사본이 생겨서 Git 연동이 끊깁니다
7. **Finish** 클릭

### 2.4 Clean Build

Import 후 반드시 클린 빌드:

1. **Project** → **Clean...** → **Clean all projects** → **Clean**
2. **Project** → **Build All** (`Ctrl+B` / `Cmd+B`)
3. Console 탭에서 빌드 성공 확인

> 첫 빌드 시 인덱싱에 시간이 걸릴 수 있습니다. 완료될 때까지 기다리세요.

### 2.5 경로 의존성 문제 해결

`.cproject` 파일에 절대 경로가 박히는 경우가 있습니다.

**문제**: 다른 PC에서 빌드 에러 — "include path not found"

**해결법**:

1. **STM32CubeIDE 변수 사용** (권장):
   - 프로젝트 우클릭 → **Properties** → **C/C++ Build** → **Settings**
   - Include Path에 절대 경로 대신 변수 사용:
     - `${ProjDirPath}/Core/Inc`
     - `${ProjDirPath}/Drivers/...`

2. **CubeMX에서 코드 재생성**:
   - `.ioc` 파일 더블클릭 → CubeMX 에디터 열림
   - **Project** → **Generate Code** (Alt+K)
   - 경로가 현재 PC에 맞게 재설정됩니다

3. **.cproject 수동 수정** (최후의 수단):
   - 텍스트 에디터로 `.cproject` 열기
   - 절대 경로를 상대 경로로 변경

### 2.6 OS별 주의사항

#### macOS (M시리즈)

```bash
# ST-Link 드라이버 (Homebrew)
brew install stlink

# STM32CubeIDE 위치
# /Applications/STM32CubeIDE.app
```

- **Rosetta 2 불필요**: STM32CubeIDE 2.1.x는 Apple Silicon 네이티브 지원
- **보안 설정**: 처음 실행 시 `시스템 설정 > 개인 정보 보호 및 보안`에서 허용 필요
- **파일 시스템**: macOS는 기본적으로 대소문자 구분 안 함 (APFS). 파일명 대소문자 주의

#### Windows 11

```powershell
# Git for Windows (git-scm.com에서 설치)
# ST-Link USB 드라이버: STM32CubeIDE 설치 시 자동 포함

# STM32CubeIDE 위치
# C:\ST\STM32CubeIDE_2.1.0\
```

- **경로 길이**: Windows는 기본 260자 제한. 프로젝트 경로를 짧게 유지 (예: `C:\STM32\`)
- **줄바꿈 설정**: Git에서 CRLF 자동 변환 설정
  ```bash
  git config --global core.autocrlf true
  ```
- **ST-Link 드라이버**: 장치 관리자에서 "STMicroelectronics STLink dongle" 확인
- **방화벽/백신**: 빌드 시 arm-none-eabi-gcc가 차단될 수 있음 → 예외 추가

#### Linux (Ubuntu)

```bash
# 필수 패키지
sudo apt update
sudo apt install git build-essential libncurses5

# ST-Link udev 규칙 (디버거 접근 권한)
sudo cp /opt/st/stm32cubeide_*/plugins/com.st.stm32cube.ide.mcu.externaltools.stlink-gdb-server.*/tools/bin/native/linux_x64/stlink-server/config/udev/rules.d/*.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
sudo udevadm trigger

# 사용자를 dialout 그룹에 추가 (USB 접근)
sudo usermod -aG dialout $USER
# → 로그아웃 후 재로그인 필요!
```

- **권한 문제**: ST-Link 연결 시 "permission denied" → udev 규칙 및 dialout 그룹 확인
- **32비트 라이브러리**: 일부 툴체인은 32비트 의존성 필요
  ```bash
  sudo dpkg --add-architecture i386
  sudo apt install libc6:i386 libncurses5:i386 libstdc++6:i386
  ```

---

## 3. 일상 작업 워크플로우

### 3.1 PC를 옮길 때마다 하는 루틴

```
[PC-A에서 작업 마무리]
  1. STM32CubeIDE: File → Save All (Ctrl+Shift+S)
  2. 터미널:
     git add .
     git commit -m "작업 내용 설명"
     git push origin main

[PC-B에서 작업 시작]
  1. 터미널:
     git pull origin main
  2. STM32CubeIDE:
     - 프로젝트 우클릭 → Refresh (F5)
     - Project → Clean... → Clean
     - Project → Build All (Ctrl+B)
  3. 작업 시작
```

> **핵심**: push를 잊으면 다른 PC에서 최신 코드를 못 받습니다. 작업 끝나면 반드시 push!

### 3.2 충돌(Conflict) 발생 시

**언제 발생?**: 두 PC에서 같은 파일의 같은 부분을 수정한 경우 (push를 잊고 양쪽에서 작업)

**해결 절차:**

```bash
git pull origin main
# 충돌 발생 시:
# CONFLICT (content): Merge conflict in Core/Src/main.c
```

1. 충돌 파일을 텍스트 에디터로 열기
2. 충돌 마커 찾기:
   ```c
   <<<<<<< HEAD
   // 현재 PC의 코드
   =======
   // 다른 PC에서 push한 코드
   >>>>>>> origin/main
   ```
3. 원하는 코드만 남기고 마커(`<<<<<<<`, `=======`, `>>>>>>>`) 삭제
4. 저장 후:
   ```bash
   git add Core/Src/main.c
   git commit -m "Resolve merge conflict in main.c"
   git push origin main
   ```

> **팁**: 혼자 작업할 때는 항상 push하고 다른 PC에서 pull하면 충돌이 발생하지 않습니다.

### 3.3 .ioc 파일 (CubeMX 설정) 변경 시 주의사항

`.ioc` 파일은 STM32CubeMX의 핀/클럭/페리페럴 설정을 담고 있습니다.
이 파일은 **텍스트 merge가 사실상 불가능**합니다.

#### 규칙

1. **한 번에 한 PC에서만 .ioc 수정** — 수정 후 즉시 commit & push
2. **.ioc 수정 + 코드 재생성은 하나의 커밋으로**:
   ```bash
   # CubeMX에서 설정 변경 후 Generate Code 실행
   git add H-Walker_STM32_Test.ioc Core/ Drivers/
   git commit -m "Add FDCAN1 peripheral configuration"
   git push origin main
   ```
3. **다른 PC에서 pull 후**:
   - STM32CubeIDE에서 `.ioc` 더블클릭
   - CubeMX가 열리면 **Generate Code** 실행 (코드가 현재 환경에 맞게 재생성)
   - Clean Build 수행

#### .ioc 충돌 발생 시

**절대 텍스트 merge 하지 마세요!** 한쪽 버전을 선택하고 다시 설정:

```bash
# 원격(다른 PC) 버전 선택
git checkout --theirs H-Walker_STM32_Test.ioc
git add H-Walker_STM32_Test.ioc
git commit -m "Accept remote .ioc, will re-apply local changes"
git push origin main
# → CubeMX에서 필요한 설정을 다시 적용
```

### 3.4 자동 생성 코드와 유저 코드 관리

CubeMX가 코드를 재생성하면 `/* USER CODE */` 블록 **밖의 코드는 덮어쓰기**됩니다.

#### 안전한 코드 위치

```c
/* USER CODE BEGIN Includes */
#include "my_header.h"        // ✅ 여기는 안전!
/* USER CODE END Includes */

/* USER CODE BEGIN 0 */
void my_function(void) {      // ✅ 여기는 안전!
    // ...
}
/* USER CODE END 0 */

int main(void) {
    /* USER CODE BEGIN 1 */
    // ✅ 초기화 전 코드
    /* USER CODE END 1 */

    HAL_Init();
    SystemClock_Config();

    /* USER CODE BEGIN 2 */
    // ✅ 초기화 후, 메인 루프 전 코드
    my_function();
    /* USER CODE END 2 */

    while (1) {
        /* USER CODE BEGIN 3 */
        // ✅ 메인 루프 코드
        /* USER CODE END 3 */
    }
}
```

#### 위험한 코드 위치 (덮어쓰기됨)

```c
// ❌ USER CODE 블록 밖에 쓴 코드는 CubeMX 재생성 시 사라짐!
void will_be_deleted(void) { }    // ❌ 위험!

/* USER CODE BEGIN 0 */
void safe_function(void) { }      // ✅ 안전!
/* USER CODE END 0 */
```

#### 권장 패턴

| 코드 유형 | 위치 |
|-----------|------|
| 유저 함수 구현 | `Core/Src/` 에 **별도 .c 파일** 생성 (예: `motor_control.c`) |
| 유저 헤더 | `Core/Inc/` 에 **별도 .h 파일** 생성 (예: `motor_control.h`) |
| 인터럽트 콜백 | `Core/Src/stm32h7xx_it.c`의 USER CODE 블록 내부 |
| 메인 로직 | `Core/Src/main.c`의 USER CODE 블록 내부 |

> **핵심**: CubeMX가 건드리지 않는 **별도 파일**에 유저 코드를 작성하는 것이 가장 안전합니다.

### 3.5 커밋 메시지 컨벤션

```
<타입>: <변경 내용 설명>

타입:
  feat:     새 기능 추가
  fix:      버그 수정
  hw:       하드웨어 설정 변경 (.ioc)
  refactor: 코드 구조 변경 (기능 변경 없음)
  docs:     문서 변경
  test:     테스트 코드
```

예시:
```bash
git commit -m "hw: Add FDCAN1 configuration for motor CAN bus"
git commit -m "feat: Implement PID controller for knee joint"
git commit -m "fix: Correct ADC sampling rate calculation"
```

---

## 4. 프로젝트 구조 권장안

자세한 디렉토리 구조는 [project_structure.md](project_structure.md)를 참고하세요.

### 핵심 요약

```
STM32_Study/
├── .gitignore
├── README.md
├── Documentation/           ← 가이드, 핀맵, 회로도
├── H-Walker_STM32_Test/     ← STM32CubeIDE 프로젝트
│   ├── .cproject            ← Git 추적 필수
│   ├── .project             ← Git 추적 필수
│   ├── *.ioc                ← Git 추적 필수
│   ├── Core/Inc/            ← 유저 헤더
│   ├── Core/Src/            ← 유저 소스 + main.c
│   └── Drivers/             ← HAL/CMSIS (자동 생성)
└── Shared/                  ← 공유 코드 (선택)
```

### README.md 템플릿

[templates/README_template.md](templates/README_template.md) 파일을 STM32_Study 레포의 루트 README로 사용하세요.

### 하드웨어 문서화

핀맵, CAN 보드레이트, UART 설정 등은 [templates/hardware_pinmap_template.md](templates/hardware_pinmap_template.md) 형식으로 `Documentation/Hardware/` 폴더에 저장합니다.

문서화할 항목:
- **핀맵**: GPIO 할당, AF(Alternate Function) 번호
- **통신 설정**: CAN FD 보드레이트, UART 설정, SPI 모드
- **클럭 트리**: HSE 주파수, PLL 설정, 각 버스 클럭
- **전원**: 전압 레일, 소비 전류 예측
- **커넥터 핀아웃**: 보드의 물리적 커넥터 배치

---

## 5. 추가 설정

### 5.1 Workspace 설정 vs Project 설정

| 항목 | Workspace 설정 | Project 설정 |
|------|---------------|-------------|
| 위치 | `.metadata/` 폴더 | `.cproject`, `.project` 파일 |
| Git 추적 | ❌ 추적하지 않음 | ✅ 반드시 추적 |
| 내용 | 에디터 레이아웃, 폰트 크기, 열린 파일 목록 | 컴파일러 옵션, 인클루드 경로, 빌드 설정 |
| PC마다 | 달라도 됨 (개인 취향) | 같아야 함 (빌드에 영향) |

**Workspace 재설정이 필요할 때**:
- STM32CubeIDE가 이상하게 동작하면 workspace를 새로 만들어도 됩니다
- File → Switch Workspace → Other → 새 폴더 선택
- 프로젝트를 다시 Import하면 됩니다 (프로젝트 설정은 .cproject에 저장되어 있으므로 유지됨)

### 5.3 ST-Link 문제 대응

#### 펌웨어 버전 불일치

다른 PC에서 디버깅 시 "ST-Link firmware upgrade required" 메시지가 나올 수 있습니다.

**해결**:
1. STM32CubeIDE → **Help** → **ST-Link Upgrade**
2. **Open in update mode** → **Upgrade** 클릭
3. 업그레이드 완료 후 ST-Link 재연결

> **주의**: 모든 PC에서 같은 버전의 ST-Link 펌웨어를 사용하세요.

#### ST-Link 인식 안 될 때

| OS | 확인 사항 |
|----|----------|
| **macOS** | `brew install stlink` 후 `st-info --probe` 로 확인 |
| **Windows** | 장치 관리자 → "범용 직렬 버스 컨트롤러" → STLink 확인, 드라이버 재설치 |
| **Linux** | `lsusb | grep STMicro` 로 확인, udev 규칙 재설치, dialout 그룹 확인 |

#### 여러 ST-Link를 하나의 PC에 연결할 때

```
Run → Debug Configurations → Debugger 탭
→ ST-LINK S/N: [특정 시리얼 번호 입력]
```

시리얼 번호 확인:
```bash
# Linux/Mac
st-info --probe

# 또는 STM32CubeIDE의 ST-Link Upgrade 화면에서 확인
```

---

## 부록: 자주 묻는 질문

### Q: .ioc 파일만으로 프로젝트를 재생성할 수 있나요?

네. `.ioc` 파일에 모든 하드웨어 설정이 들어 있습니다:

1. STM32CubeIDE → **File** → **New** → **STM32 Project from an Existing STM32CubeMX Configuration File (.ioc)**
2. `.ioc` 파일 선택
3. 프로젝트 이름/위치 지정 → **Finish**
4. 코드가 자동 생성됨
5. `USER CODE BEGIN/END` 블록에 있던 유저 코드는 복원해야 함 (Git에서 복사)

### Q: 기존 프로젝트(Git 없음)를 Git으로 전환하려면?

```bash
cd ~/STM32CubeIDE/workspace_2.1.1/H-Walker_STM32_Test
git init
# .gitignore 파일 먼저 만들기 (위의 내용 복사)
git add .
git status    # Debug/, *.elf 등이 제외되었는지 확인
git commit -m "Initial commit"
git remote add origin https://github.com/chobyeongjun/STM32_Study.git
git push -u origin main
```

### Q: 두 프로젝트를 모노레포로 관리하려면?

[project_structure.md](project_structure.md)의 구조를 따르세요. 각 프로젝트가 독립 폴더:

```
STM32_Study/
├── H-Walker_STM32_Test/    ← Import 시 이 폴더 선택
└── AR_Walker_STM32/        ← Import 시 이 폴더 선택
```

STM32CubeIDE에서 두 프로젝트를 동시에 Import하여 같은 workspace에서 작업 가능합니다.
각각 독립적으로 빌드됩니다.
