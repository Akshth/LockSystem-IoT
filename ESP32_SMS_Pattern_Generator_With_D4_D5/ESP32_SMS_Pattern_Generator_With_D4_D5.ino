#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "xxx"; //wifi name
const char* password = "xxx"; //wifi pass
String twilioSID = "xxxx";  //twilio sid 
String twilioToken = "xxxx"; //twilio token
String fromNumber = "xxxx"; //number obtained from twilio
String toNumber = "xxxxx"; //number where the generated pattern works

String pattern[10];
int sequenceLength = 0;
int requiredTouches = 0;
bool patternSent = false;  // Track if the pattern has been sent

void sendToPhone(String body) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Cannot send SMS: WiFi not connected.");
    return;
  }
  HTTPClient http;
  String url = "https://api.twilio.com/2010-04-01/Accounts/" + twilioSID + "/Messages.json";
  if (!http.begin(url)) {
    Serial.println("Failed to initialize HTTP client for SMS");
    return;
  }
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.setAuthorization(twilioSID.c_str(), twilioToken.c_str());
  String postData = "To=" + toNumber + "&From=" + fromNumber + "&Body=" + body;
  int code = http.POST(postData);
  if (code > 0) {
    Serial.println("SMS sent, HTTP Code: " + String(code));
  } else {
    Serial.println("SMS failed, HTTP Code: " + String(code));
  }
  http.end();
}

void setup() {
  Serial.begin(9600);  // For Serial Monitor (USB)
  Serial2.begin(9600, SERIAL_8N1, 4, 5);  // Use built-in Serial2 on D4 (RX, GPIO4), D5 (TX, GPIO5)
  Serial.println("ESP32 setup starting...");

  // Connect to WiFi with timeout
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.begin(ssid, password);
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 15000) {
    delay(1000);
    Serial.print("WiFi Status: ");
    Serial.println(WiFi.status());
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi Connected, IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Failed to connect to WiFi.");
  }

  // Generate the random pattern ONCE at startup
  randomSeed(analogRead(0));
  sequenceLength = random(3, 6);  // Random pattern length (3–5 steps)
  requiredTouches = random(2, 6);  // Random number of touches (2–5)
  String directions[] = {"UP", "DOWN", "LEFT", "RIGHT"};
  for (int i = 0; i < sequenceLength; i++) {
    pattern[i] = directions[random(0, 4)];
  }

  // Log the generated pattern
  Serial.print("Generated pattern: ");
  for (int i = 0; i < sequenceLength; i++) {
    Serial.print(pattern[i] + " ");
  }
  Serial.println("(Length: " + String(sequenceLength) + ", Touches: " + String(requiredTouches) + ")");

  delay(1000);
}

void loop() {
  // Only send the pattern and SMS if they haven't been sent yet
  if (!patternSent) {
    // Send pattern and touch count via SMS
    String message = "Touch " + String(requiredTouches) + " times. Pattern: ";
    for (int i = 0; i < sequenceLength; i++) {
      message += pattern[i] + " ";
    }
    Serial.println("Sending SMS: " + message);
    sendToPhone(message);

    // Send pattern to Arduino Uno via Serial2
    String patternData = "PATTERN:" + String(sequenceLength) + ":" + String(requiredTouches) + ":";
    for (int i = 0; i < sequenceLength; i++) {
      patternData += pattern[i] + " ";
    }
    Serial2.println(patternData);
    Serial.println("Sent to Arduino: " + patternData);

    patternSent = true;  // Mark the pattern as sent
  }

  // Do nothing else, just wait (no new pattern generation)
  delay(1000);
}
