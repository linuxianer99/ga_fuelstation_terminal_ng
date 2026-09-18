#include "fuel_station_controller.h"

namespace fuelstation {

void FuelStationController::begin() {
  display_.begin();
  nfc_.begin();
  relay_.begin(PIN_RELAY);
  flowMeter_.begin(PIN_FLOW_METER);
  state_ = StationState::Idle;
  Serial.println("Fuel Station Controller initialisiert");
}

void FuelStationController::update() {
  String detectedCard;
  if (nfc_.readCardId(detectedCard)) {
    if (state_ == StationState::Idle) {
      beginSession(detectedCard);
    } else {
      endSession(detectedCard);
    }
  }

  if (state_ == StationState::PumpRunning) {
    session_.pulseCount = flowMeter_.getPulseCount();
    session_.liters = flowMeter_.getLiters();
    display_.showPumpRunning(session_.liters);
  }
}

void FuelStationController::beginSession(const String& cardUid) {
  session_.cardUid = cardUid;
  session_.pulseCount = 0;
  session_.liters = 0.0f;
  session_.startedAt = millis();
  flowMeter_.reset();
  relay_.set(true);
  state_ = StationState::PumpRunning;
  display_.showAccepted(cardUid);
  delay(1200);
  Serial.println("Pumpe eingeschaltet");
}

void FuelStationController::endSession(const String& cardUid) {
  session_.cardUid = cardUid;
  session_.pulseCount = flowMeter_.getPulseCount();
  session_.liters = flowMeter_.getLiters();
  session_.endedAt = millis();
  relay_.set(false);
  state_ = StationState::Idle;
  display_.showPumpStopped();
  delay(1200);
  backend_.connectWiFi();
  backend_.sendSession(session_);
  Serial.println("Pumpe ausgeschaltet und Backend-Request gesendet");
  display_.showIdle();
}

}  // namespace fuelstation
