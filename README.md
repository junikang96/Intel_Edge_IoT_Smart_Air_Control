# 🌬️ Smart Air Control System : 환경 센서 기반 자동 환기 제어 시스템

이 프로젝트는 실내 공기 질을 실시간으로 모니터링하고, 필요시 환기 팬을 자동 제어하여 쾌적한 환경을 유지하는 IoT 기반 임베디드 공조 시스템을 구축하는 것을 목표로 하였습니다.

---

## 🌐 시스템 구성

- **Arduino Nano**를 이용해 MQ135, DHT11 센서 데이터 수집
- **Bluetooth 통신**을 통해 Raspberry Pi 서버에 센서 데이터 전송
- **Raspberry Pi**에서 서버 수신 및 MySQL DB 저장
- **STM32**를 이용해 서버 명령 수신 후 환기 팬 속도 제어
- **Wi-Fi 모듈 (ESP8266)**을 통한 TCP/IP 소켓 통신

---

## 👨‍💻 담당 역할

- STM32F411RE MCU를 활용한 팬 제어 시스템 설계 및 구현
- TIM4 기반 PWM 출력 로직 개발 (5단계 속도 제어)
- TIM3 인터럽트를 활용한 주기적 데이터 수집 및 송신 플로우 설계
- UART6을 통한 ESP8266 통신 제어 및 서버 명령 파싱
- 하드웨어 회로 구성 및 센서 데이터 연동 테스트

---

## 🔧 하드웨어 제작

- **센서**:
  - MQ135: 실내 CO₂, NH₃ 등 유해가스 농도 측정
  - DHT11: 온습도 측정
- **환기 팬**:
  - STM32에서 PWM 신호 출력
  - 팬의 회전 속도를 5단계로 제어
- **통신 모듈**:
  - Arduino → Raspberry Pi (Bluetooth)
  - Raspberry Pi → STM32 (TCP/IP over Wi-Fi)

---

## 🛠️ 사용 기술

- **MCU**: STM32F411RE (Nucleo-64)
- **센서**: MQ135 (공기질), DHT11 (온습도)
- **아두이노**: 센서 데이터 수집 및 블루투스 통신
- **Raspberry Pi**:
  - 서버 소켓 수신 및 명령 송신
  - MySQL 저장 처리
  - 블루투스 통신 중계
- **Wi-Fi**: ESP8266 (UART6 → TCP)
- **PWM**: 팬 제어용 (TIM4 기반)
- **타이머**: 주기 제어용 (TIM3 기반)

---

## 💡 주요 기능

- 아두이노가 센서 측정값 전송 (MQ135, DHT11)
- 블루투스를 통해 라즈베리파이로 전송
- 서버가 수신 후 MySQL 저장 및 판단
- STM32에 명령 전송 (`[ID]MOTOR@1~3/OFF/ON`)
- STM32는 PWM으로 센서 측정값에 따라 자동으로 팬 속도 제어
- 버튼을 통해 수동으로도 팬 속도 제어 가능
- 주기적 데이터 수집/송신을 TIM3 인터럽트로 관리

---

## 🎥 시연 영상 (추후 업데이트 예정)

- [👉 팬 자동 제어 및 수동 제어 모드 시연 영상](https://youtube.com/shorts/vbZUc9HCvQQ?feature=share)

---

## 📦 프로젝트 구성

```plaintext
.
├── arduino/                 # 환경 센서 측정용 아두이노 코드
│   └── mini_project.ino    
│
├── raspberrypi/             # 서버 및 블루투스-아두이노 중계기 역할
│   ├── Bluetooth_client/    
│   ├── server/              
│   └── sql_client_/         
│
├── stm32/                   # 팬 제어 및 Wi-Fi 통신 MCU
│   ├── Core/               
│   ├── Drivers/             
│   ├── mini_project_final.ioc     
│   ├── STM32F411RETX_FLASH.ld     
│   └── STM32F411RETX_RAM.ld       
```

---

## 📉 규정 및 유의 사항

- Wi-Fi SSID/PASS는 STM32 코드 내에 지정되어 있으므로 수정 필요