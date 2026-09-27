// RLR INVENTIONS
// ESPNOW RC CAR Transmitter

#include <esp_now.h>
#include <WiFi.h>

// Joystick pins
#define VRx 34
#define VRy 35
#define SW  32

// Data structure
typedef struct struct_message {
  int xValue;
  int yValue;
  bool buttonPressed;
} struct_message;

struct_message dataToSend;

// Receiver MAC
uint8_t receiverMAC[] = {0x58, 0x2A, 0xBD, 0xD7, 0x66, 0x70}; //  58:2A:BD:D7:66:70

// NEW ESP-IDF v5.x callback
void onDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Failed");
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("=== ESP-NOW TRANSMITTER STARTED ===");

  pinMode(SW, INPUT_PULLUP);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init Failed!");
    return;
  }

  esp_now_register_send_cb(onDataSent);

  // Add peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  Serial.println("Transmitter Ready!");
}

void loop() {
  dataToSend.xValue = analogRead(VRx);
  dataToSend.yValue = analogRead(VRy);
  dataToSend.buttonPressed = !digitalRead(SW);

  // ★ Serial Debug Output
  Serial.print("X: ");
  Serial.print(dataToSend.xValue);
  Serial.print(" | Y: ");
  Serial.print(dataToSend.yValue);
  Serial.print(" | Button: ");
  Serial.println(dataToSend.buttonPressed ? "PRESSED" : "RELEASED");

  // Send to receiver
  esp_now_send(receiverMAC, (uint8_t *)&dataToSend, sizeof(dataToSend));

  delay(100);
}
