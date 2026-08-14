#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int signalInput = A3; 
const int MOTOR_BOARD2 = 6;  

LiquidCrystal_I2C lcd(0x27, 16, 2);
int rawSignal;
int pwmValue;
int speedPercent;

void setup() {
  pinMode(MOTOR_BOARD2, OUTPUT);
  
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0,0);
  lcd.print("REMOTE SYSTEM");
  lcd.setCursor(0,1);
  lcd.print("Initializing...");
  delay(1500);
  lcd.clear();
}

void loop() {
  
  rawSignal = analogRead(signalInput);
  
 
  pwmValue = map(rawSignal, 0, 1023, 0, 255);
  
  
  speedPercent = map(rawSignal, 0, 1023, 0, 100);

  
  analogWrite(MOTOR_BOARD2, pwmValue);

  
  lcd.setCursor(0,0);
  lcd.print("Motor Spd: ");
  lcd.print(speedPercent);
  lcd.print("%   "); 

   lcd.setCursor(0,1);
  if(speedPercent < 5) {
    lcd.print("IDLE/STOPPED    ");
  } else {
   
    int bars = map(speedPercent, 0, 100, 0, 16);
    for(int i = 0; i < bars; i++) {
      lcd.print((char)255); 
    }
    for(int i = bars; i < 16; i++) {
      lcd.print(" "); 
    }
  }

  delay(200); 
}