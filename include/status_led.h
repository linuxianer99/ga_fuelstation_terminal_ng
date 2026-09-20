#pragma once

#include "config.h"

namespace fuelstation {

class StatusLed {
 public:
  void begin();
  void update(StationState state);

 private:
  uint32_t fadeStartedAt_ = 0;
};

}  // namespace fuelstation