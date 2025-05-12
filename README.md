Components Used: 
Arduino Uno, ESP32, Joystick Module, Touch Sensor, Servo Motor, 
LCD Display (I2C, 16x2), Twilio API, Jumper Wires, Breadboard.


🔧 How It Works
ESP32 (Pattern Generator + SMS Sender)
Connects to WiFi and generates a random pattern (3–5 joystick directions) and required number of touches (2–5).

Sends the pattern and touch count to:

The Arduino Uno via Serial (pins D4 and D5).

A mobile number via Twilio SMS API.

Sends only once at startup and remains idle afterward.

Arduino Uno (Pattern Verifier + Lock Controller)
Receives the pattern and touch count from ESP32.

Waits for the user to:

Touch the sensor the specified number of times.

Input the correct joystick pattern (UP/DOWN/LEFT/RIGHT).

On successful match:

Unlocks a servo motor (e.g., for a physical lock).

Displays status on a 16x2 I2C LCD.

Automatically resets after unlocking to be ready for a new pattern.



Well it's not completed yet the device will not be able to lock 
if possible lock the device by doing some touches in the touch sensor


See ya'll

By Akshath & team
