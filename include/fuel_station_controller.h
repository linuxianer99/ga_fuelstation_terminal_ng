#pragma once

#include "backend_client.h"
#include "buzzer.h"
#include "config.h"
#include "configuration.h"
#include "display_controller.h"
#include "flow_meter.h"
#include "nfc_reader.h"
#include "pump_relay.h"
#include "status_led.h"
#include "web_server.h"

namespace fuelstation {

class FuelStationController {
 public:
  void begin();
  void update();
  StationState state() const;
  const SessionData& session() const;

 private:
  void beginSession(const String& cardUid);
  void endSession(const String& cardUid);

  DisplayController display_;
  Buzzer buzzer_;
  NfcReader nfc_;
  PumpRelay relay_;
  StatusLed statusLed_;
  FlowMeter flowMeter_;
  Configuration configuration_;
  BackendClient backend_;
  WebServerController webServer_;
  StationState state_ = StationState::Idle;
  SessionData session_;
  uint32_t nextOfflineRetryAt_ = 0;
};

}  // namespace fuelstation
