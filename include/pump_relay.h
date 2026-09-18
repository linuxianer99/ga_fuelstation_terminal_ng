#pragma once

#include "config.h"

namespace fuelstation {

class PumpRelay {
 public:
  void begin(uint8_t pin);
  void set(bool enabled);
  bool isEnabled() const;

 private:
  uint8_t pin_ = 0;
  bool enabled_ = false;
};

}  // namespace fuelstation
