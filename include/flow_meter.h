#pragma once

#include "config.h"

namespace fuelstation {

class FlowMeter {
 public:
  void begin(uint8_t pin, float pulsesPerLitre);
  void reset();
  uint32_t getPulseCount() const;
  float getLiters() const;

 private:
  static void IRAM_ATTR onPulseStatic(void* instance);

  uint8_t pin_ = 0;
  float pulsesPerLitre_ = DEFAULT_PULSES_PER_LITRE;
  volatile uint32_t pulseCount_ = 0;
};

}  // namespace fuelstation
