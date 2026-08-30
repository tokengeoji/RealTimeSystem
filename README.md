# OMAP-L138 Real-Time Audio Processing

TI OMAP-L138 LCDK/EVM의 C6740 DSP에서 TLV320AIC3106 오디오 코덱 입력을 받아 실시간으로 처리하는 Code Composer Studio 프로젝트 모음이다. 각 LAB는 독립적으로 import/build할 수 있는 CCS 프로젝트이며, 최종 오디오 처리 경로는 `LAB7`에 구현되어 있다.

버전별 디렉터리 재구성과 코드 차이는 [LAB variant comparison](docs/lab-variants.md)에 정리했다.

## Start here

- [Final audio pipeline](labs/LAB7/)
- [LAB variant comparison](docs/lab-variants.md)

## Overview

최종 `labs/LAB7`은 다음 데이터 경로를 사용한다.

```text
LINE_IN
  -> TLV320AIC3106 ADC
  -> McASP0 RBUF14
  -> EDMA3 RX
  -> input ping/pong buffer
  -> EDMA Hwi / SYS/BIOS Swi
  -> output ping/pong buffer
  -> EDMA3 TX
  -> McASP0 XBUF13
  -> TLV320AIC3106 DAC
```

- 48 kHz, 16-bit, stereo PCM
- 한 블록 400 samples, 약 8.33 ms
- 샘플 1개를 `[left 16-bit][right 16-bit]`의 32-bit word로 처리
- EDMA3가 다음 블록을 전송하는 동안 SYS/BIOS Swi가 이전 블록을 처리
- 버튼 입력으로 talk-through와 2 Hz 진폭 변조 모드 전환

## Tech stack

| Layer | Technology | Applied implementation | Evidence |
|---|---|---|---|
| Hardware | TI OMAP-L138 LCDK/EVM, C6740 DSP | DSP에서 오디오 데이터와 주변장치 레지스터를 처리 | `labs/LAB7/.ccsproject`, `labs/LAB7/.cproject` |
| Audio codec | TLV320AIC3106, I2C0 (`I2C_Init(400)`) | I2C 레지스터 설정, LINE_IN ADC와 DAC 경로 구성 | `labs/LAB7/lib/Codec.c`, `labs/LAB7/lib/I2C.c` |
| Language | C99 | 메모리 매핑 레지스터, 인터럽트 핸들러, DSP 신호처리 구현 | `labs/LAB7/.cproject`, `labs/LAB7/*.c` |
| IDE/build | Code Composer Studio 9.3.0 | CCS 프로젝트 import/build/debug | `labs/LAB7/.ccsproject` |
| Compiler/target | TI C6000 Compiler 8.3.5, `-mv6740` | C6740용 ELF 바이너리 생성 | `labs/LAB7/.cproject` |
| RTOS | TI SYS/BIOS 6.76.3.01 | Hwi, Swi, Clock, Idle, Task 실행 단위 구성 | `labs/LAB7/app.cfg` |
| RTOS/XDC | XDCtools 3.60.2.34_core, `ti.targets.elf.C674`, `ti.platforms.evmOMAPL138` | SYS/BIOS configuration과 target/platform 생성 | `labs/LAB7/.cproject`, `labs/LAB7/app.cfg` |
| SDK/API | OMAP-L138 PDK 1.0.11, CSL | 보드 레지스터 정의와 저수준 peripheral access | `labs/LAB7/.cproject`, `labs/LAB7/lib/*.h` |
| Peripheral drivers | SYSCFG/PINMUX, GPIO, I2C, McASP0, Timer, DSP interrupt controller | 버튼·LED·codec·오디오 직렬 포트 초기화와 interrupt 연결 | `labs/LAB7/lib/*.c` |
| DMA | EDMA3 | McASP RX/TX와 메모리 버퍼 연결, PARAMSET·transfer complete interrupt | `labs/LAB7/lib/EDMA_McASP.c`, `labs/LAB7/HWI.c` |
| Audio format | 48 kHz, 16-bit, stereo PCM | `[left 16-bit][right 16-bit]` 32-bit word와 400-sample block 처리 | `labs/LAB7/define.h`, `labs/LAB7/SWI.c` |
| Buffering | Ping/pong double buffering | EDMA 전송과 Swi 신호처리를 겹쳐 실행 | `labs/LAB7/HWI.c`, `labs/LAB7/SWI.c` |
| DSP processing | sine generation, 2 Hz amplitude modulation, talk-through | 좌·우 샘플을 분리해 처리 후 32-bit word로 재결합 | `labs/LAB7/sinef.c`, `labs/LAB7/SWI.c` |
| Memory/linker | little-endian, EABI, ELF, TI linker command file | 코드/데이터 메모리 배치와 `.image` section 지정 | `labs/LAB7/.cproject`, `labs/LAB6/LAB6B/mylinker.cmd` |
| Debug support | JTAG debug, GEL script | 타깃 디버깅과 LAB3A 실험 보조 | `labs/LAB3/LAB3A/my_gel.gel` |

주변장치는 CSL/PDK 헤더와 메모리 매핑 레지스터를 이용해 초기화한다. 저장소에서 실제로 확인되는 기술만 기록했으며, Linux·FreeRTOS·FPGA·FFT 라이브러리 등은 사용 기술로 기재하지 않았다.

## Real-time implementation

`labs/LAB7`의 실행 순서는 다음과 같다.

1. PINMUX와 GPIO를 초기화한다.
2. I2C로 TLV320AIC3106을 48 kHz/16-bit/LINE_IN 경로로 설정한다.
3. McASP0을 32-bit 전송 단위, 1 slot으로 설정한다.
4. EDMA RX/TX PARAMSET을 ping/pong 버퍼에 연결한다.
5. EDMA 완료 Hwi가 다음 버퍼 주소를 교체하고 `Swi_or`로 처리할 버퍼를 전달한다.
6. Swi가 원음 복사 또는 좌·우 샘플의 2 Hz 진폭 변조를 수행한다.
7. GPIO interrupt와 Timer1으로 버튼 입력을 500 ms 디바운싱한다.

Hwi에서는 전송 완료와 버퍼 교체를 처리하고, CPU 연산은 Swi로 분리했다. 단일 버퍼 크기는 `400 * 4 = 1,600 bytes`이며 입·출력 ping/pong 네 개는 총 6,400 bytes다.

## Repository layout

```text
labs/
├─ LAB0/                      # C6740 시작 코드와 linker 확인
├─ LAB1/                      # GPIO/LED polling; LAB1A는 main.c의 compile-time branch
├─ LAB2/                      # GPIO interrupt 기본 버전
│  ├─ LAB2A/                  # Timer interrupt
│  ├─ LAB2B/                  # 두 개의 timer interrupt
│  └─ LAB2C/                  # timer 기반 button debounce
├─ LAB3/                      # sine generation 기본 버전
│  └─ LAB3A/                  # 반복 생성과 GEL 파일
├─ LAB4/                      # SYS/BIOS 기본 버전
│  ├─ LAB4A/                  # Clock/Idle 기반 GPIO 처리
│  └─ LAB4B/                  # Hwi timer와 button debounce
├─ LAB5/                      # sample-by-sample audio ISR
│  ├─ LAB5A/                  # half-second delay buffer
│  ├─ LAB5B/                  # 40-sample block processing
│  ├─ LAB5C/                  # 400-sample ping/pong + Swi
│  ├─ LAB5D/                  # Hwi/Swi 파일 분리
│  └─ LAB5E/                  # input.note 기반 Task music synthesis
├─ LAB6/                      # 1D EDMA memory transfer
│  ├─ LAB6A/                  # timer-triggered 1D EDMA
│  ├─ LAB6B/                  # 2D image transfer and chain
│  ├─ LAB6C/                  # 2D scale up/down
│  └─ LAB6D/                  # 2D placement/flip/diamond experiments
└─ LAB7/                      # final EDMA audio pipeline
docs/
└─ lab-variants.md            # 코드 기준 버전 차이
```

`labs/LAB2`~`labs/LAB6`의 상위 폴더에는 해당 이름의 기본 프로젝트가 있고, 알파벳 버전은 하위 폴더에 있다. 폴더를 합치면서 코드 내용이나 CCS 프로젝트 이름은 변경하지 않았다.

## Build and run

### Requirements

- Code Composer Studio
- TI C6000 Compiler 8.3.5
- SYS/BIOS 6.76.3.01 및 XDCtools
- OMAP-L138 PDK/CSL
- OMAP-L138 LCDK/EVM 및 JTAG 연결

### CCS

1. CCS에서 `labs/LAB7` 또는 확인하려는 버전(예: `labs/LAB4/LAB4B`)을 existing CCS project로 import한다.
2. `.cproject`에 남아 있는 TI 설치 경로를 로컬 설치 경로로 수정한다.
3. 프로젝트를 Build하고 OMAP-L138 타깃에 Load/Debug한다.
4. `LINE_IN`을 연결하고 버튼으로 처리 모드를 전환한다.

각 LAB의 `.project`, `.cproject`, `.ccsproject`, `app.cfg`는 원래 CCS 프로젝트 단위로 유지되어 있다. `Debug/`, `.config/`, `src/`와 compiler output은 재생성 대상이므로 저장소에 포함하지 않는다.

## Notes and limitations

- 저장소에는 보드가 필요한 CCS 프로젝트만 있으며 호스트 PC에서 실행되는 테스트 프로그램은 없다.
- 실제 보드 측정값인 end-to-end latency, CPU load, SNR, underrun/overrun 횟수는 포함되어 있지 않다.
- `labs/LAB7`의 8.33 ms는 400-sample block period이며 시스템 전체 latency 측정값이 아니다.
- `labs/LAB7`의 `DELEY` 매크로명과 달리 실제 alternate mode는 delay line이 아니라 2 Hz 진폭 변조다.
- `labs/LAB5/LAB5E/main.c`는 실행 시 상위 경로의 `..\\Input.note`를 열도록 작성되어 있으므로 실행 환경에 맞게 입력 파일 경로를 확인해야 한다.
- `labs/LAB6/LAB6B`~`labs/LAB6/LAB6D`의 `image.h`는 2D EDMA 메모리 이동 실험용 정적 이미지 데이터이며 카메라/LCD 드라이버는 포함하지 않는다.
