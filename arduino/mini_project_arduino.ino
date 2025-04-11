/*
  blue test: 
  http://www.kccistc.net/
  작성일 : 2024.03.22
  작성자 : IoT 임베디드 KSH
  수정일 : 2025.03.04
*/
#include <SoftwareSerial.h>
#include <Wire.h>
#include <MsTimer2.h>
#include <LiquidCrystal_I2C.h>
#include <time.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define DEBUG
#define BTN1_PIN 2  // 수동/자동 모드 전환 버튼
#define BTN2_PIN 3  // 모터 ON/OFF 버튼
#define MOTOR_PIN 6
#define LED_BUILTIN_PIN 13
#define R_LED 12
#define Y_LED 9
#define G_LED 8
#define ARR_CNT 5
#define CMD_SIZE 60

char lcdLine1[17] = "Mode: AUTO"; // 첫 번째 줄 - 모드 표시
char lcdLine2[17] = ""; // 두 번째 줄 - 센서 데이터
char sendBuf[CMD_SIZE];
char recvId[10] = "JJH_ARD";  // SQL 저장 클라이이언트 ID
char stm32Id[10] = "JHK_STM32"; // STM32 클라이언트 ID

// 버튼 상태 변수
bool lastBtn1 = LOW;       // 버튼1의 이전 상태
bool currentBtn1 = HIGH;   // 버튼1의 현재 상태
bool lastBtn2 = LOW;       // 버튼2의 이전 상태
bool currentBtn2 = HIGH;   // 버튼2의 현재 상태

bool ledOn = false;      // LED 상태
bool timerIsrFlag = false;
bool isAutoMode = true;  // 기본값은 자동 모드
unsigned int secCount;

// STM32에서 수신한 센서 값을 저장할 변수
int gas = 0;      // 가스 센서 값
int temp = 0;   // 온도 값
int humi = 0;   // 습도 값
bool gasFlag = false;  
int getSensorTime;
SoftwareSerial BTSerial(10, 11); // RX ==>BT:TXD, TX ==> BT:RXD

void setup()
{
#ifdef DEBUG
  Serial.begin(115200);
  Serial.println("setup() start!");
#endif
  lcd.init();
  lcd.backlight();
  
  // LCD 초기 표시
  lcdDisplay(0, 0, lcdLine1);
  lcdDisplay(0, 1, lcdLine2);
  
  // 버튼 핀 설정
  pinMode(BTN1_PIN, INPUT);
  pinMode(BTN2_PIN, INPUT);
  
  pinMode(LED_BUILTIN_PIN, OUTPUT);
  pinMode(R_LED, OUTPUT);
  pinMode(Y_LED, OUTPUT);
  pinMode(G_LED, OUTPUT);
  
  BTSerial.begin(9600); 
  MsTimer2::set(1000, timerIsr); 
  MsTimer2::start();
}

void loop()
{
  if (BTSerial.available())
    bluetoothEvent();
    
  if (timerIsrFlag)
  {
    timerIsrFlag = false;
    
    if(getSensorTime != 0 && !(secCount % getSensorTime)) {
      char tempStr[5];
      char humiStr[5];
      dtostrf(temp, 4, 1, tempStr);  
      dtostrf(humi, 4, 1, humiStr);  
      sprintf(sendBuf,"[%s]SENSOR@%d@%s@%s\r\n",recvId,gas,tempStr,humiStr);
      BTSerial.write(sendBuf);   
    }    
    
    // 가스 값이 변경되었을 때 알림 전송 (500 기준)
    if ((gas >= 500) && !gasFlag) {
      gasFlag = true;
      sprintf(sendBuf, "[%s]GAS@%d\n", recvId, gas);
      BTSerial.write(sendBuf);
    } 
    else if ((gas < 500) && gasFlag) {
      gasFlag = false;
      sprintf(sendBuf, "[%s]GAS@%d\n", recvId, gas);
      BTSerial.write(sendBuf);
    }
  }
  
  // 버튼1(모드 전환) 처리
  currentBtn1 = debounce(lastBtn1, BTN1_PIN);
  if (lastBtn1 == HIGH && currentBtn1 == LOW) {  // 버튼1을 눌렀을 때
    isAutoMode = !isAutoMode;  // 모드 전환
    updateModeDisplay();       // LCD 표시 업데이트
    
    // 자동 모드로 전환 시 모터 제어 초기화 (가스 값에 따라 자동 제어됨)
    if (isAutoMode) {
      // LED 초기화
      digitalWrite(R_LED, LOW);
      digitalWrite(Y_LED, LOW);
      digitalWrite(G_LED, LOW);
      // 현재 가스 값에 따라 모터 제어
      controlMotorByGas(gas);
    }
  }
  lastBtn1 = currentBtn1;
  
  // 버튼2(수동 모드에서 모터 ON/OFF) 처리
  currentBtn2 = debounce(lastBtn2, BTN2_PIN);
  if (lastBtn2 == HIGH && currentBtn2 == LOW) {  // 버튼2를 눌렀을 때
    if (!isAutoMode) {  // 수동 모드일 때만 동작
      ledOn = !ledOn;
      digitalWrite(LED_BUILTIN_PIN, ledOn);
      
      // 모터 ON/OFF 명령 전송
      sprintf(sendBuf, "[%s]MOTOR@%s\n", stm32Id, ledOn ? "ON" : "OFF");
      BTSerial.write(sendBuf);
      
#ifdef DEBUG
      Serial.print("Manual motor control: ");
      Serial.println(sendBuf);
#endif
    }
  }
  lastBtn2 = currentBtn2;

#ifdef DEBUG
  if (Serial.available())
    BTSerial.write(Serial.read());
#endif
}

void bluetoothEvent()
{
  int i = 0;
  char * pToken;
  char * pArray[ARR_CNT] = {0};
  char recvBuf[CMD_SIZE] = {0};
  int len = BTSerial.readBytesUntil('\n', recvBuf, sizeof(recvBuf) - 1);

  Serial.print("Recv : ");
  Serial.println(recvBuf);

  pToken = strtok(recvBuf, "[@]");
  while (pToken != NULL)
  {
    pArray[i] =  pToken;
    if (++i >= ARR_CNT)
      break;
    pToken = strtok(NULL, "[@]");
  }
  Serial.println(pArray[0]);
  // STM32에서 오는 메시지만 처리 (ALLMSG 또는 JHK_STM32에서 오는 메시지)
  
    //Serial.print("allmsg or stm32 ");
    // SENSOR 데이터 수신 (STM32에서 보내는 센서 값)
    if (!strcmp(pArray[0], "SENSOR") && i >= 4) {
      //Serial.print("data saving ");
      // 가스, 온도, 습도 값 저장
      gas = atoi(pArray[1]); // pArray[1]은 "SENSOR", pArray[2]는 가스 값
      //Serial.print("gas: ");
      //Serial.println(gas);
      temp = atoi(pArray[2]); // pArray[3]은 온도 값
      //Serial.print("temp: ");
      //Serial.println(temp);
      humi = atoi(pArray[3]); // pArray[4]은 습도 값
      //Serial.print("humi: ");
      //Serial.println(humi);
      // LCD에 표시 (두 번째 줄)
      sprintf(lcdLine2, "G:%d T:%d H:%d", gas, temp, humi);
      lcdDisplay(0, 1, lcdLine2);
      
      // 자동 모드일 때만 LED 제어 및 모터 명령 전송
      if (isAutoMode) {
        controlMotorByGas(gas);
      }
    }

  // 서버 연결 관련 메시지는 항상 처리
  if (!strncmp(pArray[1], " New", 4)) // New Connected
  {
    return;
  }
  else if (!strncmp(pArray[1], " Alr", 4)) //Already logged
  {
    return;
  }
}

void lcdDisplay(int x, int y, char * str)
{
  int len = 16 - strlen(str);
  lcd.setCursor(x, y);
  lcd.print(str);
  for (int i = len; i > 0; i--)
    lcd.write(' ');
}

// LCD에 현재 모드 표시 업데이트 (첫 번째 줄)
void updateModeDisplay()
{
  sprintf(lcdLine1, "Mode: %s", isAutoMode ? "AUTO" : "MANUAL");
  lcdDisplay(0, 0, lcdLine1);
  
#ifdef DEBUG
  Serial.print("Mode changed to: ");
  Serial.println(isAutoMode ? "AUTO" : "MANUAL");
#endif
}

void timerIsr()
{
  timerIsrFlag = true;
  secCount++;
}

// 확장된 debounce 함수 - 특정 핀을 매개변수로 받음
bool debounce(bool last, int pin)
{
  bool current = digitalRead(pin);
  if (last != current)
  {
    delay(5);
    current = digitalRead(pin);
  }
  return current;
}

// 가스 값에 따라 모터를 제어하는 함수
void controlMotorByGas(int gasValue) {
  if (gasValue >= 1000) {
    digitalWrite(R_LED, HIGH);
    digitalWrite(Y_LED, LOW);
    digitalWrite(G_LED, LOW);
    sprintf(sendBuf, "[%s]MOTOR@3\n", stm32Id);
    BTSerial.write(sendBuf);
    Serial.print("Send motor command: ");
    Serial.println(sendBuf);
  }
  else if (gasValue >= 301 && gasValue <= 999) {
    digitalWrite(R_LED, LOW);
    digitalWrite(Y_LED, HIGH);
    digitalWrite(G_LED, LOW);
    sprintf(sendBuf, "[%s]MOTOR@2\n", stm32Id);
    BTSerial.write(sendBuf);
    Serial.print("Send motor command: ");
    Serial.println(sendBuf);
  }
  else if (gasValue <= 300) {
    digitalWrite(R_LED, LOW);
    digitalWrite(Y_LED, LOW);
    digitalWrite(G_LED, HIGH);
    sprintf(sendBuf, "[%s]MOTOR@1\n", stm32Id);
    BTSerial.write(sendBuf);
    Serial.print("Send motor command: ");
    Serial.println(sendBuf);
  }
}