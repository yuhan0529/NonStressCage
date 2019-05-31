#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "DHT.h" //습도
//Signal 이 연결된 아두이노의 핀번호
#include <Servo.h>
 
LiquidCrystal_I2C lcd(0x27, 16, 2); //lcd 객체
Servo servo;
int value = 0; //서보
#define DHTPIN 2 //온습도
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
#define setbutton 3  //셋 버튼
#define waterPin A1  //수위센서
#define servoPin A0  //서보모터
#define downbutton 4 //다운버튼
#define upbutton 5  //업버튼
#define backbutton 6  //뒤로가기 버튼
#define motorPin 7 //팬
#define relayPin 8 //릴레이제어
#define fanPin 9 //히터 팬
#define bluePin 10 //led
#define greenPin 11 //led
#define redPin 12 //led
int cnt = 0; //set 모드
int ledcnt = 0; //led 색
int fan_c = 0; //팬 모드
int switch_Gval; //초록 스위치
float humi; //습도
float temp; //온도
int temp_set=0; //온도 설정 //set 온도부터 5도까지
boolean set_ = false; //온도 SET 여부
String fan_mod = "OFF"; //fan 온오프 여부 //set온도에서 10도까지

// 함수 선언////////////////////
void led();
void temp_humi();
void fanModEnd();
void water();
/////////////////////////////////
void setup() {
  Serial.begin(9600);
  dht.begin();
  servo.attach(servoPin);
  pinMode(relayPin, OUTPUT);
  pinMode(motorPin, OUTPUT);
  pinMode(fanPin , OUTPUT);
  pinMode(setbutton, INPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  analogWrite(fanPin,0);
  //lcd 세팅
  lcd.begin();
  lcd.backlight(); //백라이트
  lcd.setCursor(0,0);
  lcd.clear();

  servo.write(0);
}

//루프-----------------------------------------------------------------------
void loop() {

  //버튼 선언
  int setbuttonState, downbuttonState, upbuttonState , backbuttonState;
  setbuttonState = digitalRead(setbutton); 
  downbuttonState = digitalRead(downbutton);
  upbuttonState = digitalRead(upbutton);
  backbuttonState = digitalRead(backbutton);
  
  //버튼 눌렀다면
  if(setbuttonState == LOW){
      set_ = true; //set 버튼 눌림
      Serial.print(F("LOW"));
      lcd.clear();
      lcd.setCursor(1,0);
      lcd.print(F("TEMP SET : "));
      lcd.print(temp_set);
  }
  if(set_== true){ //셋 모드 일경우
    if(setbuttonState == LOW){ //set 한번더 누르면
      cnt++;
    }
    //온도 설정 모드//////////////////////////
      if(cnt % 2 != 0){
      lcd.setCursor(0,0);
      lcd.print(F(">"));
      if(upbuttonState == LOW){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print(F(">"));
        temp_set ++;
      }else if(downbuttonState == LOW){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print(F(">"));
        temp_set --;
      }else if(backbuttonState == LOW){
        lcd.clear();
        cnt = 0;
        set_ = false;
      }
    }
    /////////////////////////////
      //팬 모드
    if(cnt % 2 == 0){
      lcd.setCursor(0,1);
      lcd.print(F(">"));
      if(upbuttonState == LOW){
        lcd.clear();
        lcd.setCursor(0,1);
        lcd.print(F(">"));
        fan_c ++;
      }else if(downbuttonState == LOW){
        lcd.clear();
        lcd.setCursor(0,1);
        lcd.print(F(">"));
        fan_c ++;
      }else if(backbuttonState == LOW){
        lcd.clear();
        cnt = 0;
        set_ = false;
      }
      //팬모드 2넘어가면 다시 0으로
      if(fan_c >2){
        fan_c = 0;
      }else if(fan_c < 0){
        fan_c = 2;
      }
    }
    //팬모드 종료////////////////////////
    fanModEnd();
  }
  //set모드가 아니라면/////////////////
  if(set_==false){
    //////////////////온습도 불러오기//////////
    temp_humi();
    ///////////////////////////////////////
    if(upbuttonState == LOW){
        led();
    }else if(downbuttonState == LOW){
        digitalWrite(bluePin,LOW);
        digitalWrite(greenPin,LOW);
        digitalWrite(redPin,LOW);
    }
  }
  //팬 모드가 자동//////////////////////
    if(fan_mod == "AUTO"){ 
      if(temp > (temp_set + 5)) //temp가 설정온도+5 보다  높으면 팬 작동
        analogWrite(motorPin , 255);
      else
        analogWrite(motorPin, 0);
    }else if(fan_mod == "ON"){ //팬모드 온//////////////////
      analogWrite(motorPin , 255);
    }else if(fan_mod == "OFF"){ //팬 모드 오프///////////////////
      analogWrite(motorPin , 0);
    }

//히터 제어

    if(temp < temp_set){ //설정 온도 보다 낮으면
      digitalWrite(relayPin, HIGH); //히터
      analogWrite(fanPin, 255);  
    }else{
      digitalWrite(relayPin, LOW);
      analogWrite(fanPin,0);
    }

    water();
    
    delay(200);
}


//함수 //////////////////////////////////////////////

//물 센서로 물감지
void water(){
  int val = analogRead(waterPin);
  Serial.println(val);
  if(val < 200){ //물 있을때 (평소)
    servo.write(0);
  }else if(val > 400){ //물부족
  servo.write(90);
  }
}

//팬 작동
void fanModEnd(){
   lcd.setCursor(1,0);
    lcd.print(F("TEMP SET : "));
    lcd.print(temp_set);
    lcd.setCursor(1,1);
    lcd.print(F("FAN MOD : "));   
    switch(fan_c){ //세팅 한 값
      case 0:
        fan_mod = "OFF";
        break;
      case 1:
        fan_mod = "ON";
        break;
      case 2:
        fan_mod = "AUTO";
        break;
    }
    lcd.print(fan_mod);
}
//온/습도 받아오기
void temp_humi(){
    humi = dht.readHumidity();
    temp = dht.readTemperature();
    lcd.setCursor(0,0);
    lcd.print(F("  Temp : "));
    lcd.print(temp);
    lcd.print(F("    "));
    lcd.setCursor(0,1);
    // 습도//////////////////
    lcd.print(F("  Humi : "));
    lcd.print((int)humi);
    lcd.print(F(" %  "));   
    
}

//조명 제어
void led(){
  ledcnt++;
  if(ledcnt>2)
    ledcnt = 0;

  switch(ledcnt){
    case 0 :
      digitalWrite(bluePin,HIGH);
      digitalWrite(redPin,LOW);
      digitalWrite(greenPin,LOW);
      break;
    case 1 :
      digitalWrite(greenPin,HIGH);
      digitalWrite(bluePin,LOW);
      digitalWrite(redPin,LOW);
      break;
    case 2 :
      digitalWrite(redPin,HIGH);
      digitalWrite(bluePin,LOW);
      digitalWrite(greenPin,LOW);
      break;
  }
}
