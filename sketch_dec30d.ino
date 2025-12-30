#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#define SERVO_CH 0
#define LIMIT_PIN D3

// ===== SERVO (MG90S SAFE RANGE) =====
#define SERVO_MIN 300   // ~0°
#define SERVO_MAX 500   // ~180°
#define SERVO_CENTER 410

int servoPos = SERVO_CENTER;

// ===== WIFI AP =====
const char* ssid = "ESP_SERVO";
const char* password = "12345678";

ESP8266WebServer server(80);
Adafruit_PWMServoDriver pwm(0x40);

// ===== SERVO MOVE =====
void moveServo(int delta) {
  servoPos += delta;

  if (servoPos < SERVO_MIN) servoPos = SERVO_MIN;
  if (servoPos > SERVO_MAX) servoPos = SERVO_MAX;

  pwm.setPWM(SERVO_CH, 0, servoPos);
}

// ===== WWW PAGE =====
String htmlPage() {
  bool limitState = digitalRead(LIMIT_PIN);

  String stateText = limitState ? "OFF" : "ON";

  String page =
    "<!DOCTYPE html><html><head>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<style>"
    "button{width:120px;height:60px;font-size:20px;margin:10px;}"
    "</style></head><body>"
    "<h2>ESP8266 Servo Control</h2>"
    "<p><b>Servo position:</b> " + String(servoPos) + "</p>"
    "<p><b>Limit switch (GPIO0 / D3):</b> " + stateText + "</p>"
    "<a href='/minus'><button>-100 deg</button></a>"
    "<a href='/plus'><button>+100 deg</button></a>"
    "</body></html>";

  return page;
}

// ===== HANDLERS =====
void handleRoot() {
  server.send(200, "text/html", htmlPage());
}

void handlePlus() {
  moveServo(+100);
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleMinus() {
  moveServo(-100);
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);

  pinMode(LIMIT_PIN, INPUT_PULLUP);

  // I2C
  Wire.begin(D2, D1);
  pwm.begin();
  pwm.setPWMFreq(50);

  pwm.setPWM(SERVO_CH, 0, servoPos);

  // WiFi AP
  WiFi.softAP(ssid, password);
  Serial.println("WiFi AP started");
  Serial.println(WiFi.softAPIP());

  // WWW
  server.on("/", handleRoot);
  server.on("/plus", handlePlus);
  server.on("/minus", handleMinus);
  server.begin();
}

void loop() {
  server.handleClient();
}
