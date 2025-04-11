# 🌬️ Smart Air Control System

환경 센서를 이용해 실내 공기 질을 측정하고, 서버로 데이터를 전송하여 환기 팬을 자동으로 제어하는 임베디드 공조 시스템입니다.  
STM32, Raspberry Pi, Arduino로 구성되었습니다.

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
- STM32는 PWM으로 팬 속도 조절
- TIM3 기반으로 30초 주기마다 센서 재측정 및 전송

---

## ▶️ 시험 순서

1. 아두이노: `mini_project.ino` 업로드
2. Raspberry Pi:
   - `Bluetooth_client/` 시작 (아두이노 연결)
   - `server/` 시작 (TCP 서버)
   - `sql_client_/` 시작 (DB 저장)
3. STM32: CubeIDE로 프로젝트 빌드 후 보드 업로드
4. 실시간 측정 및 제어 자동 수행

---

## 📉 규정 및 유의 사항

- Wi-Fi SSID/PASS는 STM32 코드 내에 지정되어 있으므로 수정 필요

---

