#include "backend_client.h"

namespace fuelstation {

bool BackendClient::connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Verbinde mit WiFi");
  for (uint8_t i = 0; i < 20 && WiFi.status() != WL_CONNECTED; ++i) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi Verbindung fehlgeschlagen");
    return false;
  }

  Serial.println("WiFi verbunden");
  Serial.println(WiFi.localIP());
  return true;
}

bool BackendClient::sendSession(const SessionData& session) {
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

  WiFiClientSecure client;
  client.setInsecure();

  if (!client.connect(API_HOST, 443)) {
    Serial.println("HTTPS Verbindung fehlgeschlagen");
    return false;
  }

  JsonDocument doc;
  doc["card_uid"] = session.cardUid;
  doc["pulse_count"] = session.pulseCount;
  doc["liters"] = session.liters;
  doc["started_at"] = session.startedAt;
  doc["ended_at"] = session.endedAt;
  doc["status"] = "pump_stopped";

  String payload;
  serializeJson(doc, payload);

  client.print("POST ");
  client.print(API_PATH);
  client.print(" HTTP/1.1\r\n");
  client.print("Host: ");
  client.print(API_HOST);
  client.print("\r\n");
  client.print("Content-Type: application/json\r\n");
  client.print("Content-Length: ");
  client.print(payload.length());
  client.print("\r\n");
  client.print("Connection: close\r\n\r\n");
  client.print(payload);
  client.print("\r\n");

  delay(500);
  while (client.available()) {
    String line = client.readStringUntil('\n');
    Serial.println(line);
  }

  Serial.println("JSON Payload gesendet");
  return true;
}

}  // namespace fuelstation
