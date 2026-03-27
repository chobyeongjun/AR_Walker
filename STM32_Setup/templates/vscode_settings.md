# VS Code에서 STM32 프로젝트 사용하기

STM32CubeIDE가 메인 IDE이지만, VS Code로 코드 열람/간단 편집/디버깅이 가능합니다.

---

## 1. 필수 확장 설치

| 확장 | ID | 용도 |
|------|-----|------|
| C/C++ | `ms-vscode.cpptools` | IntelliSense, 코드 탐색 |
| Cortex-Debug | `marus25.cortex-debug` | STM32 디버깅 (ST-Link) |
| Arm Assembly | `dan-c-underwood.arm` | 어셈블리 구문 강조 |

```bash
code --install-extension ms-vscode.cpptools
code --install-extension marus25.cortex-debug
code --install-extension dan-c-underwood.arm
```

---

## 2. c_cpp_properties.json (IntelliSense 설정)

프로젝트 폴더에 `.vscode/c_cpp_properties.json`을 만듭니다.

```json
{
    "configurations": [
        {
            "name": "STM32",
            "includePath": [
                "${workspaceFolder}/Core/Inc",
                "${workspaceFolder}/Drivers/STM32H7xx_HAL_Driver/Inc",
                "${workspaceFolder}/Drivers/STM32H7xx_HAL_Driver/Inc/Legacy",
                "${workspaceFolder}/Drivers/CMSIS/Device/ST/STM32H7xx/Include",
                "${workspaceFolder}/Drivers/CMSIS/Include",
                "${workspaceFolder}/Middlewares/**"
            ],
            "defines": [
                "USE_HAL_DRIVER",
                "STM32H743xx"
            ],
            "compilerPath": "${userHome}/st/stm32cubeide_*/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.*/tools/bin/arm-none-eabi-gcc",
            "cStandard": "c11",
            "cppStandard": "c++17",
            "intelliSenseMode": "gcc-arm"
        }
    ],
    "version": 4
}
```

> **compilerPath**: STM32CubeIDE 설치 경로에 따라 다릅니다. 아래에서 OS별 경로를 확인하세요.

### OS별 컴파일러 경로

| OS | 경로 예시 |
|----|----------|
| **macOS** | `/Applications/STM32CubeIDE.app/Contents/Eclipse/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.*/tools/bin/arm-none-eabi-gcc` |
| **Windows** | `C:/ST/STM32CubeIDE_2.1.0/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.*/tools/bin/arm-none-eabi-gcc.exe` |
| **Linux** | `~/st/stm32cubeide_*/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.*/tools/bin/arm-none-eabi-gcc` |

---

## 3. launch.json (디버깅 설정)

`.vscode/launch.json`:

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug STM32 (OpenOCD)",
            "type": "cortex-debug",
            "request": "launch",
            "servertype": "openocd",
            "cwd": "${workspaceFolder}",
            "executable": "${workspaceFolder}/Debug/H-Walker_STM32_Test.elf",
            "device": "STM32H743VITx",
            "configFiles": [
                "interface/stlink.cfg",
                "target/stm32h7x.cfg"
            ],
            "runToEntryPoint": "main",
            "showDevDebugOutput": "none"
        },
        {
            "name": "Debug STM32 (ST-Util)",
            "type": "cortex-debug",
            "request": "launch",
            "servertype": "stutil",
            "cwd": "${workspaceFolder}",
            "executable": "${workspaceFolder}/Debug/H-Walker_STM32_Test.elf",
            "device": "STM32H743VITx",
            "runToEntryPoint": "main"
        }
    ]
}
```

### OpenOCD 설치

| OS | 설치 명령 |
|----|----------|
| **macOS** | `brew install open-ocd` |
| **Windows** | [xpack OpenOCD](https://xpack.github.io/openocd/install/) 다운로드 후 PATH에 추가 |
| **Linux** | `sudo apt install openocd` |

---

## 4. tasks.json (빌드 연동)

STM32CubeIDE의 headless 빌드를 VS Code에서 실행:

`.vscode/tasks.json`:

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "STM32 Build",
            "type": "shell",
            "command": "${config:stm32cubeide.path}/stm32cubeide",
            "args": [
                "--launcher.suppressErrors",
                "-nosplash",
                "-application", "org.eclipse.cdt.managedbuilder.core.headlessbuild",
                "-data", "${workspaceFolder}/..",
                "-build", "H-Walker_STM32_Test/Debug"
            ],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": "$gcc"
        },
        {
            "label": "STM32 Clean",
            "type": "shell",
            "command": "${config:stm32cubeide.path}/stm32cubeide",
            "args": [
                "--launcher.suppressErrors",
                "-nosplash",
                "-application", "org.eclipse.cdt.managedbuilder.core.headlessbuild",
                "-data", "${workspaceFolder}/..",
                "-cleanBuild", "H-Walker_STM32_Test/Debug"
            ],
            "problemMatcher": "$gcc"
        }
    ]
}
```

### VS Code settings.json에 CubeIDE 경로 추가

```json
{
    "stm32cubeide.path": "/Applications/STM32CubeIDE.app/Contents/MacOS"
}
```

---

## 5. 중요 참고사항

- `.vscode/` 폴더는 `.gitignore`에 포함되어 있으므로 **각 PC에서 개별 생성** 필요
- 이 문서의 설정을 복사하여 사용하면 됨
- **빌드와 디버깅은 STM32CubeIDE에서 하는 것을 권장** — VS Code는 코드 편집/열람 보조 용도
- VS Code에서 편집한 후 반드시 STM32CubeIDE에서 빌드 확인할 것
