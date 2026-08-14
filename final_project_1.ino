#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

#define INT_BUTTON_PIN 2  
#define SERVO_PIN 3       
#define MOTOR_PIN 4       
#define TO_BOARD2_PIN 5   
#define BULB_PIN 6        
#define BUZZER_PIN 8      
#define PIR_PIN 13        
#define TEMP_PIN A0       
#define SOIL_PIN A1       
#define LDR_PIN A2        
#define SMOKE_PIN A3      

enum SystemState { IDLE, SENSING, ACTUATING, ERROR };
volatile SystemState currentState = IDLE; 

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo servoMotor;
unsigned long lastRead = 0, lastTelemetry = 0;
int temperature = 0, soil = 0, lightValue = 0, smokeValue = 0, pirValue = 0;

int tempBuffer[8], tempIndex = 0;
long tempTotal = 0;

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(TO_BOARD2_PIN, OUTPUT); 
  pinMode(BULB_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  
  pinMode(INT_BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INT_BUTTON_PIN), emergencyStop, FALLING);

  servoMotor.attach(SERVO_PIN);
  for(int i=0; i<8; i++) tempBuffer[i] = 0;
}

void loop() {
  if (currentState != ERROR) {
    if (millis() - lastRead >= 500) {
      lastRead = millis();
      runFSM(); 
    }
  } else {
    lockdown(); 
  }

  if (millis() - lastTelemetry >= 2000) {
    lastTelemetry = millis();
    sendTelemetry();
  }
}

void runFSM() {
  switch (currentState) {
    case IDLE: currentState = SENSING; break;
    case SENSING:
      readSensors();
      if (soil > 1022) currentState = ERROR; 
      else currentState = ACTUATING;
      break;
    case ACTUATING:
      controlDevices();
      displayLCD();
      currentState = SENSING;
      break;
  }
}

void readSensors() {
  tempTotal -= tempBuffer[tempIndex];
  tempBuffer[tempIndex] = analogRead(TEMP_PIN);
  tempTotal += tempBuffer[tempIndex];
  tempIndex = (tempIndex + 1) % 8;
  temperature = map(tempTotal / 8, 20, 358, -40, 125); 

  soil = analogRead(SOIL_PIN);
  lightValue = analogRead(LDR_PIN);
  smokeValue = analogRead(SMOKE_PIN);
  pirValue = digitalRead(PIR_PIN);
}

void controlDevices() {
 
  digitalWrite(BULB_PIN, (pirValue == HIGH) ? HIGH : LOW);
  

  digitalWrite(BUZZER_PIN, (smokeValue > 500) ? HIGH : LOW);
  

  if (temperature > 25 || lightValue > 400) {
    digitalWrite(MOTOR_PIN, HIGH);   
    analogWrite(TO_BOARD2_PIN, 255); 
    digitalWrite(MOTOR_PIN, LOW);
    analogWrite(TO_BOARD2_PIN, 0);
  }
  
  
  servoMotor.write((soil < 350) ? 90 : 0);
}

void displayLCD() {
  lcd.setCursor(0, 0);
  lcd.print("T:"); lcd.print(temperature);
  lcd.print(" L:"); lcd.print(lightValue);
  lcd.print(" S:"); lcd.print(soil); 
  
  lcd.setCursor(0, 1);
  if (smokeValue > 700) lcd.print("SMOKE DETECTED");
  else if (pirValue == HIGH) lcd.print("INTRUDER ALERT");
  else lcd.print("SYSTEM NORMAL ");
}

void emergencyStop() { currentState = ERROR; }

void lockdown() {
  digitalWrite(MOTOR_PIN, LOW);
  analogWrite(TO_BOARD2_PIN, 0); 
  digitalWrite(BULB_PIN, LOW);
  digitalWrite(BUZZER_PIN, HIGH);
  servoMotor.write(0);
  lcd.setCursor(0, 0); lcd.print("EMERGENCY STOP ");
  lcd.setCursor(0, 1); lcd.print("SYSTEM LOCKED  ");
}

void sendTelemetry() {
  
  Serial.print("{\"tmp\":"); Serial.print(temperature);
  Serial.print(",\"smk\":"); Serial.print(smokeValue);
  Serial.print(",\"ldr\":"); Serial.print(lightValue);
  Serial.print(",\"mst\":"); Serial.print(soil);
  Serial.print(",\"pir\":"); Serial.print(pirValue);
  Serial.println("}");
}