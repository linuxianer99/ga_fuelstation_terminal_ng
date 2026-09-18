#include "fuel_station_controller.h"

namespace fuelstation {

void FuelStationController::begin() {
  display_.begin();
  nfc_.begin();
  relay_.begin(PIN_RELAY);
  configuration_.begin();
  buzzer_.begin(configuration_.values().buzzerIntensity);
  flowMeter_.begin(PIN_FLOW_METER, configuration_.values().pulsesPerLitre);
  backend_.begin(configuration_.values(), configuration_.isFilesystemReady());
  backend_.connectWiFi();
  backend_.flushPendingSessions();
  state_ = StationState::Idle;
  nextOfflineRetryAt_ = millis() + 30000;
  Serial.println("Fuel Station Controller initialisiert");
}

void FuelStationController::update() {
  String detectedCard;
  if (nfc_.readCardId(detectedCard)) {
    buzzer_.beep();
    if (state_ == StationState::Idle) {
      beginSession(detectedCard);
    } else {
      endSession(detectedCard);
    }
  }

  if (state_ == StationState::Idle &&
      static_cast<int32_t>(millis() - nextOfflineRetryAt_) >= 0) {
    nextOfflineRetryAt_ = millis() + 30000;
    if (backend_.connectWiFi()) {
      backend_.flushPendingSessions();
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
  const bool connected = backend_.connectWiFi();
  const bool sent = connected && backend_.sendSession(session_);
  if (!sent) {
    backend_.queueSession(session_);
    Serial.println("Pumpe ausgeschaltet; Session offline gespeichert");
  } else {
    backend_.flushPendingSessions();
    Serial.println("Pumpe ausgeschaltet und Backend-Request gesendet");
  }
  display_.showIdle();
}

}  // namespace fuelstation
