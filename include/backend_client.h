#pragma once

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#include "config.h"

namespace fuelstation {

class BackendClient {
 public:
  bool connectWiFi();
  bool sendSession(const SessionData& session);
};

}  // namespace fuelstation
