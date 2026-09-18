#pragma once

#include "backend_client.h"
#include "buzzer.h"
#include "config.h"
#include "configuration.h"
#include "display_controller.h"
#include "flow_meter.h"
#include "nfc_reader.h"
#include "pump_relay.h"

namespace fuelstation {

class FuelStationController {
 public:
  void begin();
  void update();

 private:
  void beginSession(const String& cardUid);
  void endSession(const String& cardUid);

  DisplayController display_;
  Buzzer buzzer_;
  NfcReader nfc_;
  PumpRelay relay_;
  FlowMeter flowMeter_;
  Configuration configuration_;
  BackendClient backend_;
  StationState state_ = StationState::Idle;
  SessionData session_;
  uint32_t nextOfflineRetryAt_ = 0;
};

}  // namespace fuelstation
