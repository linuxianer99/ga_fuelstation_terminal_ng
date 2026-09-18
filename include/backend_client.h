#pragma once

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#include "config.h"

namespace fuelstation {

class BackendClient {
 public:
  void begin();
  bool connectWiFi();
  bool sendSession(const SessionData& session);
  bool queueSession(const SessionData& session);
  void flushPendingSessions();

 private:
  bool sendPayload(const String& payload);
  String serializeSession(const SessionData& session) const;
  String nextQueuePath() const;
};

}  // namespace fuelstation
