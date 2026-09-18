#pragma once

#include <Arduino.h>

namespace fuelstation {

constexpr uint8_t PIN_RELAY = 33;
constexpr uint8_t PIN_FLOW_METER = 4;
constexpr uint8_t PIN_PN532_SCK = 36;
constexpr uint8_t PIN_PN532_MISO = 35;
constexpr uint8_t PIN_PN532_MOSI = 37;
constexpr uint8_t PIN_PN532_SS = 7;
constexpr uint8_t LCD_ADDRESS = 0x27;
constexpr float LITERS_PER_PULSE = 100.0f;

constexpr char WIFI_SSID[] = "YOUR_WIFI_SSID";
constexpr char WIFI_PASSWORD[] = "YOUR_WIFI_PASSWORD";
constexpr char API_HOST[] = "api.example.com";
constexpr char API_PATH[] = "/api/fuel-session";

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
