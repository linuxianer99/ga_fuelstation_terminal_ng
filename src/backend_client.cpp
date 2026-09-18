#include "backend_client.h"

#include <LittleFS.h>

namespace fuelstation {

void BackendClient::begin() {
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS Initialisierung fehlgeschlagen");
    return;
  }

  Serial.println("LittleFS initialisiert");
}

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

  return sendPayload(serializeSession(session));
}

bool BackendClient::queueSession(const SessionData& session) {
  File file = LittleFS.open(nextQueuePath(), FILE_WRITE);
  if (!file) {
    Serial.println("Offline-Session konnte nicht gespeichert werden");
    return false;
  }

  const String payload = serializeSession(session);
  const size_t written = file.print(payload);
  file.close();

  if (written != payload.length()) {
    Serial.println("Offline-Session wurde unvollstaendig gespeichert");
    return false;
  }

  Serial.println("Session fuer Offline-Upload gespeichert");
  return true;
}

void BackendClient::flushPendingSessions() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  File root = LittleFS.open("/");
  if (!root) {
    return;
  }

  File file = root.openNextFile();
  while (file) {
    const String path = file.name();
    File pending = LittleFS.open(path, FILE_READ);
    String payload;
    if (pending) {
      payload = pending.readString();
      pending.close();
    }
    file.close();

    if (!path.startsWith("/offline_") || payload.isEmpty()) {
      file = root.openNextFile();
      continue;
    }

    if (!sendPayload(payload)) {
      Serial.println("Offline-Upload fehlgeschlagen; Datei bleibt erhalten");
      break;
    }

    LittleFS.remove(path);
    Serial.print("Offline-Session uebertragen: ");
    Serial.println(path);
    file = root.openNextFile();
  }
  root.close();
}

String BackendClient::serializeSession(const SessionData& session) const {
  JsonDocument doc;
  doc["card_uid"] = session.cardUid;
  doc["pulse_count"] = session.pulseCount;
  doc["liters"] = session.liters;
  doc["started_at"] = session.startedAt;
  doc["ended_at"] = session.endedAt;
  doc["status"] = "pump_stopped";

  String payload;
  serializeJson(doc, payload);
  return payload;
}

String BackendClient::nextQueuePath() const {
  String path;
  uint32_t sequence = millis();
  do {
    path = "/offline_" + String(sequence++) + ".json";
  } while (LittleFS.exists(path));
  return path;
}

bool BackendClient::sendPayload(const String& payload) {
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

  WiFiClientSecure client;
  client.setInsecure();

  if (!client.connect(API_HOST, 443)) {
    Serial.println("HTTPS Verbindung fehlgeschlagen");
    return false;
  }

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

  const uint32_t timeoutAt = millis() + 5000;
  while (!client.available() && millis() < timeoutAt) {
    delay(10);
  }

  const String statusLine = client.readStringUntil('\n');
  const bool successful = statusLine.startsWith("HTTP/1.1 2") ||
                          statusLine.startsWith("HTTP/1.0 2");
  if (!successful) {
    Serial.print("Backend antwortet mit: ");
    Serial.println(statusLine);
  }
  return successful;
}

}  // namespace fuelstation
