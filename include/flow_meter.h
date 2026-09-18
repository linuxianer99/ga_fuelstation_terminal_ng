#pragma once

#include "config.h"

namespace fuelstation {

class FlowMeter {
 public:
  void begin(uint8_t pin);
  void reset();
  uint32_t getPulseCount() const;
  float getLiters() const;

 private:
  static void IRAM_ATTR onPulseStatic(void* instance);

  uint8_t pin_ = 0;
  volatile uint32_t pulseCount_ = 0;
};

}  // namespace fuelstation
