// RLR INVENTIONS
#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.begin();     // IMPORTANT
  delay(100);

  Serial.print("ESP32 WiFi MAC: ");
  Serial.println(WiFi.macAddress());
}

void loop() {
}
