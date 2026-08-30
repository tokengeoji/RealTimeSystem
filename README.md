# OMAP-L138 실시간 오디오 신호처리 시스템

TI OMAP-L138 LCDK/EVM의 C6740 DSP에서 오디오 입력을 실시간으로 수집하고, EDMA 기반 ping/pong 버퍼와 TI SYS/BIOS 스케줄링으로 신호를 처리한 뒤 오디오로 출력하는 임베디드 프로젝트다.

최종 실행 경로는 `LAB7`에 모여 있다. 나머지 프로젝트 폴더는 같은 하드웨어를 직접 제어하면서 GPIO, 인터럽트, 타이머, 오디오 코덱, 프레임 처리, EDMA를 검증·확장한 소스 변형이며, 기능 설명은 하나의 시스템 관점으로 통합했다.

## 프로젝트 결과

- LINE_IN 스테레오 오디오를 48 kHz, 16-bit PCM으로 수신
- McASP0과 EDMA3로 오디오 데이터 이동을 CPU 연산과 분리
- 400 샘플 ping/pong 버퍼로 입출력과 신호처리를 겹쳐 실행
- 원음 전달 또는 2 Hz 사인파 기반 진폭 변조를 버튼으로 전환
- GPIO·Timer 기반 버튼 디바운싱과 LED heartbeat 구현
- 코덱, 직렬 오디오 인터페이스, DMA, interrupt, RTOS scheduler를 하나의 데이터 경로로 통합

## 시스템 구성

```text
LINE_IN
   │
   ▼
TLV320AIC3106 ADC ── McASP0 RBUF14 ── EDMA RX ──┐
                                                │
                                      input ping/pong
                                                │
                                      SYS/BIOS SWI
                                      ├─ talk-through
                                      └─ 2 Hz amplitude modulation
                                                │
                                      output ping/pong
                                                │
TLV320AIC3106 DAC ◄─ McASP0 XBUF13 ◄─ EDMA TX ◄─┘

Push button ── GPIO interrupt ── Timer debounce ── mode change
LED D4      ◄─ SYS/BIOS Idle/processing heartbeat
```

## 하드웨어 사양

| 항목 | 구성 |
|---|---|
| 보드/타깃 | TI OMAP-L138 LCDK/EVM |
| DSP | TI C6740 계열 부동소수점 DSP |
| 프로젝트 디바이스 | `TMS320C67XX.OMAPL138.LCDKOMAPL138` |
| XDC 플랫폼 | `ti.platforms.evmOMAPL138` |
| 오디오 코덱 | TLV320AIC3106 |
| 오디오 입력 | LINE_IN |
| 샘플링 주파수 | 48,000 Hz |
| 코덱 워드 길이 | 16-bit |
| 채널 | 좌·우 스테레오 |
| 코덱 제어 | I2C0, 소스 설정값 400 |
| 오디오 직렬 포트 | McASP0 |
| 수신/송신 레지스터 | `RBUF14` / `XBUF13` |
| EDMA 이벤트 채널 | 수신 `REVT=0`, 송신 `XEVT=1` |
| 버튼 | GPIO2 4번·5번 핀 |
| 상태 LED | D4~D7 |
| 타이머 | DSP Timer0·Timer1 |

코드 주석에는 코덱 시스템 클록을 24.576 MHz로 명시하고 있다. CPU 동작 주파수, 실제 보드 메모리 용량, 아날로그 입출력 커넥터 사양처럼 소스에 직접 확정되지 않은 값은 추정하지 않았다.

## 소프트웨어 스택

- C99
- TI Code Composer Studio
- TI C6000 Compiler 8.3.5, `-mv6740`
- little-endian, EABI
- TI SYS/BIOS 6.76.3.01
- XDCtools 기반 RTSC 설정
- TI OMAP-L138 PDK 및 Chip Support Library
- 메모리 매핑 레지스터 직접 제어

직접 제어하는 주변장치는 SYSCFG/PINMUX, GPIO, I2C, McASP, Timer, DSP interrupt controller, EDMA3다.

## 실시간 처리 설계

### 오디오 데이터 포맷

코덱은 16-bit 좌·우 샘플을 생성하고, 소프트웨어는 이를 하나의 32-bit 워드에 패킹한다.

```text
uint32_t sample = [ left 16-bit ][ right 16-bit ]
```

### 블록과 버퍼

- `SAMPLING_FREQ = 48000`
- `BUFLEN = 400`
- `ELEMENT_SIZE = 4 bytes`
- 입력 버퍼: `in_ping_buffer`, `in_pong_buffer`
- 출력 버퍼: `out_ping_buffer`, `out_pong_buffer`
- 버퍼 1개: 1,600 bytes
- 네 개 버퍼: 6,400 bytes
- 블록 시간: 400 / 48,000 = 약 8.33 ms
- 처리 빈도: 초당 120개 블록

EDMA가 한쪽 버퍼로 다음 오디오 블록을 수신·송신하는 동안, DSP CPU는 반대쪽 버퍼를 계산한다. 이 구조로 샘플마다 CPU가 주변장치 레지스터를 직접 처리하지 않고 블록 단위로 신호처리를 수행한다.

### Hwi와 Swi의 역할 분리

1. McASP 데이터가 EDMA를 통해 입력 ping 또는 pong 버퍼에 채워진다.
2. EDMA 전송 완료 이벤트가 `EDMA_ISR`를 호출한다.
3. Hwi가 완료 플래그를 정리하고 다음 수신·송신 버퍼 주소를 교체한다.
4. Hwi가 `Swi_or`로 처리할 버퍼 정보를 전달한다.
5. `ProcessSwi0`가 `Swi_getTrigger`를 이용해 방금 채워진 버퍼를 선택한다.
6. SWI가 원음 전달 또는 진폭 변조를 수행해 출력 버퍼를 채운다.
7. 다음 EDMA 송신이 출력 버퍼를 `XBUF13`으로 이동한다.

기본 설정에서는 `EDMA_LINK`가 정의되어 있지 않다. 따라서 최종 실행 경로는 링크 체인만으로 버퍼를 넘기기보다 Hwi에서 `PARAMSET`의 주소를 직접 교체하고 자체 재사용 링크를 설정하는 방식이다.

## 신호처리 기능

### Talk-through

`TALKTHR` 모드에서는 입력 32-bit 워드를 그대로 출력 버퍼로 복사한다. 이 경로는 코덱 ADC부터 DSP, EDMA, McASP, DAC까지의 기본 오디오 loop-through를 검증한다.

### 2 Hz 진폭 변조

다른 모드에서는 `GenSine`이 48 kHz 기준 2 Hz 사인파를 생성한다. 입력 워드에서 좌·우 16-bit 샘플을 분리한 뒤 같은 사인파 값을 각각 곱하고 다시 32-bit 워드로 결합한다.

소스의 모드 이름에는 `DELEY`가 있지만, 실제 구현은 지연선이 아니라 2 Hz 진폭 변조다.

## 하드웨어 제어 방식

### TLV320AIC3106 초기화

`CodecInit`이 I2C를 통해 코덱 레지스터를 순서대로 설정한다.

- 코덱 소프트 리셋
- 48 kHz 샘플링 PLL 설정
- 16-bit 오디오 직렬 인터페이스 설정
- LINE_IN ADC 경로 선택
- 좌·우 ADC gain 설정
- 좌·우 DAC 전원과 출력 경로 설정
- AGC 비활성화

`Codec_RSET`은 레지스터 주소와 값을 2 byte 데이터로 조합해 `I2C_Write`로 전송한다.

### McASP0 설정

`ConfigMcASP`가 수신·송신 마스크와 포맷, frame sync, TDM slot, serializer를 설정한다. 최종 소스는 32-bit 전송 단위와 1-slot 설정을 사용하고, 수신은 `RBUF14`, 송신은 `XBUF13`으로 연결한다.

### EDMA3 설정

`ConfigEDMAForR`와 `ConfigEDMAForX`가 McASP 데이터 레지스터와 메모리 버퍼 사이의 EDMA PARAMSET을 만든다.

- 수신: `RBUF14` → 입력 버퍼
- 송신: 출력 버퍼 → `XBUF13`
- 동기화: asynchronous transfer
- 한 전송 요소: 4 byte
- 한 블록: 400 elements
- 완료 시 transfer-complete interrupt 발생

### 사용자 입력과 상태 표시

GPIO bank 2 버튼 이벤트가 발생하면 Hwi가 처리 모드를 바꾼다. 직후 GPIO interrupt를 잠시 끄고 Timer1을 시작하며, 500 ms 뒤 timer ISR에서 interrupt 상태를 정리하고 다시 활성화한다. D4는 처리 블록 카운터에 따라 약 0.5초마다 토글되어 시스템 동작 여부를 표시한다.

## SYS/BIOS 구성

`LAB7/app.cfg`에 다음 실행 단위를 등록한다.

- Hwi vector 4 / event 40: `TIMER1_TINT12_ISR`
- Hwi vector 8 / event 49: `GPIO_PUSHBUTTON_ISR`
- Hwi vector 5 / event 8: `EDMA_ISR`
- SWI0: `ProcessSwi0`
- Idle callback: `IdleLED`

초기화는 PINMUX·GPIO → I2C·코덱 → McASP → EDMA → interrupt enable → `BIOS_start` 순서로 진행된다.

## 저장소 구조

```text
LAB7/
├─ main.c                 # 최종 시스템 초기화와 EDMA 구성
├─ HWI.c                  # EDMA 완료·버튼 Hwi, 버퍼 교체
├─ SWI.c                  # 블록 단위 오디오 신호처리
├─ app.cfg                # SYS/BIOS Hwi·Swi·Idle 구성
├─ define.h               # 샘플링·버퍼 상수
└─ lib/
   ├─ Codec.c             # TLV320AIC3106 레지스터 설정
   ├─ I2C.c               # I2C 레지스터 송수신
   ├─ McASP.c              # 오디오 직렬 포트 설정
   ├─ EDMA_McASP.c         # McASP용 EDMA PARAMSET
   ├─ GPIO.c / LED_DIPSW.c # 버튼·DIP·LED 제어
   ├─ Timer.c              # timer 설정
   └─ Interrupt.c          # DSP interrupt 설정
```

폴더의 다른 CCS 프로젝트에는 동일 공통 라이브러리의 복사본, EDMA 1D 배열 전송, 2D 이미지 메모리 이동, 파일 기반 음표 합성 코드가 포함되어 있다. 이는 동일 DSP 주변장치의 보조 검증 코드이며 최종 오디오 시스템의 실행 경로와 구분한다.

## 빌드와 실행

실제 보드 실행에는 다음 환경이 필요하다.

1. TI Code Composer Studio와 C6000 Compiler 8.3.5 설치
2. TI SYS/BIOS 6.76.3.01, XDCtools, OMAP-L138 PDK/CSL 설치
3. CCS에서 `LAB7`을 existing project로 import
4. 프로젝트의 include/library 경로를 로컬 TI 설치 경로에 맞게 조정
5. OMAP-L138 LCDK/EVM을 연결하고 Build 후 Debug/Load
6. LINE_IN을 연결한 뒤 버튼으로 처리 모드 전환

프로젝트 설정 파일에는 개발 당시의 절대 경로가 일부 남아 있으므로 다른 PC에서는 CCS 경로 재설정이 필요하다.

## 검증 범위와 한계

- 소스와 CCS 프로젝트 설정을 기준으로 하드웨어 데이터 경로와 실행 흐름을 확인했다.
- 로컬 폴더에는 `LAB7.out`과 `LAB7.map` 빌드 산출물이 있었지만, GitHub에는 재생성 가능한 소스 중심으로 올린다.
- 8.33 ms는 400 샘플 블록의 처리 주기이며, 실제 end-to-end latency가 아니다.
- CPU load, SNR, underrun/overrun 횟수, 실제 오디오 녹음 결과는 저장소에 측정 자료가 없다.
- EDMA 완료 플래그를 Hwi 내부에서 확인하는 busy wait가 있으므로 제품화 단계에서는 ISR 실행 시간과 완료 순서를 계측해야 한다.
- 카메라·LCD 드라이버는 확인되지 않았으므로 2D 이미지 코드는 화면 제품 기능으로 설명하지 않는다.

## 취업용 프로젝트 설명

TI OMAP-L138 C6740 DSP에서 TLV320AIC3106 코덱을 I2C로 초기화하고 McASP0과 EDMA3를 이용해 48 kHz 스테레오 PCM 스트림을 실시간 처리했다. 400 샘플 ping/pong 버퍼를 설계해 EDMA가 다음 오디오 블록을 전송하는 동안 SYS/BIOS Swi가 이전 블록을 처리하도록 구성했으며, Hwi에서는 전송 완료와 버퍼 교체만 담당하도록 역할을 분리했다. 또한 GPIO interrupt와 Timer 디바운싱, 버튼 기반 처리 모드 전환, LED heartbeat까지 구현해 레지스터 수준 하드웨어 제어부터 실시간 데이터 경로 설계까지 경험했다.

## 원본 근거

- 최종 실행 흐름: `LAB7/main.c`, `LAB7/app.cfg`
- 실시간 처리: `LAB7/HWI.c`, `LAB7/SWI.c`, `LAB7/define.h`
- 코덱·통신: `LAB7/lib/Codec.c`, `LAB7/lib/I2C.c`
- 오디오 포트·DMA: `LAB7/lib/McASP.c`, `LAB7/lib/EDMA_McASP.c`
- 하드웨어 제어: `LAB7/lib/Init.c`, `LAB7/lib/GPIO.c`, `LAB7/lib/LED_DIPSW.c`, `LAB7/lib/Timer.c`, `LAB7/lib/Interrupt.c`
- 타깃 설정: `LAB7/.cproject`, `LAB7/Debug/configPkg/compiler.opt`, `LAB7/Debug/configPkg/linker.cmd`
