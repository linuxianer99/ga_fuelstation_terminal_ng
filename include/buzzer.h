#pragma once

#include "config.h"

namespace fuelstation {

class Buzzer {
 public:
  void begin(uint8_t intensity);
  void beep();

 private:
  uint8_t intensity_ = DEFAULT_BUZZER_INTENSITY;
};

}  // namespace fuelstation
