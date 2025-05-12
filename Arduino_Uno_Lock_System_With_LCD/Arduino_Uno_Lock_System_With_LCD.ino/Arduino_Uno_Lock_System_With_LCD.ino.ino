#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Pin assignments
const int joyX = A0;      // Joystick X-axis (A0)
const int joyY = A1;      // Joystick Y-axis (A1)
const int servoPin = 9;   // Servo signal pin (D9)
const int touchPin = 2;   // Touch sensor pin (D2)

// LCD setup (I2C address 0x27, 16x2 display)
LiquidCrystal_I2C lcd(0x27, 16, 2);

Servo myServo;
String pattern[10];
int sequenceLength = 0;
int requiredTouches = 0;
int lock = 0;
bool touchCompleted = false;

void setup() {
  Serial.begin(9600);  // Serial communication with ESP32
  pinMode(touchPin, INPUT);

  // Initialize LCD
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.print("Starting...");
  delay(2000);

  // Initialize Servo
  myServo.attach(servoPin);
  myServo.write(90);  // Initial lock position
  lcd.clear();
  lcd.print("Device Locked");
  Serial.println("Arduino Uno setup complete");
}

void loop() {
  // Check for incoming data from ESP32
  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');
    if (data.startsWith("PATTERN:")) {
      // Parse pattern and touch count (e.g., "PATTERN:3:4:UP DOWN LEFT")
      data.remove(0, 8);  // Remove "PATTERN:"
      int firstColon = data.indexOf(':');
      sequenceLength = data.substring(0, firstColon).toInt();
      data.remove(0, firstColon + 1);
      int secondColon = data.indexOf(':');
      requiredTouches = data.substring(0, secondColon).toInt();
      data.remove(0, secondColon + 1);
      for (int i = 0; i < sequenceLength; i++) {
        int space = data.indexOf(' ');
        if (space == -1) {
          pattern[i] = data;
          break;
        }
        pattern[i] = data.substring(0, space);
        data.remove(0, space + 1);
      }
      Serial.println("Received pattern: " + String(sequenceLength) + " steps, " + String(requiredTouches) + " touches");
      lcd.clear();
      lcd.print("Pattern received");
      delay(2000);
      touchCompleted = false;  // Reset for new pattern
    }
  }

  // Perform touch step if pattern is received and not yet completed
  if (sequenceLength > 0 && !touchCompleted) {
    Serial.println("Asking for touch input...");
    lcd.clear();
    lcd.print("Touch to unlock");
    lcd.setCursor(0, 1);
    lcd.print("Count: 0");
    
    int count = 0;
    int lastTouchState = -1;
    while (count < requiredTouches) {
      int touchState = digitalRead(touchPin);
      if (touchState != lastTouchState) {
        lastTouchState = touchState;
      }
      if (touchState == HIGH) {
        count++;
        lcd.setCursor(7, 1);
        lcd.print(count);
        Serial.println("Touch detected, count: " + String(count));
        delay(300);
        while (digitalRead(touchPin) == HIGH) {
          delay(50);
        }
        delay(300);
      }
      delay(50);
    }
    touchCompleted = true;
    lcd.clear();
    lcd.print("Touch Complete");
    Serial.println("Touch step completed");
    delay(2000);
  }

  // Ask for pattern input if touches are completed
  if (touchCompleted) {
    Serial.println("Asking for pattern input...");
    lcd.clear();
    lcd.print("Enter pattern");
    
    int step = 0;
    while (step < sequenceLength) {
      String move = readJoystickDirection();
      if (move != "") {
        Serial.println("User entered: " + move);
        lcd.setCursor(0, 1);
        lcd.print("Move: " + move + "     ");
        if (move == pattern[step]) {
          Serial.println("Correct move, step: " + String(step + 1));
          lcd.setCursor(0, 1);
          lcd.print("✔ " + move + "     ");
          step++;
          delay(500);
        } else {
          Serial.println("Wrong pattern, resetting steps");
          lcd.setCursor(0, 1);
          lcd.print("Wrong! Retry");
          delay(1500);
          step = 0;
          lcd.setCursor(0, 1);
          lcd.print("                ");
        }
        waitForNeutral();
      }
    }

    // Pattern matched, unlock/lock the servo
    Serial.println("Pattern matched, unlocking...");
    lcd.clear();
    lcd.print("Unlocked!");
    if (lock == 0) {
      myServo.write(0);  // Unlock
      lock = 1;
      Serial.println("🔓 Unlocked");
    } else {
      myServo.write(90);  // Lock
      lock = 0;
      // Serial.println("🔒 Locked again");
    }
    delay(3000);
    lcd.clear();
    lcd.print("Device Locked");
    // Serial.println("Device locked again");
    touchCompleted = false;  // Reset for next pattern
    sequenceLength = 0;      // Clear pattern to wait for new one
  }
}

String readJoystickDirection() {
  int yVal = analogRead(joyY);
  int xVal = analogRead(joyX);
  String dir = "";
  if (yVal < 400) dir = "UP";
  else if (yVal > 600) dir = "DOWN";
  else if (xVal > 600) dir = "LEFT";
  else if (xVal < 400) dir = "RIGHT";
  return dir;
}

void waitForNeutral() {
  while (true) {
    int yVal = analogRead(joyY);
    int xVal = analogRead(joyX);
    if (yVal >= 400 && yVal <= 600 && xVal >= 400 && xVal <= 600) {
      break;
    }
    delay(50);
  }
}