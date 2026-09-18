#pragma once

#include <Arduino.h>

namespace fuelstation {

constexpr uint8_t PIN_RELAY = 33;
constexpr uint8_t PIN_FLOW_METER = 3;
constexpr uint8_t PIN_PN532_SCK = 36;
constexpr uint8_t PIN_PN532_MISO = 35;
constexpr uint8_t PIN_PN532_MOSI = 37;
constexpr uint8_t PIN_PN532_SS = 7;
constexpr uint8_t LCD_ADDRESS = 0x27;
constexpr float DEFAULT_PULSES_PER_LITRE = 90.0f;
constexpr uint8_t BUZZER_PIN = 4;
constexpr uint8_t BUZZER_CHANNEL = 0;
constexpr uint16_t BUZZER_FREQ = 2000;
constexpr uint16_t BUZZER_LEN = 500;
constexpr uint8_t BUZZER_RESOLUTION_BITS = 8;
constexpr uint8_t DEFAULT_BUZZER_INTENSITY = 20;

constexpr char WIFI_SSID[] = "YOUR_WIFI_SSID";
constexpr char WIFI_PASSWORD[] = "YOUR_WIFI_PASSWORD";
constexpr char API_HOST[] = "api.example.com";
constexpr char API_PATH[] = "/api/fuel-session";

struct RuntimeConfig {
  String wifiSsid = WIFI_SSID;
  String wifiPassword = WIFI_PASSWORD;
  String apiHost = API_HOST;
  String apiPath = API_PATH;
  uint16_t apiPort = 443;
  bool tlsInsecure = true;
  float pulsesPerLitre = DEFAULT_PULSES_PER_LITRE;
  uint8_t buzzerIntensity = DEFAULT_BUZZER_INTENSITY;
};

struct SessionData {
  String cardUid;
  uint32_t pulseCount = 0;
  float liters = 0.0f;
  uint32_t startedAt = 0;
  uint32_t endedAt = 0;
};

enum class StationState {
  Idle,
  PumpRunning
};

}  // namespace fuelstation
