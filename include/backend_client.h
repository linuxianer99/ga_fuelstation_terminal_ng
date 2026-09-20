#pragma once

#include <WiFi.h>
#include <WiFiClientSecure.h>
#if defined(CONFIG_ETH_ENABLED) && CONFIG_ETH_ENABLED
#include <ETH.h>
#endif

#include "config.h"

namespace fuelstation {

class BackendClient {
 public:
  void begin(const RuntimeConfig& config, bool filesystemReady);
  bool connectWiFi();
  bool sendSession(const SessionData& session);
  bool queueSession(const SessionData& session);
  void flushPendingSessions();

 private:
  bool isNetworkConnected() const;
  bool sendPayload(const String& payload);
  String serializeSession(const SessionData& session) const;
  String nextQueuePath() const;

  RuntimeConfig config_;
  bool filesystemReady_ = false;
};

}  // namespace fuelstation
