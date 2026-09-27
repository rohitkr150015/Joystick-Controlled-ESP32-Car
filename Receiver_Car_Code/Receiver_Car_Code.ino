// RLR INVENTIONS
// ESP-NOW car receiver
#include <WiFi.h>
#include <esp_now.h>
#include "Steering.h"

// Motor A is the left side; motor B is the right side.
#define ENA 32
#define IN1 12
#define IN2 14
#define ENB 33
#define IN3 27
#define IN4 26

// Center values from your Serial Monitor log. Set these to the readings
// printed with your joystick RELEASED if the car creeps or turns at rest.
constexpr int JOY_CENTER_X = 1782;
constexpr int JOY_CENTER_Y = 1790;
constexpr int JOY_DEADZONE = 250;
constexpr uint32_t SIGNAL_TIMEOUT_MS = 500;
static_assert(JOY_DEADZONE >= 0 && JOY_CENTER_X > JOY_DEADZONE &&
              JOY_CENTER_X + JOY_DEADZONE < 4095 &&
              JOY_CENTER_Y > JOY_DEADZONE && JOY_CENTER_Y + JOY_DEADZONE < 4095,
              "Joystick centers/deadzone must leave usable travel on both sides");

// Read the transmitter's two-int prefix; its trailing button is unused.
struct JoystickData {
  int x;
  int y;
};
JoystickData incomingData = {JOY_CENTER_X, JOY_CENTER_Y};
portMUX_TYPE dataMux = portMUX_INITIALIZER_UNLOCKED;
uint32_t lastReceivedMs = 0;
bool receivedData = false;
bool receiverReady = false;

void onDataReceived(const esp_now_recv_info *info, const uint8_t *data, int data_len) {
  if (data == nullptr || data_len < static_cast<int>(sizeof(JoystickData))) return;
  JoystickData packet;
  memcpy(&packet, data, sizeof(packet));
  if (packet.x < 0 || packet.x > 4095 || packet.y < 0 || packet.y > 4095) return;
  uint32_t now = millis();
  portENTER_CRITICAL(&dataMux);
  incomingData = packet;
  lastReceivedMs = now;
  receivedData = true;
  portEXIT_CRITICAL(&dataMux);
}

// Signed speed: positive = forward, negative = reverse, zero = stop.
void setMotor(int enablePin, int pin1, int pin2, int speed) {
  analogWrite(enablePin, 0);
  digitalWrite(pin1, speed > 0 ? HIGH : LOW);
  digitalWrite(pin2, speed < 0 ? HIGH : LOW);
  analogWrite(enablePin, speed < 0 ? -speed : speed);
}

void stopCar() {
  setMotor(ENA, IN1, IN2, 0);
  setMotor(ENB, IN3, IN4, 0);
}

void setup() {
  Serial.begin(115200);
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  stopCar();
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed!");
    return;
  }
  if (esp_now_register_recv_cb(onDataReceived) != ESP_OK) {
    Serial.println("ESP-NOW receive callback registration failed!");
    return;
  }
  receiverReady = true;
  Serial.println("Receiver ready. Release joystick to check center X/Y.");
}

void loop() {
  JoystickData joystick;
  uint32_t receivedAt;
  bool hasData;
  portENTER_CRITICAL(&dataMux);
  joystick = incomingData;
  receivedAt = lastReceivedMs;
  hasData = receivedData;
  portEXIT_CRITICAL(&dataMux);
  uint32_t now = millis();
  bool signalActive = receiverReady && hasData && (now - receivedAt <= SIGNAL_TIMEOUT_MS);
  MotorSpeeds speeds = {0, 0};
  if (signalActive) {
    int steering = axisSpeed(joystick.x, JOY_CENTER_X, JOY_DEADZONE);
    int throttle = axisSpeed(joystick.y, JOY_CENTER_Y, JOY_DEADZONE);
    speeds = mixSteering(throttle, steering);
    setMotor(ENA, IN1, IN2, speeds.left);
    setMotor(ENB, IN3, IN4, speeds.right);
  } else {
    stopCar();
  }
  static uint32_t lastLogMs = 0;
  if (now - lastLogMs >= 250) {
    lastLogMs = now;
    Serial.printf("X: %d | Y: %d | Left: %d | Right: %d | %s\n",
                  joystick.x, joystick.y, speeds.left, speeds.right,
                  signalActive ? "Receiving" : "No signal - stopped");
  }
  delay(10);
}
