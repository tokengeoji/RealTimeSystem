# LAB variant comparison

이 문서는 저장소에 있던 CCS 프로젝트를 계열별로 묶은 뒤, 각 버전의 실제 소스·설정 차이를 기록한다. 디렉터리 이동은 파일을 찾기 쉽게 하기 위한 구조 변경이며, 각 CCS 프로젝트의 코드와 프로젝트명은 보존했다.

## Grouped layout

```text
LAB0/
LAB1/
LAB2/                 # LAB2 기본 버전
├─ LAB2A/
├─ LAB2B/
└─ LAB2C/
LAB3/
└─ LAB3A/
LAB4/
├─ LAB4A/
└─ LAB4B/
LAB5/
├─ LAB5A/
├─ LAB5B/
├─ LAB5C/
├─ LAB5D/
└─ LAB5E/
LAB6/
├─ LAB6A/
├─ LAB6B/
├─ LAB6C/
└─ LAB6D/
LAB7/
```

## LAB0–LAB1: board I/O base

| Version | Code-level difference |
|---|---|
| `LAB0` | `hello.c`와 `OMAPL138.cmd`로 C6740 프로젝트 시작과 메모리 배치를 확인한다. |
| `LAB1` | `main.c`에서 GPIO를 polling한다. 현재 활성 `#else` branch는 두 push button 상태에 따라 LED 그룹을 토글한다. 파일 안의 `#if 0` branch에는 DIP switch 기반 `LAB1` 예제가 남아 있다. |
| `LAB1A` | 별도 CCS 폴더가 아니다. `LAB1/main.c`의 활성 `#else` branch에 있는 push-button 예제의 표시명이다. |

따라서 `LAB1A/`를 새 프로젝트로 복사하지 않았다. 현재 소스의 compile-time branch를 그대로 보존하는 것이 코드 변경이 가장 적고, 두 예제를 모두 확인할 수 있다.

## LAB2: GPIO interrupt, timer, debounce

공통적으로 OMAP-L138 GPIO bank 2의 push button interrupt와 LED/DIP 라이브러리를 사용한다.

| Version | Main/ISR 차이 | 핵심 기술 |
|---|---|---|
| `LAB2` | GPIO ISR에서 `led_flag`와 LED 모드를 변경하고, main loop가 LED를 토글한다. | GPIO interrupt |
| `LAB2A` | Timer interrupt를 하나 추가한다. `ConfigTimer_32bit(500)` 후 `TIMER_TINT12_ISR`가 D4~D7을 토글한다. | GPIO interrupt + timer interrupt |
| `LAB2B` | Timer0/Timer1을 동시에 구성한다. 두 timer ISR이 D6~D7, D4~D5를 나누어 토글한다. | multiple timer interrupt |
| `LAB2C` | Timer1은 LED blink에 사용하고 Timer0은 button debounce에 사용한다. button ISR이 GPIO interrupt를 끄고 Timer0을 시작하며, Timer0 ISR이 상태를 clear한 뒤 GPIO interrupt를 재활성화한다. | software debounce + dual timer |

주요 근거 파일은 각 폴더의 `main.c`, `ISR.c`, `lib/Timer.c`, `lib/Interrupt.c`이다.

## LAB3: sine generation

두 버전 모두 `sine.c`의 `GenSine()`을 사용해 8 kHz 기준 사인 샘플을 만든다.

| Version | Difference |
|---|---|
| `LAB3` | 250 Hz 사인을 `STEP=10` 단위로 200-sample 배열에 이어서 생성한다. |
| `LAB3A` | 200 Hz 사인을 200-sample 배열 전체에 반복 생성하고 `MicroSecDelay(300000)`으로 반복 사이를 지연한다. `my_gel.gel`이 추가되어 디버거/GEL 실험 구성이 포함된다. |

이 단계의 `main.c`는 파형 배열을 생성하지만 오디오 codec/McASP 출력 경로까지 연결하지 않는다.

## LAB4: SYS/BIOS scheduling

| Version | Configuration and code difference |
|---|---|
| `LAB4` | SYS/BIOS 기본 프로젝트와 `Hello World`만 확인한다. `BIOS_start()`는 주석 처리되어 있고 보드 I/O 초기화도 없다. |
| `LAB4A` | `BIOS_start()`를 활성화하고 GPIO/LED를 초기화한다. `app.cfg`에서 500-tick/250-tick `Clock` 두 개와 두 개의 `Idle` callback을 등록해 button polling과 LED 처리를 SYS/BIOS 실행 단위로 분리한다. |
| `LAB4B` | `LAB4A`의 Clock/Idle polling 대신 Hwi를 사용한다. `app.cfg`에서 vector 4/event 40의 Timer1 Hwi와 vector 8/event 49의 GPIO Hwi를 등록하고, button Hwi가 Timer1을 시작해 500 ms 동안 입력을 차단한다. `ProcessClk`는 LED heartbeat를 담당한다. |

즉 `LAB4` 계열의 차이는 주변장치 자체보다 SYS/BIOS의 실행 모델이 `기본 설정 → Clock/Idle → Hwi`로 확장된 점이다.

## LAB5: codec/McASP audio processing

`LAB5`부터 TLV320AIC3106을 I2C로 초기화하고 McASP0의 `RBUF14`/`XBUF13`을 통해 오디오 샘플을 주고받는다. 공통 오디오 설정은 48 kHz, 16-bit, stereo이며, 버튼은 처리 모드를 바꾼다.

| Version | Buffer/processing design | Difference |
|---|---|---|
| `LAB5` | sample-by-sample | `MCASP_ISR`에서 매 샘플을 직접 읽고 쓴다. `TALKTHR`는 입력을 통과시키고, 다른 모드는 300 Hz 사인을 생성한다. 처리와 I/O가 같은 ISR에 있다. |
| `LAB5A` | `BUFLEN = SAMPLING_FREQ/2` | 약 0.5초 circular buffer를 만들고, alternate mode에서 현재 입력의 left와 과거 `data[cnt]`의 right를 조합한다. 지연 효과를 ISR 안에서 샘플 단위로 구현한다. |
| `LAB5B` | `BUFLEN = 40` | 40샘플을 ISR에서 수집한 뒤 같은 ISR 안에서 block copy 또는 `GenSine()` 기반 2 Hz 진폭 변조를 수행한다. float sine buffer가 추가된다. |
| `LAB5C` | `BUFLEN = 400`, input/output ping/pong | 샘플 I/O는 `MCASP_ISR`가 담당하고 블록이 끝나면 `Swi_post(SWI0)`로 `ProcessSwi0()`를 실행한다. 다만 처리 함수가 아직 `HWI.c` 안에 있다. |
| `LAB5D` | `BUFLEN = 400`, input/output ping/pong | `ProcessSwi0()`를 `SWI.c`로 분리한다. Hwi는 현재 버퍼를 바꾸고 `Swi_or(SWI0, which_buffer)`로 처리 대상 버퍼를 전달하며, Swi는 `Swi_getTrigger()`로 대상 버퍼를 선택한다. EDMA 없이 인터럽트 기반 block pipeline을 구성한 버전이다. |
| `LAB5E` | Task 기반 note playback | `input.note` 파일을 읽어 key/duration을 파싱하고 `PlayNote()` Task가 음표를 순서대로 재생한다. `MCASP_ISR`는 `omega`와 `time_index`를 사용해 현재 음을 출력한다. 앞선 button mode/ping-pong audio effect 실험과는 별도의 합성 경로다. |

`LAB5C`와 `LAB5D`는 block processing과 RTOS scheduling의 경계를 시험한 단계이고, `LAB7`에서 여기에 EDMA 전송과 Hwi 기반 ping/pong 주소 교체가 추가된다.

## LAB6: EDMA transfer and 2D memory movement

`LAB6` 계열은 오디오 경로가 아니라 EDMA3의 transfer parameter를 검증하는 메모리 이동 실험이다.

| Version | EDMA difference |
|---|---|
| `LAB6` | `Uint32` source/destination 배열을 사용한 1D transfer다. `EDMA_EVENT5`를 software event로 발생시키고, A-sync/AB-sync 설정과 polling 또는 compile-time interrupt 경로를 확인한다. |
| `LAB6A` | 1D 배열 transfer는 같지만 Timer0 event(`EDMA_EVENT10`)를 trigger로 사용한다. `Hwi.c`에서 timer event를 처리하고 Task가 transfer 완료를 기다린다. |
| `LAB6B` | `image.h`의 256×256 `Uint8` 이미지를 DDR의 3×3 배치 영역으로 옮기는 2D transfer다. `SetupEDMA3Chain()`으로 event 5~11을 연결하고 event 12에서 마지막 전송을 완료하는 chain 실험이 활성 branch다. |
| `LAB6C` | `SetupEDMA3Down()`에서 source stride 2와 절반의 A/B/CCNT를 사용해 축소 전송을 수행하고, `SetupEDMA3Up()`에서 destination stride 2와 offset을 사용해 확대/배치한다. |
| `LAB6D` | 2D parameter helper를 확장해 parallel, parallel2, diamond, reverse, 좌우/상하 flip, link-chain 설정을 구현했다. 현재 활성 branch는 diamond 및 reverse-diamond 전송이며 나머지 경로는 conditional/commented 실험 코드로 남아 있다. |

`LAB6B`~`LAB6D`에는 이미지 데이터와 `.image` linker section이 있지만 LCD controller나 camera capture 코드는 없다. 따라서 이 계열은 화면 제품 기능이 아니라 EDMA의 2D stride/index/link 동작을 검증하는 코드로 분류한다.

## LAB7: final audio pipeline

`LAB7`은 `LAB5D`의 block-level Hwi/Swi 구조에 EDMA3를 연결한 최종 경로다.

- `main.c`: PINMUX, GPIO, I2C, codec, McASP, EDMA와 BIOS 시작 순서를 구성한다.
- `lib/EDMA_McASP.c`: McASP RX/TX PARAMSET의 source/destination, ACNT=4, BCNT=400, CCNT=1을 설정한다.
- `HWI.c`: EDMA 완료를 확인하고 다음 input/output ping/pong 주소를 `PARAMSET`에 기록한 뒤 `Swi_or`를 호출한다.
- `SWI.c`: `Swi_getTrigger()`로 완료된 input buffer를 선택해 talk-through 또는 2 Hz 진폭 변조를 실행한다.
- `app.cfg`: Timer1 Hwi, GPIO Hwi, EDMA Hwi, SWI0, Idle callback을 등록한다.

최종 버전에서 `EDMA_LINK` 매크로는 정의되어 있지 않으므로, 활성 경로는 정적 link chain만으로 순환하지 않고 Hwi가 PARAMSET 주소와 count를 직접 갱신하는 방식이다.

## Known source caveats

- 여러 LAB에는 교육 단계에서 복사된 공통 `lib/` 파일이 각각 존재한다. 이를 하나의 공통 라이브러리로 합치면 CCS 프로젝트의 include/link 설정까지 바뀌므로 이번 정리에서는 복사본을 유지했다.
- `LAB5A`, `LAB5B`, `LAB5C`, `LAB5D`, `LAB7`의 모드 매크로는 `DELEY`라고 이름 붙었지만 해당 최종 구현은 delay line이 아니라 진폭 변조다.
- `LAB5/LAB5E/main.c`의 입력 경로는 `..\\Input.note`이며 저장소의 샘플 파일은 `LAB5/LAB5E/input.note`다. 실행 전에 CCS working directory 또는 경로를 맞춰야 한다.
- Hwi 내부의 EDMA 완료 대기와 일부 busy-wait는 교육용 구현에 남아 있다. 제품용 전환 시 ISR 실행 시간과 overrun/underrun을 측정해야 한다.
