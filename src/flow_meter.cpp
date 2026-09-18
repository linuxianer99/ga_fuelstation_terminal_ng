#include "flow_meter.h"

namespace fuelstation {

void FlowMeter::begin(uint8_t pin, float pulsesPerLitre) {
  pin_ = pin;
  if (pulsesPerLitre > 0.0f) {
    pulsesPerLitre_ = pulsesPerLitre;
  }
  pinMode(pin_, INPUT_PULLUP);
  pulseCount_ = 0;
  attachInterruptArg(digitalPinToInterrupt(pin_), &FlowMeter::onPulseStatic, this, RISING);
}

void FlowMeter::reset() {
  noInterrupts();
  pulseCount_ = 0;
  interrupts();
}

uint32_t FlowMeter::getPulseCount() const {
  return pulseCount_;
}

float FlowMeter::getLiters() const {
  return static_cast<float>(pulseCount_) / pulsesPerLitre_;
}

void IRAM_ATTR FlowMeter::onPulseStatic(void* instance) {
  static_cast<FlowMeter*>(instance)->pulseCount_++;
}

}  // namespace fuelstation
