#include "configuration.h"

#include <ArduinoJson.h>
#include <LittleFS.h>

namespace fuelstation {

bool Configuration::begin() {
  filesystemReady_ = LittleFS.begin(true);
  if (!filesystemReady_) {
    Serial.println("LittleFS Initialisierung fehlgeschlagen");
    return false;
  }

  Serial.println("LittleFS initialisiert");
  loadJson();
  return true;
}

const RuntimeConfig& Configuration::values() const {
  return values_;
}

bool Configuration::isFilesystemReady() const {
  return filesystemReady_;
}

bool Configuration::loadJson() {
  File file = LittleFS.open("/config.json", FILE_READ);
  if (!file) {
    Serial.println("config.json nicht gefunden; Standardwerte werden verwendet");
    return false;
  }

  JsonDocument doc;
  const DeserializationError error = deserializeJson(doc, file);
  file.close();
  if (error) {
    Serial.print("config.json ungueltig: ");
    Serial.println(error.c_str());
    return false;
  }

  if (doc["wifi_ssid"].is<const char*>()) {
    values_.wifiSsid = doc["wifi_ssid"].as<const char*>();
  }
  if (doc["wifi_password"].is<const char*>()) {
    values_.wifiPassword = doc["wifi_password"].as<const char*>();
  }
  if (doc["api_host"].is<const char*>()) {
    values_.apiHost = doc["api_host"].as<const char*>();
  }
  if (doc["api_path"].is<const char*>()) {
    values_.apiPath = doc["api_path"].as<const char*>();
  }
  if (doc["api_port"].is<uint16_t>()) {
    values_.apiPort = doc["api_port"].as<uint16_t>();
  }
  if (doc["tls_insecure"].is<bool>()) {
    values_.tlsInsecure = doc["tls_insecure"].as<bool>();
  }
  const float pulsesPerLitre = doc["pulse_per_litre"].as<float>();
  if (pulsesPerLitre > 0.0f) {
    values_.pulsesPerLitre = pulsesPerLitre;
  }

  Serial.println("config.json geladen");
  return true;
}

}  // namespace fuelstation
