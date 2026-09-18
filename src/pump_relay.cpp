#include "pump_relay.h"

namespace fuelstation {

void PumpRelay::begin(uint8_t pin) {
  pin_ = pin;
  pinMode(pin_, OUTPUT);
  set(false);
}

void PumpRelay::set(bool enabled) {
  enabled_ = enabled;
  digitalWrite(pin_, enabled_ ? HIGH : LOW);
}

bool PumpRelay::isEnabled() const {
  return enabled_;
}

}  // namespace fuelstation
