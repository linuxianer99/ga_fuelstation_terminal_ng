#pragma once

#include <LiquidCrystal_I2C.h>

#include "config.h"

namespace fuelstation {

class DisplayController {
 public:
  void begin();
  void showIdle();
  void showAccepted(const String& uid);
  void showPumpRunning(float liters);
  void showPumpStopped();

 private:
  LiquidCrystal_I2C lcd_{LCD_ADDRESS, 16, 2};
};

}  // namespace fuelstation
